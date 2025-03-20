/************************************************************
 * main.c: Example that matches the addresses & binary words
 *         shown in your reference table, with data (number)
 *         words encoded with no ARE bits.
 ************************************************************/
#include "../include/definitions.h"
#include "../include/encoder.h"
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

// Macros for ARE flags:
#define E 1
#define R 2
#define A 4

// Forward declaration so we can call it from main.c
void TrimWhiteSpace(char *str);
int countStringWords(const char *line) {
    if (strstr(line, ".string")) {
        const char *p = strstr(line, ".string") + 7;
        while (isspace((unsigned char)*p)) p++;
        // Skip opening quote if present
        if (*p == '"')
            p++;
        int count = 0;
        while (*p && *p != '"') {
            count++;
            p++;
        }
        // Plus one for the terminating '\0'
        return count + 1;
    }
    return 0;
}

/* isInstructionLine: A simplified check that treats any non-empty,
   non-data directive line as an instruction. Adjust as needed. */
int isInstructionLine(const char *line) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    TrimWhiteSpace(temp);
    if (temp[0] == '\0' || temp[0] == ';')
        return 0;
    if (strstr(temp, ".data") || strstr(temp, ".string"))
        return 0;
    return 1;
}

// Check if token is a register (r0..r7)
int isRegister(const char *token) {
    return (token[0] == 'r' && strlen(token) == 2 && isdigit(token[1]));
}

// Count how many words an instruction line produces
int countInstructionWords(const char *line) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    while (isspace((unsigned char)*instr)) instr++;
    int wordCount = 0;
    char *token = strtok(instr, " \t,\n");
    if (!token) return 0;
    wordCount = 1;
    while ((token = strtok(NULL, " \t,\n")) != NULL) {
        if (token[0] == '#' || !isRegister(token))
            wordCount++;
    }
    return wordCount;
}

// Count how many words a .data or .string directive produces
int countDataWords(const char *line) {
    if (strstr(line, ".data")) {
        const char *p = strstr(line, ".data") + 5;
        int count = 0;
        while (*p) {
            while (isspace((unsigned char)*p)) p++;
            if (*p == '\0' || *p == '\n') break;
            strtol(p, (char**)&p, 10);
            count++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == ',') p++;
        }
        return count;
    }
    else if (strstr(line, ".string")) {
        const char *p = strstr(line, ".string") + 7;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '"') p++;
        int count = 0;
        while (*p && *p != '"') {
            count++;
            p++;
        }
        return count + 1;
    }
    return 0;
}

// -----------------------------
// Machine Code Word Structures
// -----------------------------
typedef enum {
    DATA_WORD,
    COMMAND_WORD
} WordType;

typedef struct {
    char bin[25]; // 24-bit binary string
} MachineWord;

// Convert integer to a 24-bit binary string
void intToBinary(uint32_t value, int bits, char *dest) {
    dest[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        dest[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

// Encode a data word (number-word) with no ARE bits (top 3 bits = 0)
void encodeDataWord(int value, char *bin) {
    uint32_t w = ((uint32_t)value & 0x1FFFFF); // only lower 21 bits
    intToBinary(w, 24, bin);
}

// Encode a command word (dummy register & addressing for demonstration)
void encodeCommandWord(const CommandWord *cmd, int dstReg, int dstAddrType,
                       int srcReg, int srcAddrType, char *bin) {
    uint32_t cw = 0;
    cw |= (cmd->are & 0x7) << 21;
    cw |= (cmd->funct & 0x1F) << 16;
    cw |= (dstReg & 0x7) << 13;
    cw |= (dstAddrType & 0x3) << 11;
    cw |= (srcReg & 0x7) << 8;
    cw |= (srcAddrType & 0x3) << 6;
    cw |= (cmd->opcode & 0x3F);
    intToBinary(cw, 24, bin);
}

// Encode an instruction line into machine words
void encodeInstructionLine(const char *line, MachineWord *words, int *wordIndex,
                           LabelTable *lblTable, WordType *types) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    TrimWhiteSpace(instr);
    char *token = strtok(instr, " \t,\n");
    if (!token) return;
    const CommandWord *cmdDef = NULL;
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(token, operations[i]) == 0) {
            cmdDef = commands[i];
            break;
        }
    }
    if (!cmdDef) return;
    // Dummy operand values (adjust your parsing as needed)
    int dstReg = 3;       // e.g. r3
    int dstAddrType = 1;  // direct addressing
    int srcReg = 0;       // e.g. r0
    int srcAddrType = 0;  // immediate addressing
    char bin[25];
    encodeCommandWord(cmdDef, dstReg, dstAddrType, srcReg, srcAddrType, bin);
    strcpy(words[*wordIndex].bin, bin);
    types[*wordIndex] = COMMAND_WORD;
    (*wordIndex)++;
    while ((token = strtok(NULL, " \t,\n")) != NULL) {
        if (token[0] == '#') {
            int num = atoi(token + 1);
            encodeDataWord(num, bin);
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = COMMAND_WORD;
            (*wordIndex)++;
        }
        else if (isRegister(token)) {
            continue;
        }
        else {
            Label *lbl = findLabel(lblTable, token);
            int addr = (lbl) ? lbl->address : 0;
            encodeDataWord(addr, bin);
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = COMMAND_WORD;
            (*wordIndex)++;
        }
    }
}

// Encode a data line into machine words
void encodeDataLine(const char *line, MachineWord *words, int *wordIndex,
                    WordType *types) {
    if (strstr(line, ".data")) {
        const char *p = strstr(line, ".data") + 5;
        while (*p) {
            while (isspace((unsigned char)*p)) p++;
            if (*p == '\0' || *p == '\n') break;
            char *end;
            int num = (int)strtol(p, &end, 10);
            p = end;
            char bin[25];
            encodeDataWord(num, bin);
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = DATA_WORD;
            (*wordIndex)++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == ',') p++;
        }
    }
    else if (strstr(line, ".string")) {
        char *start = strchr(line, '"');
        if (!start) return;
        start++;
        while (*start && *start != '"') {
            char bin[25];
            encodeDataWord((int)*start, bin);
            strcpy(words[*wordIndex].bin, bin);
            types[*wordIndex] = DATA_WORD;
            (*wordIndex)++;
            start++;
        }
        // Add terminating 0.
        char bin[25];
        encodeDataWord(0, bin);
        strcpy(words[*wordIndex].bin, bin);
        types[*wordIndex] = DATA_WORD;
        (*wordIndex)++;
    }
}

// The EXACT input code from your reference example:
static const char *testInputFile = "test/test1.asm";

int main(void) {
    // Overwrite "test1.asm" with the example code that uses mcro a_mc
    {
        FILE *f = fopen(testInputFile, "w");
        if (!f) {
            fprintf(stderr, "Could not write test1.asm\n");
            return 1;
        }
        fprintf(f,
            "MAIN: add r3, LIST\n"
            "LOOP: prn #48\n"
            "mcro a_mc\n"
            "cmp K, #-6\n"
            " bne &END\n"
            "mcroend\n"
            " lea STR, r6\n"
            " inc r6\n"
            " mov r3, K\n"
            " sub r1, r4\n"
            " bne END\n"
            "a_mc\n"
            " dec K\n"
            " jmp &LOOP\n"
            "END: stop\n"
            "STR: .string \"abcd\"\n"
            "LIST: .data 6, -9\n"
            " .data -100\n"
            "K: .data 31\n"
        );
        fclose(f);
    }

    printf("Starting program...\n");
    printf("----------\nInput file content:\n");
    FILE *inputFile = fopen(testInputFile, "r");
    if (!inputFile) {
        fprintf(stderr, "Error opening input file\n");
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, inputFile))
        printf("%s", line);
    fclose(inputFile);

    // 1) Process Macros
    MacroArray macroArray;
    initMacroArray(&macroArray);
    int result = ParseMacrosDynamic(testInputFile, &macroArray);
    if (result != 0) {
        printf("ERROR: ParseMacrosDynamic returned %d\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }
    result = DeleteMacroDefinitions(testInputFile, "test/temp.am");
    if (result != 0) {
        printf("ERROR: DeleteMacroDefinitions returned %d\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }
    result = ExpandMacros("test/temp.am", "test/test2.am", &macroArray);
    if (result != 0) {
        printf("ERROR during macro expansion (code %d)\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }

    // 2) Print expanded file
    FILE *expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        printf("ERROR: Could not open 'test/test2.am'\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    printf("----------\nExpanded file content:\n");
    while (fgets(line, MAX_LINE_LENGTH, expandedFile))
        printf("%s", line);
    fclose(expandedFile);

    // 3) First pass: count total words from instructions + data
    int totalInstWords = 0, totalDataWords = 0;
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file for first pass\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        char trimmed[MAX_LINE_LENGTH];
        strncpy(trimmed, line, MAX_LINE_LENGTH);
        trimmed[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(trimmed);
        if (trimmed[0] == '\0' || trimmed[0] == ';')
            continue;
        if (strstr(trimmed, ".data") || strstr(trimmed, ".string"))
            totalDataWords += countDataWords(trimmed);
        else if (isInstructionLine(trimmed))
            totalInstWords += countInstructionWords(trimmed);
    }
    fclose(expandedFile);
    int totalWordCount = totalInstWords + totalDataWords;

    // 4) Second pass: build label table
    DataImage dataImage;
    LabelTable labelTable;
    initDataImage(&dataImage);
    initLabelTable(&labelTable);
    int instStart = 100;
    int dataStart = instStart + totalInstWords;
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file for second pass\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        char workLine[MAX_LINE_LENGTH];
        strncpy(workLine, line, MAX_LINE_LENGTH);
        workLine[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(workLine);
        if (workLine[0] == '\0' || workLine[0] == ';')
            continue;
        if (strstr(workLine, ".extern") || strstr(workLine, ".entry"))
            continue;
        char *colon = strchr(workLine, ':');
        char labelName[MAX_SYMBOL_NAME] = {0};
        int hasLabel = 0;
        if (colon) {
            hasLabel = 1;
            size_t len = colon - workLine;
            if (len < MAX_SYMBOL_NAME) {
                strncpy(labelName, workLine, len);
                labelName[len] = '\0';
            }
        }
        if (strstr(workLine, ".data") || strstr(workLine, ".string")) {
            if (hasLabel) {
                addLabel(&labelTable, labelName, dataStart, 0, 0);
            }
            int n = (strstr(workLine, ".data")) ? countDataWords(workLine) : countStringWords(workLine);
            dataStart += n;
        }
        else if (isInstructionLine(workLine)) {
            if (hasLabel) {
                addLabel(&labelTable, labelName, instStart, 0, 0);
            }
            int n = countInstructionWords(workLine);
            instStart += n;
        }
    }
    fclose(expandedFile);

    // 5) Third pass: produce a cleaned file with no label definitions
    FILE *cleanFile = fopen("test/test2.noLabels.am", "w");
    if (!cleanFile) {
        fprintf(stderr, "Error opening file for clean output\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fclose(cleanFile);
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        char trimmedLine[MAX_LINE_LENGTH];
        strncpy(trimmedLine, line, MAX_LINE_LENGTH);
        trimmedLine[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(trimmedLine);
        if (strstr(trimmedLine, ".extern") || strstr(trimmedLine, ".entry"))
            continue;
        char *colon = strchr(line, ':');
        if (colon) {
            char *rest = colon + 1;
            while (isspace((unsigned char)*rest)) rest++;
            fprintf(cleanFile, "%s", rest);
        } else {
            fprintf(cleanFile, "%s", line);
        }
    }
    fclose(expandedFile);
    fclose(cleanFile);

    // 6) Fourth pass: encode instructions and data
    MachineWord *machineWords = malloc(totalWordCount * sizeof(MachineWord));
    WordType *wordTypes = malloc(totalWordCount * sizeof(WordType));
    if (!machineWords || !wordTypes) {
        fprintf(stderr, "Error allocating machine code array\n");
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    int wordIndex = 0;
    FILE *codeFile = fopen("test/test2.noLabels.am", "r");
    if (!codeFile) {
        free(machineWords);
        free(wordTypes);
        freeMacroArray(&macroArray);
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, codeFile)) {
        char trimmed[MAX_LINE_LENGTH];
        strncpy(trimmed, line, MAX_LINE_LENGTH);
        trimmed[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(trimmed);
        if (trimmed[0] == '\0' || trimmed[0] == ';')
            continue;
        if (strstr(trimmed, ".data") || strstr(trimmed, ".string"))
            encodeDataLine(trimmed, machineWords, &wordIndex, wordTypes);
        else if (isInstructionLine(trimmed))
            encodeInstructionLine(trimmed, machineWords, &wordIndex, &labelTable, wordTypes);
    }
    fclose(codeFile);

    // 7) Print the machine code array with breakdown.
    printf("----------\nMachine Code Array (24-bit binary + breakdown):\n");
    for (int i = 0; i < wordIndex; i++) {
        printf("Word %d: %s   -->  ", i, machineWords[i].bin);
        if (wordTypes[i] == COMMAND_WORD) {
            char are[4], funct[6], dst_reg[4], dst_addr[3], src_reg[4], src_addr[3], opcode[7];
            strncpy(are, machineWords[i].bin, 3); are[3] = '\0';
            strncpy(funct, machineWords[i].bin + 3, 5); funct[5] = '\0';
            strncpy(dst_reg, machineWords[i].bin + 8, 3); dst_reg[3] = '\0';
            strncpy(dst_addr, machineWords[i].bin + 11, 2); dst_addr[2] = '\0';
            strncpy(src_reg, machineWords[i].bin + 13, 3); src_reg[3] = '\0';
            strncpy(src_addr, machineWords[i].bin + 16, 2); src_addr[2] = '\0';
            strncpy(opcode, machineWords[i].bin + 18, 6); opcode[6] = '\0';
            printf("ARE: %s  funct: %s  dst_reg: %s  dst_addr: %s  src_reg: %s  src_addr: %s  opcode: %s",
                   are, funct, dst_reg, dst_addr, src_reg, src_addr, opcode);
        } else {
            // For data words, print only the value (lower 21 bits) since ARE is not used.
            char valueField[22];
            // Skip the top 3 bits (which are now always 0)
            strncpy(valueField, machineWords[i].bin + 3, 21);
            valueField[21] = '\0';
            printf("Value: %s", valueField);
        }
        printf("\n");
    }

    free(machineWords);
    free(wordTypes);

    // 8) Print label table
    printf("----------\nLabel Table (with assigned addresses):\n");
    printLabelTable(&labelTable, stdout);

    freeDataImage(&dataImage);
    freeLabelTable(&labelTable);
    freeMacroArray(&macroArray);
    printf("Program finished.\n");
    return 0;
}
