#include "../include/parser.h"
#include "../include/macro.h"
#include "../include/commands.h"
#include "../include/label.h"
#include "../include/definitions.h"
#include "../include/data.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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
    return 0;
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

int parseDataDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".data");
    if (!p) return -1; // Not a data directive
    p += 5; // Move past ".data"
    
    while (isspace((unsigned char)*p)) {
        p++;
    }
    
    while (*p != '\0' && *p != '\n') {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        
        if (*p == '\0' || *p == '\n') break;
        
        errno = 0;
        char *endptr;
        int value = (int)strtol(p, &endptr, 10);
        if (p == endptr) {
            fprintf(stderr, "Error: invalid number in .data directive: %s\n", p);
            return -1;
        }
        if (errno != 0) {
            perror("strtol");
            return -1;
        }
        
        if (addDataValue(data, value) != 0) {
            fprintf(stderr, "Error: unable to add data value %d\n", value);
            return -1;
        }
        
        p = endptr;
        while (isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == ',') {
            p++;
        }
    }
    return 0;
}

int parseStringDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".string");
    if (!p) return -1; // Not a string directive
    p += strlen(".string");
    
    while (isspace((unsigned char)*p)) {
        p++;
    }
    
    // Accept either ASCII double quote (") or left curly quote (“)
    int open_quote_len = 0;
    if (strncmp(p, "\"", 1) == 0) {
        open_quote_len = 1;
    } else if (strncmp(p, "\xE2\x80\x9C", 3) == 0) { // UTF-8 for “
        open_quote_len = 3;
    } else {
        fprintf(stderr, "Error: .string directive missing opening quote\n");
        return -1;
    }
    p += open_quote_len;
    
    int closing_found = 0;
    while (*p) {
        if (strncmp(p, "\"", 1) == 0) {
            p += 1;
            closing_found = 1;
            break;
        } else if (strncmp(p, "\xE2\x80\x9D", 3) == 0) { // UTF-8 for ”
            p += 3;
            closing_found = 1;
            break;
        }
        if (addDataValue(data, (int)*p) != 0) {
            fprintf(stderr, "Error adding string character %c\n", *p);
            return -1;
        }
        p++;
    }
    
    if (!closing_found) {
        fprintf(stderr, "Error: .string directive missing closing quote\n");
        return -1;
    }
    
    if (addDataValue(data, 0) != 0) {
        fprintf(stderr, "Error adding terminating 0 for string\n");
        return -1;
    }
    
    return 0;
}



int parseStringDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".string");
    if (!p) return -1;  // Not a .string directive
    p += strlen(".string");
    
    while (isspace((unsigned char)*p)) {
        p++;
    }
    
    // Check for opening quote (allowing standard " or curly “)
    if (*p != '"' && strncmp(p, "\xE2\x80\x9C", 3) != 0) {
        fprintf(stderr, "Error: .string directive missing opening quote\n");
        return -1;
    }
    if (*p == '"') {
        p++;
    } else {
        p += 3;
    }
    
    // Process characters until closing quote (allow standard " or curly ”)
    while (*p && *p != '"' && strncmp(p, "\xE2\x80\x9D", 3) != 0) {
        if (addDataValue(data, (int)*p) != 0) {
            fprintf(stderr, "Error adding character '%c'\n", *p);
            return -1;
        }
        p++;
    }
    
    if (*p == '"') {
        p++;
    } else if (strncmp(p, "\xE2\x80\x9D", 3) == 0) {
        p += 3;
    } else {
        fprintf(stderr, "Error: .string directive missing closing quote\n");
        return -1;
    }
    
    // Append terminating 0
    if (addDataValue(data, 0) != 0) {
        fprintf(stderr, "Error adding terminating 0 for string\n");
        return -1;
    }
    
    return 0;
}
