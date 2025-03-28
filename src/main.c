#include "../include/definitions.h"
#include "../include/preassembler.h"
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
    
<<<<<<< HEAD
    /* ---------- Parse Macros into Dynamic Array ---------- */
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
    printf("----------\nFound %zu macros.\n", macroArray.count);
    for (size_t i = 0; i < macroArray.count; i++) {
        printf("Macro %zu: %s\n", i, macroArray.macros[i].name);
    }
    
    /* ---------- Expand Macros ---------- */
    result = ExpandMacros("test/test1.asm", "test/test2.asm", &macroArray);
    if (result != 0) {
        printf("ERROR during macro expansion (code %d)\n", result);
        freeMacroArray(&macroArray);
=======
    /* ---------- Count Macros ---------- */
    Macro macros[MAX_MACROS] = {0};
    size_t macro_count = 0;
    int result = ParseMacros("test/test1.asm", macros, &macro_count);
    if (result != 0) {
        printf("ERROR: ParseMacros returned %d\n", result);
        return 1;
    }
    printf("----------\nFound %zu macros.\n", macro_count);
    
    /* ---------- Expand Macros ---------- */
    result = ExpandMacros("test/test1.asm", "test/test2.asm");
    if (result != 0) {
        printf("ERROR during macro expansion (code %d)\n", result);
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        return 1;
    }
    printf("----------\nExpanded file content:\n");
    FILE *expandedFile = fopen("test/test2.asm", "r");
    if (!expandedFile) {
        printf("ERROR: Could not open output file 'test/test2.asm'.\n");
<<<<<<< HEAD
        freeMacroArray(&macroArray);
=======
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        printf("%s", line);
    }
    fclose(expandedFile);
    
    /* ---------- Process Expanded File for Labels & Directives ---------- */
    DataImage dataImage;
    LabelTable labelTable;
    int lineNumber = 0;
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
    
    expandedFile = fopen("test/test2.asm", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file\n");
<<<<<<< HEAD
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        freeMacroArray(&macroArray);
=======
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        return STATUS_CATASTROPHIC;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        lineNumber++;
<<<<<<< HEAD
=======
        // If a label is defined on this line
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        char *colon = strchr(line, ':');
        if (colon) {
            char labelName[MAX_SYMBOL_NAME];
            size_t labelLen = colon - line;
            if (labelLen < MAX_SYMBOL_NAME) {
                strncpy(labelName, line, labelLen);
                labelName[labelLen] = '\0';
<<<<<<< HEAD
=======
                // If the line has a directive, set the label's address to the current dataImage.count.
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
                if (strstr(line, ".data") != NULL || strstr(line, ".string") != NULL) {
                    addLabel(&labelTable, labelName, dataImage.count, 0, 0);
                } else {
                    addLabel(&labelTable, labelName, lineNumber, 0, 0);
                }
            }
        }
<<<<<<< HEAD
=======
        // Process .data directive
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        if (strstr(line, ".data") != NULL) {
            if (parseDataDirective(line, &dataImage) != 0) {
                fprintf(stderr, "Failed to parse .data directive on line %d\n", lineNumber);
            }
        }
<<<<<<< HEAD
=======
        // Process .string directive
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        if (strstr(line, ".string") != NULL) {
            if (parseStringDirective(line, &dataImage) != 0) {
                fprintf(stderr, "Failed to parse .string directive on line %d\n", lineNumber);
            }
        }
    }
    fclose(expandedFile);
    
    /* ---------- Final Output ---------- */
<<<<<<< HEAD
    printLabelTable(&labelTable, stdout);

=======
    printf("----------\nLabel Table:\n");
    printLabelTable(&labelTable, stdout);
    
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
    printf("----------\nStored .data/.string values:\n");
    for (size_t i = 0; i < dataImage.count; i++) {
        printf("%d ", dataImage.values[i]);
    }
    printf("\n");
    
    freeLabelTable(&labelTable);
    freeDataImage(&dataImage);
    freeMacroArray(&macroArray);
    printf("Program finished.\n");
    return EXIT_SUCCESS;
}
