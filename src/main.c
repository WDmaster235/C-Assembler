/************************************************************
 * main.c: Assembler driver rewritten for clarity and correctness.
 * This version prints:
 *   - The input file content,
 *   - The expanded file content,
 *   - The machine code array (24-bit binary) with breakdown.
 *
 * For command words, the 24-bit word is arranged (from MSB to LSB) as:
 *   Bits 23-18: opcode (6 bits)
 *   Bits 17-16: source addressing mode (2 bits)
 *   Bits 15-13: source register (3 bits)
 *   Bits 12-11: destination addressing mode (2 bits)
 *   Bits 10-8:  destination register (3 bits)
 *   Bits 7-3:   funct (5 bits)
 *   Bits 2-0:   ARE (3 bits)  <-- forced to A (binary 100)
 *
 * For non-command words (data words and extra words for immediates/labels),
 * the full 24 bits are printed then split into:
 *   Space: first 21 bits,  ARE: last 3 bits.
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

#define MAX_MNEMONIC_LENGTH 20

// Helper: print a substring without copying.
static void printSubstring(const char *s, int start, int length) {
    for (int i = start; i < start + length; i++) {
        putchar(s[i]);
    }
}

// Returns nonzero if the line (after trimming) is an instruction.
static int isInstructionLine(const char *line) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    TrimWhiteSpace(temp);
    if (temp[0] == '\0' || temp[0] == COMMENT_CHAR)
        return 0;
    if (strstr(temp, ".data") || strstr(temp, ".string"))
        return 0;
    return 1;
}

// Returns nonzero if the token represents a register (r0..r7).
static int isRegister(const char *token) {
    return (token[0] == 'r' && strlen(token) == 2 && isdigit(token[1]));
}

// Count words produced by a .string directive.
static int countStringWords(const char *line) {
    if (strstr(line, ".string")) {
        const char *p = strstr(line, ".string") + 7;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '"')
            p++;
        int count = 0;
        while (*p && *p != '"') {
            count++;
            p++;
        }
        return count + 1; // include terminating null
    }
    return 0;
}

// Count words produced by an instruction line.
static int countInstructionWords(const char *line) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    TrimWhiteSpace(instr);
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

// Count words produced by a .data or .string directive.
static int countDataWords(const char *line) {
    if (strstr(line, ".data")) {
        const char *p = strstr(line, ".data") + 5;
        int count = 0;
        while (*p) {
            while (isspace((unsigned char)*p)) p++;
            if (*p == '\0' || *p == '\n')
                break;
            strtol(p, (char**)&p, 10);
            count++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == ',') p++;
        }
        return count;
    } else if (strstr(line, ".string")) {
        return countStringWords(line);
    }
    return 0;
}

// Machine word type enumeration.
typedef enum {
    DATA_WORD,
    COMMAND_WORD
} WordType;

// Machine word structure.
typedef struct {
    char bin[25];                       // 24-bit binary string.
    char mnemonic[MAX_MNEMONIC_LENGTH]; // Non-empty for command words.
} MachineWord;

// Convert an integer to a 24-bit binary string.
static void intToBinary(uint32_t value, int bits, char *dest) {
    dest[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        dest[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

/* Revised encodeDataWord:
   For a signed immediate value stored in 21 bits (with top 3 bits zero),
   if negative, add (1 << 21) to produce proper two's complement.
*/
static void encodeDataWord(int value, char *bin) {
    if (value < 0)
        value += (1 << 21);
    intToBinary((uint32_t)value, 24, bin);
}

/* encodeCommandWord:
   Constructs a 24-bit command word with fields (MSB to LSB):
     - opcode (6 bits)
     - source addressing mode (2 bits)
     - source register (3 bits)
     - destination addressing mode (2 bits)
     - destination register (3 bits)
     - funct (5 bits)
     - ARE (3 bits)   <-- forced to A (binary 100)
*/
static void encodeCommandWord(const CommandWord *cmd, int srcReg, int srcAddrType,
                              int dstReg, int dstAddrType, char *bin) {
    uint32_t cw = 0;
    cw |= (cmd->opcode & 0x3F) << 18;
    cw |= (srcAddrType & 0x3) << 16;
    cw |= (srcReg & 0x7) << 13;
    cw |= (dstAddrType & 0x3) << 11;
    cw |= (dstReg & 0x7) << 8;
    cw |= (cmd->funct & 0x1F) << 3;
    cw |= A;  // Force ARE to A (binary 100)
    intToBinary(cw, 24, bin);
}

/* encodeInstructionLine:
   Parses an instruction line, distinguishes one- vs two-operand commands via
   the oneOperand flag in CommandWord. For one-operand commands, forces source
   addressing mode and register to 0. Extra machine words are generated for
   non-register operands.
*/
static void encodeInstructionLine(const char *line, MachineWord *words, int *wordIndex,
                                  LabelTable *lblTable, WordType *types) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    TrimWhiteSpace(instr);
    char *mnemonic = strtok(instr, " \t,\n");
    if (!mnemonic) return;
    const CommandWord *cmdDef = NULL;
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(mnemonic, operations[i]) == 0) {
            cmdDef = commands[i];
            break;
        }
    }
    if (!cmdDef) return;
    int oneOperand = cmdDef->oneOperand;
    char *operand1 = strtok(NULL, " \t,\n");
    char *operand2 = oneOperand ? NULL : strtok(NULL, " \t,\n");
    int srcAddr = 0, srcReg = 0, dstAddr = 0, dstReg = 0;
    if (operand1) {
        if (operand1[0] == '#') {
            srcAddr = ADDRESS_IMMEDIATE;
            srcReg = 0;
        } else if (isRegister(operand1)) {
            srcAddr = ADDRESS_REGISTER;
            srcReg = operand1[1] - '0';
        } else if (operand1[0] == '&') {
            srcAddr = ADDRESS_RELATIVE;
            srcReg = 0;
        } else {
            srcAddr = ADDRESS_DIRECT;
            srcReg = 0;
        }
    }
    if (!oneOperand && operand2) {
        if (operand2[0] == '#') {
            dstAddr = ADDRESS_IMMEDIATE;
            dstReg = 0;
        } else if (isRegister(operand2)) {
            dstAddr = ADDRESS_REGISTER;
            dstReg = operand2[1] - '0';
        } else if (operand2[0] == '&') {
            dstAddr = ADDRESS_RELATIVE;
            dstReg = 0;
        } else {
            dstAddr = ADDRESS_DIRECT;
            dstReg = 0;
        }
    } else {
        dstAddr = 0;
        dstReg = 0;
    }
    // For one-operand commands, force source addressing and register to zero.
    if (oneOperand) {
        srcAddr = 0;
        srcReg = 0;
    }
    char bin[25];
    encodeCommandWord(cmdDef, srcReg, srcAddr, dstReg, dstAddr, bin);
    strncpy(words[*wordIndex].mnemonic, mnemonic, MAX_MNEMONIC_LENGTH - 1);
    words[*wordIndex].mnemonic[MAX_MNEMONIC_LENGTH - 1] = '\0';
    strcpy(words[*wordIndex].bin, bin);
    types[*wordIndex] = COMMAND_WORD;
    (*wordIndex)++;
    // For non-register operands, add an extra machine word.
    if (operand1 && (srcAddr == ADDRESS_IMMEDIATE || srcAddr == ADDRESS_DIRECT || srcAddr == ADDRESS_RELATIVE)) {
        int value = 0;
        if (srcAddr == ADDRESS_IMMEDIATE)
            value = atoi(operand1 + 1);
        else
            value = (findLabel(lblTable, operand1) ? findLabel(lblTable, operand1)->address : 0);
        encodeDataWord(value, bin);
        words[*wordIndex].mnemonic[0] = '\0';
        strcpy(words[*wordIndex].bin, bin);
        types[*wordIndex] = COMMAND_WORD;
        (*wordIndex)++;
    }
    if (!oneOperand && operand2 && (dstAddr == ADDRESS_IMMEDIATE || dstAddr == ADDRESS_DIRECT || dstAddr == ADDRESS_RELATIVE)) {
        int value = 0;
        if (dstAddr == ADDRESS_IMMEDIATE)
            value = atoi(operand2 + 1);
        else
            value = (findLabel(lblTable, operand2) ? findLabel(lblTable, operand2)->address : 0);
        encodeDataWord(value, bin);
        words[*wordIndex].mnemonic[0] = '\0';
        strcpy(words[*wordIndex].bin, bin);
        types[*wordIndex] = COMMAND_WORD;
        (*wordIndex)++;
    }
}

static void encodeDataLine(const char *line, MachineWord *words, int *wordIndex,
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
            words[*wordIndex].mnemonic[0] = '\0';
            types[*wordIndex] = DATA_WORD;
            (*wordIndex)++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == ',') p++;
        }
    } else if (strstr(line, ".string")) {
        char *start = strchr(line, '"');
        if (!start) return;
        start++;
        while (*start && *start != '"') {
            char bin[25];
            encodeDataWord((int)*start, bin);
            strcpy(words[*wordIndex].bin, bin);
            words[*wordIndex].mnemonic[0] = '\0';
            types[*wordIndex] = DATA_WORD;
            (*wordIndex)++;
            start++;
        }
        char bin[25];
        encodeDataWord(0, bin);
        strcpy(words[*wordIndex].bin, bin);
        words[*wordIndex].mnemonic[0] = '\0';
        types[*wordIndex] = DATA_WORD;
        (*wordIndex)++;
    }
}

static const char *testInputFile = "test/test1.asm";

int main(void) {
    printf("Starting program.\n");
    
    // 1) Print input file content
    printf("----------\nInput file content:\n");
    FILE *inputFile = fopen(testInputFile, "r");
    if (!inputFile) {
        fprintf(stderr, "Error opening input file %s\n", testInputFile);
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, inputFile))
        printf("%s", line);
    fclose(inputFile);
    
    // 2) Process macros
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
    
    // 3) Print expanded file content
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
    
    // 4) First pass: count total words.
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
        if (trimmed[0] == '\0' || trimmed[0] == COMMENT_CHAR)
            continue;
        if (strstr(trimmed, ".data") || strstr(trimmed, ".string"))
            totalDataWords += countDataWords(trimmed);
        else if (isInstructionLine(trimmed))
            totalInstWords += countInstructionWords(trimmed);
    }
    fclose(expandedFile);
    int totalWordCount = totalInstWords + totalDataWords;
    
    // 5) Second pass: build label table.
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
        if (workLine[0] == '\0' || workLine[0] == COMMENT_CHAR)
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
            if (hasLabel)
                addLabel(&labelTable, labelName, dataStart, 0, 0);
            int n = (strstr(workLine, ".data")) ? countDataWords(workLine) : countStringWords(workLine);
            dataStart += n;
        } else if (isInstructionLine(workLine)) {
            if (hasLabel)
                addLabel(&labelTable, labelName, instStart, 0, 0);
            int n = countInstructionWords(workLine);
            instStart += n;
        }
    }
    fclose(expandedFile);
    
    // 6) Third pass: produce a clean file with no label definitions.
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
    
    // 7) Fourth pass: encode instructions and data.
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
        if (trimmed[0] == '\0' || trimmed[0] == COMMENT_CHAR)
            continue;
        if (strstr(trimmed, ".data") || strstr(trimmed, ".string"))
            encodeDataLine(trimmed, machineWords, &wordIndex, wordTypes);
        else if (isInstructionLine(trimmed))
            encodeInstructionLine(trimmed, machineWords, &wordIndex, &labelTable, wordTypes);
    }
    fclose(codeFile);
    
    // 8) Print machine code array with breakdown.
    printf("----------\nMachine Code Array (24-bit binary + breakdown):\n");
    for (int i = 0; i < wordIndex; i++) {
        // Print the full 24-bit binary word.
        printf("%s", machineWords[i].bin);
        if (strlen(machineWords[i].mnemonic) > 0)
            printf("  (Mnemonic: %s)", machineWords[i].mnemonic);
        printf("   -->  ");
        // If mnemonic is non-empty, treat as a command word.
        if (strlen(machineWords[i].mnemonic) > 0) {
            printf("opcode: ");
            printSubstring(machineWords[i].bin, 0, 6);
            printf("  src_addr: ");
            printSubstring(machineWords[i].bin, 6, 2);
            printf("  src_reg: ");
            printSubstring(machineWords[i].bin, 8, 3);
            printf("  dst_addr: ");
            printSubstring(machineWords[i].bin, 11, 2);
            printf("  dst_reg: ");
            printSubstring(machineWords[i].bin, 13, 3);
            printf("  funct: ");
            printSubstring(machineWords[i].bin, 16, 5);
            printf("  ARE: ");
            printSubstring(machineWords[i].bin, 21, 3);
        } else {
            // "No Command" words: split into Space and ARE.
            printf("Space: ");
            printSubstring(machineWords[i].bin, 0, 21);
            printf("  ARE: ");
            printSubstring(machineWords[i].bin, 21, 3);
        }
        printf("\n");
    }
    
    free(machineWords);
    free(wordTypes);
    
    // 9) Print label table.
    printf("----------\nLabel Table (with assigned addresses):\n");
    printLabelTable(&labelTable, stdout);
    
    freeDataImage(&dataImage);
    freeLabelTable(&labelTable);
    freeMacroArray(&macroArray);
    printf("Program finished.\n");
    return 0;
}
