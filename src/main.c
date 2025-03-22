/************************************************************
 * main.c: Assembler driver
 *
 * Merges all prior logic:
 *  - .data/.string => no ARE bits
 *  - .extern / .entry logic
 *  - .ext includes all references to external labels
 *  - .ent includes each entry label
 *  - Writes .ob, .ext, .ent in the 'test' folder
 *  - Replaces the lambda with a normal C function
 ************************************************************/

#include "../include/definitions.h"
#include "../include/encoder.h"   /* if you have one */
#include "../include/parser.h"
#include "../include/io.h"
#include "../include/label.h"
#include "../include/data.h"
#include "../include/macro.h"
#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_MNEMONIC_LENGTH 20

/************************************************************
 * 1) Data structures for referencing external labels
 *    multiple times (one line per usage).
 ************************************************************/

typedef struct {
    char labelName[MAX_SYMBOL_NAME];
    int address;  /* usage address where the code references this extern label */
} ExternalRef;

typedef struct {
    ExternalRef *items;
    size_t count;
    size_t capacity;
} ExtRefArray;

/* Initialize the external references array */
static void initExtRefArray(ExtRefArray *arr) {
    arr->count = 0;
    arr->capacity = 8;
    arr->items = (ExternalRef *)malloc(arr->capacity * sizeof(ExternalRef));
}

/* Free the external references array */
static void freeExtRefArray(ExtRefArray *arr) {
    if (!arr) return;
    if (arr->items) free(arr->items);
    arr->items = NULL;
    arr->capacity = 0;
    arr->count = 0;
}

/* Add an external reference (labelName, usageAddress) to the array */
static void AddExtRef(ExtRefArray *arr, const char *label, int address) {
    if (!arr || !label) return;
    if (arr->count >= arr->capacity) {
        size_t newCap = arr->capacity * 2;
        ExternalRef *tmp = realloc(arr->items, newCap * sizeof(ExternalRef));
        if (!tmp) {
            fprintf(stderr,"Error: out of memory in AddExtRef.\n");
            return;
        }
        arr->items = tmp;
        arr->capacity = newCap;
    }
    strncpy(arr->items[arr->count].labelName, label, MAX_SYMBOL_NAME-1);
    arr->items[arr->count].labelName[MAX_SYMBOL_NAME-1] = '\0';
    arr->items[arr->count].address = address;
    arr->count++;
}

/* Print each external reference in the format:
   <labelName> <address>, one line per usage. */
static void writeExtFile(const char *path, ExtRefArray *arr) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Cannot open %s for writing.\n", path);
        return;
    }
    for (size_t i = 0; i < arr->count; i++) {
        /* e.g. "MY_EXTERN 0000130" => you can choose your format. */
        fprintf(fp, "%s %07d\n", arr->items[i].labelName, arr->items[i].address);
    }
    fclose(fp);
}

/************************************************************
 * 2) Our normal MachineWord, .data no ARE bits, macros, etc.
 ************************************************************/

typedef struct {
    char bin[25];  /* 24-bit binary string */
    char mnemonic[MAX_MNEMONIC_LENGTH];
    int  address;  /* final memory address for this word */
} MachineWord;

typedef enum {
    DATA_WORD,
    COMMAND_WORD
} WordType;

/* Convert integer => 24-bit binary string */
static void intToBinary(uint32_t value, int bits, char *dest) {
    dest[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        dest[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

/* The original encodeDataWord => shift left 3 bits, ARE=100 in bits [2..0]. */
static void encodeDataWord(int value, char *bin)
{
    if (value < 0) {
        value &= 0x001FFFFF; /* keep 21 bits */
    }
    uint32_t val24 = ((uint32_t)value & 0x001FFFFF) << 3; /* shift left 3 bits */
    val24 |= 4; /* set bits 2..0 => 100 for A=100 */
    intToBinary(val24, 24, bin);
}

/* A new function for .data/.string => no ARE => bits 2..0=000. */
static void encodeDataWordNoARE(int value, char *bin)
{
    if (value < 0) {
        value &= 0x001FFFFF; 
    }
    uint32_t val24 = (uint32_t)value & 0x001FFFFF; /* no shift => bits [2..0]=000 */
    intToBinary(val24, 24, bin);
}

/************************************************************
 * 3) .extern and .entry logic
 ************************************************************/
static void handleDirective(const char *line, LabelTable *table) {
    /* e.g. line => ".extern W" or ".entry MAIN" */
    char copy[MAX_LINE_LENGTH];
    strncpy(copy, line, MAX_LINE_LENGTH);
    copy[MAX_LINE_LENGTH-1] = '\0';
    TrimWhiteSpace(copy);

    char *directive = strtok(copy, " \t");
    if (!directive) return;

    if (strcmp(directive, ".extern") == 0) {
        char *lblName = strtok(NULL, " \t");
        if (lblName) {
            /* Mark label as external => address=0 or not used */
            addLabel(table, lblName, 0, 0, 1);
        }
    } else if (strcmp(directive, ".entry") == 0) {
        char *lblName = strtok(NULL, " \t");
        if (lblName) {
            /* find or add label => isEntry=1 */
            Label *lbl = findLabel(table, lblName);
            if (lbl) {
                lbl->isEntry = 1;
            } else {
                addLabel(table, lblName, 0, 1, 0);
            }
        }
    }
}

/************************************************************
 * 4) Assembling instructions => We'll catch external references.
 ************************************************************/

/* Build the first word of an instruction => bits 23..0. */
static void encodeCommandWord(const CommandWord *cmd,
                              int srcReg, int srcAddr,
                              int dstReg, int dstAddr,
                              char *bin)
{
    uint32_t cw = 0;
    cw |= (cmd->opcode & 0x3F) << 18;
    cw |= (srcAddr & 0x3) << 16;
    cw |= (srcReg & 0x7) << 13;
    cw |= (dstAddr & 0x3) << 11;
    cw |= (cmd->funct & 0x1F) << 3;
    cw |= (dstReg & 0x7) << 8;
    cw |= A; /* bits 2..0 => 100 for absolute by default */
    intToBinary(cw, 24, bin);
}

/* 
   If isRelative=1 or direct => we look up the label's address.
   If the label is external => we set E=1, etc.
*/
static uint32_t encodeLabelOperand(const LabelTable *lblTable,
                                   const char *labelName,
                                   int currentIC,
                                   int isRelative,
                                   int *areOut)
{
    *areOut = 4; /* A=100 by default */
    Label *lbl = findLabel((LabelTable*)lblTable, labelName);
    if (!lbl) {
        return 0; /* not found => error or fallback 0 */
    }
    if (lbl->isExternal) {
        /* E=001 => external => we store 0 in the bits */
        *areOut = 1;
        return 0;
    } else {
        /* R=010 => local label => address or offset */
        *areOut = 2;
        if (isRelative) {
            int offset = lbl->address - currentIC;
            offset &= 0x001FFFFF;
            return offset;
        } else {
            return (lbl->address & 0x001FFFFF);
        }
    }
}

/* 
   We'll define a normal C function instead of a lambda, to handle 
   direct/relative label operand. This replaces the previous C++ lambda.
*/
static void handleLabelOperandC(
    const char *labelToken,
    int isRelativeMode,
    int currentIC,
    int *addressCounter,
    LabelTable *lblTable,
    MachineWord *words,
    int *wordIndex,
    WordType *types,
    ExtRefArray *extRefs
) {
    int areVal = 4;
    uint32_t val21 = encodeLabelOperand(lblTable,
                                        labelToken,
                                        currentIC,
                                        isRelativeMode,
                                        &areVal);

    /* If label is external => record usage in extRefs. */
    Label *lbl = findLabel(lblTable, labelToken);
    if (lbl && lbl->isExternal) {
        /* The usage address is the address for *this extra word*. */
        AddExtRef(extRefs, lbl->name, *addressCounter);
    }

    /* Now build the final 24 bits => shift left 3, set bits [2..0] = areVal. */
    char bin[25];
    uint32_t full24 = (val21 & 0x1FFFFF) << 3;
    full24 |= (areVal & 0x7);
    intToBinary(full24, 24, bin);

    /* store result in machineWords array. */
    words[*wordIndex].mnemonic[0] = '\0';
    strcpy(words[*wordIndex].bin, bin);
    types[*wordIndex] = COMMAND_WORD;
    words[*wordIndex].address = *addressCounter;
    (*wordIndex)++;
    (*addressCounter)++;
}

/* 
   MAIN function for encoding an instruction line => up to 2 operands.
   We also pass in an ExtRefArray => if the label is external,
   we record the usage address in extRefs.
*/
static void encodeInstructionLine(const char *line,
                                  MachineWord *words,
                                  int *wordIndex,
                                  LabelTable *lblTable,
                                  WordType *types,
                                  int currentIC,
                                  int *addressCounter,
                                  ExtRefArray *extRefs)
{
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH-1] = '\0';
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    TrimWhiteSpace(instr);

    char *mnemonic = strtok(instr, " \t,\n");
    if (!mnemonic) return;

    /* find the command definition */
    const CommandWord *cmdDef = NULL;
    for (int i=0; i<NUM_COMMANDS; i++) {
        if (strcmp(mnemonic, operations[i]) == 0) {
            cmdDef = commands[i];
            break;
        }
    }
    if (!cmdDef) return;

    int oneOperand = cmdDef->oneOperand;
    char *op1 = strtok(NULL, " \t,\n");
    char *op2 = (oneOperand) ? NULL : strtok(NULL, " \t,\n");

    int srcAddr=0, srcReg=0, dstAddr=0, dstReg=0;

    /* parse addressing modes */
    if (op1) {
        if (oneOperand) {
            if (op1[0] == '#') {
                dstAddr = ADDRESS_IMMEDIATE;
            } else if (isRegister(op1)) {
                dstAddr = ADDRESS_REGISTER;
                dstReg = op1[1] - '0';
            } else if (op1[0] == '&') {
                dstAddr = ADDRESS_RELATIVE;
            } else {
                dstAddr = ADDRESS_DIRECT;
            }
        } else {
            if (op1[0] == '#') {
                srcAddr = ADDRESS_IMMEDIATE;
            } else if (isRegister(op1)) {
                srcAddr = ADDRESS_REGISTER;
                srcReg = op1[1] - '0';
            } else if (op1[0] == '&') {
                srcAddr = ADDRESS_RELATIVE;
            } else {
                srcAddr = ADDRESS_DIRECT;
            }
        }
    }
    if (!oneOperand && op2) {
        if (op2[0] == '#') {
            dstAddr=ADDRESS_IMMEDIATE;
        } else if (isRegister(op2)) {
            dstAddr=ADDRESS_REGISTER;
            dstReg = op2[1]-'0';
        } else if (op2[0]=='&') {
            dstAddr=ADDRESS_RELATIVE;
        } else {
            dstAddr=ADDRESS_DIRECT;
        }
    }

    /* Build the main command word */
    char bin[25];
    encodeCommandWord(cmdDef, srcReg, srcAddr, dstReg, dstAddr, bin);

    /* store it in machineWords array */
    strncpy(words[*wordIndex].mnemonic, mnemonic, MAX_MNEMONIC_LENGTH-1);
    words[*wordIndex].mnemonic[MAX_MNEMONIC_LENGTH-1]='\0';
    strcpy(words[*wordIndex].bin, bin);
    types[*wordIndex] = COMMAND_WORD;
    words[*wordIndex].address = *addressCounter;
    (*wordIndex)++;
    (*addressCounter)++;

    /* if we have a src operand that needs an extra word */
    if (!oneOperand && op1) {
        if (srcAddr==ADDRESS_IMMEDIATE) {
            int num = atoi(op1+1);
            encodeDataWord(num, bin);

            words[*wordIndex].mnemonic[0]='\0';
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = COMMAND_WORD;
            words[*wordIndex].address = *addressCounter;
            (*wordIndex)++;
            (*addressCounter)++;
        } else if (srcAddr==ADDRESS_DIRECT) {
            /* call handleLabelOperandC (direct => isRelative=0) */
            handleLabelOperandC(op1, 0, currentIC+1, addressCounter,
                                lblTable, words, wordIndex, types, extRefs);
        } else if (srcAddr==ADDRESS_RELATIVE) {
            /* skip '&' => op1+1, isRelative=1 */
            handleLabelOperandC(op1+1, 1, currentIC+1, addressCounter,
                                lblTable, words, wordIndex, types, extRefs);
        }
    }

    /* if we have a dst operand that needs an extra word */
    if ((oneOperand && op1 && (dstAddr==ADDRESS_IMMEDIATE||dstAddr==ADDRESS_DIRECT||dstAddr==ADDRESS_RELATIVE))
        || (!oneOperand && op2 && (dstAddr==ADDRESS_IMMEDIATE||dstAddr==ADDRESS_DIRECT||dstAddr==ADDRESS_RELATIVE))) {

        const char *opDest = (oneOperand? op1 : op2);
        if (dstAddr==ADDRESS_IMMEDIATE) {
            int num=atoi(opDest+1);
            encodeDataWord(num, bin);

            words[*wordIndex].mnemonic[0]='\0';
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = COMMAND_WORD;
            words[*wordIndex].address = *addressCounter;
            (*wordIndex)++;
            (*addressCounter)++;
        } else if (dstAddr==ADDRESS_DIRECT) {
            handleLabelOperandC(opDest, 0, currentIC+(oneOperand?1:2),
                                addressCounter, lblTable, words, wordIndex,
                                types, extRefs);
        } else {
            /* relative => &label => skip '&' => opDest+1 */
            handleLabelOperandC(opDest+1, 1, currentIC+(oneOperand?1:2),
                                addressCounter, lblTable, words, wordIndex,
                                types, extRefs);
        }
    }
}

/* encodeDataLine => .data/.string lines => no ARE bits => encodeDataWordNoARE. */
static void encodeDataLine(const char *line,
                           MachineWord *words,
                           int *wordIndex,
                           WordType *types,
                           int *addressCounter)
{
    if (strstr(line,".data")) {
        const char *p = strstr(line,".data") + 5;
        while (*p) {
            while (isspace((unsigned char)*p)) p++;
            if (*p=='\0') break;
            char *end;
            int num=(int)strtol(p,&end,10);
            p = end;
            char bin[25];
            encodeDataWordNoARE(num, bin);

            strcpy(words[*wordIndex].bin, bin);
            words[*wordIndex].mnemonic[0] = '\0';
            types[*wordIndex] = DATA_WORD;
            words[*wordIndex].address = *addressCounter;
            (*wordIndex)++;
            (*addressCounter)++;

            while (isspace((unsigned char)*p)) p++;
            if (*p==',') p++;
        }
    } else if (strstr(line,".string")) {
        char *start=strchr(line,'"');
        if (!start) return;
        start++;
        while (*start && *start!='"') {
            char bin[25];
            encodeDataWordNoARE((int)*start, bin);

            strcpy(words[*wordIndex].bin, bin);
            words[*wordIndex].mnemonic[0] = '\0';
            types[*wordIndex] = DATA_WORD;
            words[*wordIndex].address = *addressCounter;
            (*wordIndex)++;
            (*addressCounter)++;
            start++;
        }
        /* null terminator => 0 */
        char bin[25];
        encodeDataWordNoARE(0, bin);
        strcpy(words[*wordIndex].bin, bin);
        words[*wordIndex].mnemonic[0]='\0';
        types[*wordIndex]=DATA_WORD;
        words[*wordIndex].address=*addressCounter;
        (*wordIndex)++;
        (*addressCounter)++;
    }
}

/* We'll read from test/test1.asm, but you can adapt the path. */
static const char *testInputFile="test/test1.asm";

int main(void)
{
    printf("Starting program.\n");

    /* PASS 1: Print Input File */
    printf("----------\nInput file content:\n");
    FILE *inputFile=fopen(testInputFile,"r");
    if(!inputFile) {
        fprintf(stderr,"Error opening input file %s\n", testInputFile);
        return 1;
    }
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,inputFile)) {
            printf("%s", line);
        }
    }
    fclose(inputFile);

    /* PASS 2: Process Macros */
    MacroArray macroArray;
    initMacroArray(&macroArray);
    if(ParseMacrosDynamic(testInputFile, &macroArray)!=0){
        printf("ERROR: ParseMacrosDynamic failed\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    if(DeleteMacroDefinitions(testInputFile,"test/temp.am")!=0){
        printf("ERROR: DeleteMacroDefinitions failed\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    if(ExpandMacros("test/temp.am","test/test2.am",&macroArray)!=0){
        printf("ERROR: ExpandMacros failed\n");
        freeMacroArray(&macroArray);
        return 1;
    }

    /* PASS 3: Print Expanded File */
    FILE *expandedFile=fopen("test/test2.am","r");
    if(!expandedFile) {
        printf("ERROR: Could not open test/test2.am\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    printf("----------\nExpanded file content:\n");
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,expandedFile)) {
            printf("%s", line);
        }
    }
    fclose(expandedFile);

    /* PASS 4: Count total words */
    int totalInstWords=0, totalDataWords=0;
    expandedFile=fopen("test/test2.am","r");
    if(!expandedFile) {
        fprintf(stderr,"Error opening expanded file (pass4)\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,expandedFile)){
            char trimmed[MAX_LINE_LENGTH];
            strncpy(trimmed,line,MAX_LINE_LENGTH);
            trimmed[MAX_LINE_LENGTH-1]='\0';
            TrimWhiteSpace(trimmed);
            if(trimmed[0]=='\0' || trimmed[0]==COMMENT_CHAR)
                continue;
            if(strstr(trimmed,".extern")||strstr(trimmed,".entry")){
                continue; /* don't add words, but handle in pass5 */
            }
            if(strstr(trimmed,".data")||strstr(trimmed,".string")) {
                totalDataWords += countDataWords(trimmed);
            } else {
                /* is it an instruction line? We'll do a quick check: */
                if(!strstr(trimmed,".data") && !strstr(trimmed,".string")
                   && !strstr(trimmed,".extern") && !strstr(trimmed,".entry")){
                    totalInstWords += countInstructionWords(trimmed);
                }
            }
        }
    }
    fclose(expandedFile);
    int totalWordCount=totalInstWords+totalDataWords;

    /* PASS 5: Build label table (first pass),
       also handle .extern/.entry => mark isExternal/isEntry. */
    DataImage dataImage;
    initDataImage(&dataImage);
    LabelTable labelTable;
    initLabelTable(&labelTable);

    int instStart=100;
    int dataStart=instStart+totalInstWords;
    expandedFile=fopen("test/test2.am","r");
    if(!expandedFile){
        fprintf(stderr,"Error opening expanded file (pass5)\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,expandedFile)){
            char workLine[MAX_LINE_LENGTH];
            strncpy(workLine,line,MAX_LINE_LENGTH);
            workLine[MAX_LINE_LENGTH-1] = '\0';
            TrimWhiteSpace(workLine);
            if(workLine[0]=='\0' || workLine[0]==COMMENT_CHAR)
                continue;
            /* handle .extern/.entry => set label flags */
            if(strstr(workLine,".extern") || strstr(workLine,".entry")){
                handleDirective(workLine,&labelTable);
                continue;
            }

            char *colon=strchr(workLine,':');
            char labelName[MAX_SYMBOL_NAME]={0};
            int hasLabel=0;
            if(colon){
                hasLabel=1;
                size_t len=colon-workLine;
                if(len<MAX_SYMBOL_NAME){
                    strncpy(labelName,workLine,len);
                    labelName[len]='\0';
                }
            }
            if(strstr(workLine,".data")||strstr(workLine,".string")){
                if(hasLabel){
                    addLabel(&labelTable,labelName,dataStart,0,0);
                }
                int n=(strstr(workLine,".data"))? countDataWords(workLine):countStringWords(workLine);
                dataStart+=n;
            } else {
                /* assume instruction line */
                if(hasLabel){
                    addLabel(&labelTable,labelName,instStart,0,0);
                }
                int n=countInstructionWords(workLine);
                instStart+=n;
            }
        }
    }
    fclose(expandedFile);

    /* PASS 6: Produce 'clean' file w/o labels (removing label definitions) */
    FILE *cleanFile=fopen("test/test2.noLabels.am","w");
    if(!cleanFile){
        fprintf(stderr,"Error opening test/test2.noLabels.am\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    expandedFile=fopen("test/test2.am","r");
    if(!expandedFile){
        fclose(cleanFile);
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,expandedFile)){
            char trimmedLine[MAX_LINE_LENGTH];
            strncpy(trimmedLine,line,MAX_LINE_LENGTH);
            trimmedLine[MAX_LINE_LENGTH-1]='\0';
            TrimWhiteSpace(trimmedLine);
            if(trimmedLine[0]=='\0' || trimmedLine[0]==COMMENT_CHAR)
                continue;

            /* remove .extern/.entry lines in final code, or keep them? We'll remove them. */
            if(strstr(trimmedLine,".extern")||strstr(trimmedLine,".entry"))
                continue;

            char *colon=strchr(line,':');
            if(colon){
                /* skip label part => write the remainder. */
                char *rest=colon+1;
                while(isspace((unsigned char)*rest)) rest++;
                fprintf(cleanFile,"%s", rest);
            } else {
                fprintf(cleanFile,"%s", line);
            }
        }
    }
    fclose(expandedFile);
    fclose(cleanFile);

    /* PASS 7: Encode instructions/data => machineWords */
    MachineWord *machineWords = malloc(totalWordCount*sizeof(MachineWord));
    WordType    *wordTypes   = malloc(totalWordCount*sizeof(WordType));
    if(!machineWords || !wordTypes){
        fprintf(stderr,"Error allocating machine code array\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        if(machineWords) free(machineWords);
        if(wordTypes)   free(wordTypes);
        return 1;
    }

    /* We'll keep a list of external references => so each usage is in .ext */
    ExtRefArray extRefs;
    initExtRefArray(&extRefs);

    int mWordIndex=0;
    int IC=100;
    int addressCounter=100;

    FILE *codeFile=fopen("test/test2.noLabels.am","r");
    if(!codeFile){
        free(machineWords);
        free(wordTypes);
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        freeExtRefArray(&extRefs);
        return 1;
    }
    {
        char line[MAX_LINE_LENGTH];
        while(fgets(line,MAX_LINE_LENGTH,codeFile)){
            char trimmed[MAX_LINE_LENGTH];
            strncpy(trimmed,line,MAX_LINE_LENGTH);
            trimmed[MAX_LINE_LENGTH-1]='\0';
            TrimWhiteSpace(trimmed);
            if(trimmed[0]=='\0' || trimmed[0]==COMMENT_CHAR)
                continue;
            if(strstr(trimmed,".data")||strstr(trimmed,".string")){
                encodeDataLine(trimmed, machineWords, &mWordIndex, wordTypes, &addressCounter);
            } else {
                /* treat it as instruction => pass extRefs so we can record extern usage */
                encodeInstructionLine(trimmed,
                                      machineWords,
                                      &mWordIndex,
                                      &labelTable,
                                      wordTypes,
                                      IC,
                                      &addressCounter,
                                      &extRefs);
                int used = countInstructionWords(trimmed);
                IC+=used;
            }
        }
    }
    fclose(codeFile);

    /* PASS 8: Print final machine code to console (for debug) */
    printf("----------\nMachine Code Array (24-bit binary + breakdown):\n");
    for(int i=0; i<mWordIndex; i++){
        printf("Address %d: %s", machineWords[i].address, machineWords[i].bin);
        if(wordTypes[i]==COMMAND_WORD && machineWords[i].mnemonic[0]){
            printf("  (Mnemonic: %s)", machineWords[i].mnemonic);
        }
        printf("\n");
    }

    /************************************************************
     * Finally: create the three output files in "test" folder
     ************************************************************/

    /* 1) test/1.ps.ob => decimal address + 24-bit word in hex. */
    {
        FILE *objFile=fopen("test/1.ps.ob","w");
        if(objFile) {
            for(int i=0; i<mWordIndex; i++){
                char *endptr;
                uint32_t val = (uint32_t)strtoul(machineWords[i].bin, &endptr, 2);
                fprintf(objFile, "%d %06X\n", machineWords[i].address, val);
            }
            fclose(objFile);
        } else {
            fprintf(stderr,"Cannot open test/1.ps.ob for writing.\n");
        }
    }

    /* 2) test/1.ps.ext => one line for each usage of an external label: "labelName address" */
    {
        writeExtFile("test/1.ps.ext", &extRefs);
        /* That function writes e.g.: MY_EXTERN 0000130 for each usage. */
    }

    /* 3) test/1.ps.ent => each entry label, once. */
    {
        FILE *entFile=fopen("test/1.ps.ent","w");
        if(entFile) {
            for(size_t i=0; i<labelTable.count; i++){
                if(labelTable.labels[i].isEntry){
                    fprintf(entFile, "%s %d\n", 
                            labelTable.labels[i].name,
                            labelTable.labels[i].address);
                }
            }
            fclose(entFile);
        } else {
            fprintf(stderr,"Cannot open test/1.ps.ent for writing.\n");
        }
    }

    /* Cleanup */
    freeMacroArray(&macroArray);
    freeDataImage(&dataImage);
    freeLabelTable(&labelTable);
    freeExtRefArray(&extRefs);
    free(machineWords);
    free(wordTypes);

    printf("Done.\n");
    return 0;
}
