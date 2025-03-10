#include "../include/parser.h"
#include "../include/macro.h"
#include "../include/commands.h"
#include "../include/label.h"
#include "../include/definitions.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Existing functions

void TrimWhiteSpace(char *str) {
    char *end;
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    // Null-terminate the string
    *(end + 1) = '\0';
}

int IsCommandName(char *macro) {
    if (macro == NULL) {
        return 0; // Not a valid macro name
    }
    TrimWhiteSpace(macro);
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(macro, operations[i]) == 0) {
            return 1; // Match found
        }
    }
    return 0; // No match found
}

int IsCommentLine(char *line, size_t line_length) {
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == COMMENT_CHAR) return i;
    }
    return -1;
}

// New function: ParseLabels
// This function opens the expanded file, scans each line for a label definition,
// and adds the label to the given LabelTable.
// For simplicity, a label is assumed to be any token at the beginning of a line ending with ':'.
int ParseLabels(const char *filePath, LabelTable *table) {
    if (!filePath || !table) {
        return STATUS_CATASTROPHIC;
    }
    
    FILE *fp = fopen(filePath, "r");
    if (!fp) {
        return STATUS_CATASTROPHIC;
    }
    
    char line[MAX_LINE_LENGTH];
    int lineCounter = 0;
    
    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        lineCounter++;

        // Trim leading whitespace
        char *trimmed = line;
        while (isspace((unsigned char)*trimmed)) {
            trimmed++;
        }

        // Skip empty lines or comment lines
        if (*trimmed == '\0' || *trimmed == COMMENT_CHAR) {
            continue;
        }

        // Look for a colon that indicates a label definition
        char *colon = strchr(trimmed, LABEL_DELIM);
        if (colon != NULL) {
            // Ensure it's a standalone label (no spaces before colon)
            size_t labelLength = colon - trimmed;
            if (labelLength > 0 && labelLength < MAX_SYMBOL_NAME) {
                char labelName[MAX_SYMBOL_NAME];
                strncpy(labelName, trimmed, labelLength);
                labelName[labelLength] = '\0';

                // Ensure the label name is not a command name
                if (!IsCommandName(labelName)) {
                    printf("Detected label: %s at line %d\n", labelName, lineCounter);
                    int status = addLabel(table, labelName, lineCounter, 0, 0);
                    if (status != 0) {
                        fprintf(stderr, "Warning: Could not add label '%s' (status %d)\n", labelName, status);
                    }
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

