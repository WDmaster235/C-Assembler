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
        for (size_t j = 0; j < macroArray.macros[i].line_count; j++) {
            printf("%s", macroArray.macros[i].body[j]);
        }
    }
    
    /* ---------- Delete Macro Definitions from Input File ---------- */
    result = DeleteMacroDefinitions("test/test1.asm", "test/temp.am");
    if (result != 0) {
        printf("ERROR: DeleteMacroDefinitions returned %d\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }
    
    /* ---------- Expand Macros (on the file with macro definitions removed) ---------- */
    result = ExpandMacros("test/temp.am", "test/test2.am", &macroArray);
    if (result != 0) {
        printf("ERROR during macro expansion (code %d)\n", result);
        freeMacroArray(&macroArray);
        return 1;
    }
    printf("----------\nExpanded file content:\n");
    FILE *expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        printf("ERROR: Could not open output file 'test/test2.am'.\n");
        freeMacroArray(&macroArray);
        return 1;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        printf("%s", line);
    }
    fclose(expandedFile);
    
    /* ---------- Process Expanded File for Labels, Directives, and Data ----------
       We use two separate counters:
         - codeCounter: for instructions (code labels)
         - dataImage.count: for data directives (data labels)
    */
    DataImage dataImage;
    LabelTable labelTable;
    int lineNumber = 0;
    int codeCounter = 0;   // Instruction (code) counter

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
    
    /* First pass: Use ParseLabels to add label declarations (and process .entry/.extern)
       into the label table. This adds labels with an initial address (currently set to the line number). */
    if (ParseLabels("test/test2.am", &labelTable) != 0) {
        fprintf(stderr, "Error parsing labels and directives\n");
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    
    /* Second pass: Re-scan the expanded file to update label addresses based on our counters.
       - If a line contains a .data or .string directive, update the label's address to dataImage.count.
       - Otherwise, update it to the current codeCounter (which is incremented for each instruction line).
    */
    expandedFile = fopen("test/test2.am", "r");
    if (!expandedFile) {
        fprintf(stderr, "Error opening expanded file for updating labels\n");
        freeDataImage(&dataImage);
        freeLabelTable(&labelTable);
        freeMacroArray(&macroArray);
        return STATUS_CATASTROPHIC;
    }
    while (fgets(line, MAX_LINE_LENGTH, expandedFile)) {
        lineNumber++;
        /* Process .data and .string directives */
        if (strstr(line, ".data") != NULL) {
            if (parseDataDirective(line, &dataImage) != 0) {
                fprintf(stderr, "Failed to parse .data directive on line %d\n", lineNumber);
            }
        }
        if (strstr(line, ".string") != NULL) {
            if (parseStringDirective(line, &dataImage) != 0) {
                fprintf(stderr, "Failed to parse .string directive on line %d\n", lineNumber);
            }
        }
        /* Increment codeCounter if the line is an instruction */
        if (isInstructionLine(line)) {
            codeCounter++;
        }
        /* If the line defines a label (contains ':'), update its address in the label table */
        char *colon = strchr(line, ':');
        if (colon) {
            char labelName[MAX_SYMBOL_NAME];
            size_t labelLen = colon - line;
            if (labelLen < MAX_SYMBOL_NAME) {
                strncpy(labelName, line, labelLen);
                labelName[labelLen] = '\0';
                if (strstr(line, ".data") != NULL || strstr(line, ".string") != NULL) {
                    Label *lbl = findLabel(&labelTable, labelName);
                    if (lbl) {
                        lbl->address = dataImage.count;
                    }
                } else {
                    Label *lbl = findLabel(&labelTable, labelName);
                    if (lbl) {
                        lbl->address = codeCounter;
                    }
                }
            }
        }
    }
    fclose(expandedFile);
    
    /* ---------- Final Output ---------- */
    printf("----------\nLabel Table:\n");
    printLabelTable(&labelTable, stdout);
    
    printf("----------\nStored .data/.string values:\n");
    for (size_t i = 0; i < dataImage.count; i++) {
        printf("%d ", dataImage.values[i]);
    }
    printf("\n");
    
    /* Generate output files for labels marked as .entry and .extern (as required by the specification) */
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
