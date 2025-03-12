#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"
#include "../include/label.h"
#include "../include/data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(void) {
    printf("Starting program...\n");

    Macro macros[MAX_MACROS] = {0};
    size_t count = 0;
    DataImage dataImage;
    LabelTable labelTable;
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;

    /* Initialize DataImage and LabelTable */
    if (initDataImage(&dataImage) != 0) {
        fprintf(stderr, "Error initializing data image\n");
        return STATUS_CATASTROPHIC;
    }
    if (initLabelTable(&labelTable) != 0) {
        fprintf(stderr, "Error initializing label table\n");
        return STATUS_CATASTROPHIC;
    }

    printf("Expanding macros...\n");
    int result = ParseMacros("test/test1.asm", macros, &count);
    if (result != 0) {
        printf("ERROR: ExpandMacros returned %d\n", result);
        return 1;
    }

    printf("Macro expansion complete. Count: %zu\n", count);
    printf("----------\n");
    for (size_t i = 0; i < count; i++) {
        printf("Macro %zu: %s\n", i, macros[i].name);
        for (size_t j = 0; j < macros[i].line_count; j++) {
            printf("%s", macros[i].body[j]);
        }
    }
    
    printf("----------\n");
    result = ExpandMacros("test/test1.asm", "test/test2.asm", macros, &count);
    if (result != 0) {
        printf("ERROR during macro expansion\n");
    } else {
        FILE *output_fd = fopen("test/test2.asm", "r");
        if (output_fd == NULL) {
            printf("ERROR: Could not open output file 'test/test2.asm'.\n");
            return 1;
        }
        printf("Expanded file content:\n");
        while (fgets(line, MAX_LINE_LENGTH, output_fd)) {
            printf("%s", line);
        }
        fclose(output_fd);
    }

    /* Process the expanded file to check both labels and directives (.data and .string) */
    FILE *input_fp = fopen("test/test2.asm", "r");
    if (!input_fp) {
        fprintf(stderr, "Error opening input file\n");
        return STATUS_CATASTROPHIC;
    }

    while (fgets(line, MAX_LINE_LENGTH, input_fp)) {
        lineNumber++;
        /* First, if the line defines a label, add it.
           For lines with a directive (.data or .string), the label gets the current dataImage.count as its address. */
        char *colon = strchr(line, ':');
        if (colon) {
            char labelName[MAX_SYMBOL_NAME];
            size_t labelLen = colon - line;
            if (labelLen < MAX_SYMBOL_NAME) {
                strncpy(labelName, line, labelLen);
                labelName[labelLen] = '\0';
                if (strstr(line, ".data") != NULL || strstr(line, ".string") != NULL) {
                    addLabel(&labelTable, labelName, dataImage.count, 0, 0);
                } else {
                    addLabel(&labelTable, labelName, lineNumber, 0, 0);
                }
            }
        }
        /* Process directives */
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
    }
    fclose(input_fp);

    printf("----------\nLabel Table:\n");
    printLabelTable(&labelTable, stdout);

    printf("----------\nStored .data/.string values:\n");
    for (size_t i = 0; i < dataImage.count; i++) {
        printf("%d ", dataImage.values[i]);
    }
    printf("\n");

    freeLabelTable(&labelTable);
    freeDataImage(&dataImage);
    printf("Program finished.\n");
    return EXIT_SUCCESS;
}
