#include "../include/definitions.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"
#include "../include/label.h"
#include "../include/data.h"
#include "../include/macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper: Check if a token is a register (e.g. "r0", "r1", etc.)
int isRegister(const char *token) {
    if (token[0] == 'r' && strlen(token) == 2 && isdigit(token[1])) {
        return 1;
    }
    return 0;
}

// Helper: Count the number of instruction words in a line.
// The first word is always the opcode. For each subsequent operand,
// if it is an immediate (starts with '#') or not a register, count an extra word.
int countInstructionWords(const char *line) {
    // Make a copy because we will tokenize
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';

    // Skip label definition if present (look for colon)
    char *colon = strchr(temp, ':');
    char *instr = temp;
    if (colon) {
        instr = colon + 1; // assume the instruction comes after the label
    }
    // Trim leading whitespace
    while (isspace((unsigned char)*instr)) instr++;

    // Tokenize by whitespace and comma
    int wordCount = 0;
    char *token = strtok(instr, " \t,\n");
    if (!token) return 0;
    // First token is the opcode (always one word)
    wordCount = 1;
    while ((token = strtok(NULL, " \t,\n")) != NULL) {
        // If the operand is an immediate or not a register, count an extra word.
        if (token[0] == '#' || !isRegister(token)) {
            wordCount++;
        }
    }
    return wordCount;
}

// Helper: Count number of data words in a .data directive.
// Here, we assume comma-separated numbers.
int countDataWords(const char *line) {
    const char *p = strstr(line, ".data");
    if (!p) return 0;
    p += 5; // skip ".data"
    int count = 0;
    while (*p != '\0' && *p != '\n') {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '\n')
            break;
        // Skip the number (we assume valid numbers)
        strtol(p, (char**)&p, 10);
        count++;
        while (isspace((unsigned char)*p)) p++;
        if (*p == ',') p++;
    }
    return count;
}

// Helper: Count number of words in a .string directive.
// Each character (plus a terminating 0) counts as one word.
int countStringWords(const char *line) {
    const char *p = strstr(line, ".string");
    if (!p) return 0;
    p += strlen(".string");
    while (isspace((unsigned char)*p)) p++;
    if (*p != '"' && strncmp(p, "\xE2\x80\x9C", 3) != 0)
        return 0; // error: missing opening quote
    // Advance past opening quote
    if (*p == '"')
        p++;
    else
        p += 3;
    int count = 0;
    while (*p && *p != '"' && strncmp(p, "\xE2\x80\x9D", 3) != 0) {
        count++;
        p++;
    }
    // Plus one for the terminating 0
    return count + 1;
}

int main(void) {
    printf("Starting program...\n");
    char line[MAX_LINE_LENGTH];
    
    /* ---------- Print Input File ---------- */
    printf("----------\nInput file content:\n");
    FILE *inputFile = fopen("test/test1.asm", "r");
    if (!inputFile) {
        fprintf(stderr, "Error opening input file 'test/test1.asm'\n");
        return STATUS_CATASTROPHIC;
    }
    while (fgets(line, MAX_LINE_LENGTH, inputFile)) {
        printf("%s", line);
    }
    fclose(inputFile);
    
    /* ---------- Process Macros ---------- */
    MacroArray macroArray;
    if (initMacroArray(&macroArray) != 0) {
        fprintf(stderr, "Error initializing macro array\n");
        return STATUS_CATASTROPHIC;
    }
    int result = ParseMacrosDynamic("test/test1.asm", &macroArray);
    if (result != 0) {
        printf("ERROR: ParseMacrosDynamic returned %d\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }
    /* Delete macro definitions and expand macros as before */
    result = DeleteMacroDefinitions("test/test1.asm", "test/temp.am");
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
    
    /* ---------- Print Expanded File ---------- */
    FILE *expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        printf("ERROR: Could not open output file 'test/test2.am'.\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    printf("----------\nExpanded file content:\n");
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        printf("%s", line);
    }
    fclose(expandedFile);
    
    /* ---------- First Pass: Count Instruction and Data Words ---------- */
    int totalInstWords = 0;
    int totalDataWords = 0;
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file for first pass\n");
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        // Skip empty or comment lines
        char trimmed[MAX_LINE_LENGTH];
        strncpy(trimmed, line, MAX_LINE_LENGTH);
        trimmed[MAX_LINE_LENGTH - 1] = '\0';
        // (Assume TrimWhiteSpace is available)
        TrimWhiteSpace(trimmed);
        if (trimmed[0] == '\0' || trimmed[0] == COMMENT_CHAR)
            continue;
        if (strstr(trimmed, ".data") != NULL) {
            totalDataWords += countDataWords(trimmed);
        }
        else if (strstr(trimmed, ".string") != NULL) {
            totalDataWords += countStringWords(trimmed);
        }
        else if (isInstructionLine(trimmed)) {
            totalInstWords += countInstructionWords(trimmed);
        }
    }
    fclose(expandedFile);
    
    // Set starting addresses:
    // Instructions start at 100, data start after instructions.
    int instStart = 100;
    int dataStart = instStart + totalInstWords;
    
    /* ---------- Second Pass: Build Label Table with Assigned Addresses ---------- */
    DataImage dataImage;
    LabelTable labelTable;
    // (Initialize dataImage and labelTable as before)
    if (initDataImage(&dataImage) != 0) {
        fprintf(stderr, "Error initializing data image\n");
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    if (initLabelTable(&labelTable) != 0) {
        fprintf(stderr, "Error initializing label table\n");
        freeDataImage(&dataImage);
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    
    // Reset file pointer for second pass.
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file for second pass\n");
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    int currentInstAddr = instStart;
    int currentDataAddr = dataStart;
    int lineNumber = 0;
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        lineNumber++;
        // Make a working copy.
        char workLine[MAX_LINE_LENGTH];
        strncpy(workLine, line, MAX_LINE_LENGTH);
        workLine[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(workLine);
        if (workLine[0] == '\0' || workLine[0] == COMMENT_CHAR)
            continue;
        
        // Check if a label is defined (look for a colon).
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
        
        // Determine if the line is a data directive or an instruction.
        if (strstr(workLine, ".data") != NULL) {
            // If a label is defined on this line, assign it the current data address.
            if (hasLabel) {
                addLabel(&labelTable, labelName, currentDataAddr, 0, 0);
            }
            int numWords = countDataWords(workLine);
            // (Optionally, parse and store the data values in dataImage here.)
            currentDataAddr += numWords;
        }
        else if (strstr(workLine, ".string") != NULL) {
            if (hasLabel) {
                addLabel(&labelTable, labelName, currentDataAddr, 0, 0);
            }
            int numWords = countStringWords(workLine);
            currentDataAddr += numWords;
        }
        else if (isInstructionLine(workLine)) {
            // For instruction lines, the label (if any) gets the current instruction address.
            if (hasLabel) {
                addLabel(&labelTable, labelName, currentInstAddr, 0, 0);
            }
            int numWords = countInstructionWords(workLine);
            currentInstAddr += numWords;
        }
        // Also process directives for .entry or .extern if present.
        // (Assuming those tokens are separate from labels, you can use your updateLabelDirective function.)
        // For simplicity, they are not repeated here.
    }
    fclose(expandedFile);
    
    /* ---------- Final Output ---------- */
    printf("----------\nLabel Table (with assigned addresses):\n");
    printLabelTable(&labelTable, stdout);
    
    printf("----------\nStored .data/.string values:\n");
    for (size_t i = 0; i < dataImage.count; i++) {
        printf("%d ", dataImage.values[i]);
    }
    printf("\n");
    
    /* Generate output files for .entry and .extern as before */
    if (writeEntryFile("test/test2.ent", &labelTable) != 0) {
        fprintf(stderr, "Error writing entry file\n");
    }
    if (writeExternFile("test/test2.ext", &labelTable) != 0) {
        fprintf(stderr, "Error writing extern file\n");
    }
    
    freeLabelTable(&labelTable);
    freeDataImage(&dataImage);
    freeMacroArray(&macroArray);
    printf("Program finished.\n");
    return EXIT_SUCCESS;
}
