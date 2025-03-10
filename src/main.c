#include "../include/definitions.h"
#include "../include/preassembler.h"
#include "../include/encoder.h"
#include "../include/parser.h"
#include "../include/io.h"
#include "../include/label.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Starting program...\n");

    Macro macros[MAX_MACROS] = {0};
    size_t count = 0;

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
        // Print each line from the expanded file
        FILE *output_fd = fopen("test/test2.asm", "r");
        if (output_fd == NULL) {
            printf("ERROR: Could not open output file 'test/test2.asm'.\n");
            return 1;
        }
        char line[MAX_LINE_LENGTH] = {0};
        printf("Expanded file content:\n");
        while (fgets(line, MAX_LINE_LENGTH, output_fd)) {
            printf("%s", line);
        }
        fclose(output_fd);
    }


    printf("Expanded file content:\n");
    FILE *debug_fp = fopen("test/test2.asm", "r");
    if (debug_fp) {
        char line[MAX_LINE_LENGTH] = {0};
        while (fgets(line, MAX_LINE_LENGTH, debug_fp)) {
            printf("%s", line); // Print each line to debug if labels exist
        }
        fclose(debug_fp);
    }

    // Initialize and parse labels from the expanded file.
    LabelTable labelTable;
    if (initLabelTable(&labelTable) != 0) {
        fprintf(stderr, "Error initializing label table\n");
        exit(STATUS_CATASTROPHIC);
    }
    result = ParseLabels("test/test2.asm", &labelTable);
    if (result != 0) {
        fprintf(stderr, "Error parsing labels (returned %d)\n", result);
        freeLabelTable(&labelTable);
        exit(STATUS_CATASTROPHIC);
    }
    printf("----------\nLabel parsing complete:\n");
    printLabelTable(&labelTable, stdout);
    freeLabelTable(&labelTable);

    printf("Program finished.\n");
    return EXIT_SUCCESS;
}
