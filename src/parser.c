/* parser.c */

#include "label.h"
#include "parser.h"
#include "macro.h"      // For AddMacro, FindMacroDynamic, CleanUpMacro, addMacroToArray, etc.
#include "commands.h"   // For NUM_COMMANDS, operations, COMMENT_CHAR, etc.
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>



/* =========================
   Parser Functions
   ========================= */

/* TrimWhiteSpace: removes leading and trailing whitespace from a string */
void TrimWhiteSpace(char *str) {
    if (!str) return;
    /* Remove leading whitespace */
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);  // +1 to copy the null terminator
    }
    /* Remove trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* IsCommandName: returns 1 if the given token is one of the command names, 0 otherwise */
int IsCommandName(char *macro) {
    if (!macro)
        return 0;
    TrimWhiteSpace(macro);
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(macro, operations[i]) == 0)
            return 1;
    }
    return 0;
}

/* IsCommentLine: returns the index of COMMENT_CHAR in the line or -1 if not found */
int IsCommentLine(char *line, size_t line_length) {
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == COMMENT_CHAR)
            return i;
    }
    return -1;
}

/* ParseMacrosDynamic: reads macro definitions from file_path and stores them into the dynamic MacroArray.
   Returns 0 on success, else an error code. */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray) {
    if (!file_path || !mArray)
        return STATUS_CATASTROPHIC;
    
    FILE *file_fd = fopen(file_path, "r");
    if (!file_fd)
        return STATUS_CATASTROPHIC;
    
    Macro currMacro;
    memset(&currMacro, 0, sizeof(Macro));
    
    while (1) {
        int status = AddMacro(file_fd, &currMacro);
        if (status == STATUS_NO_RESULT)
            break;  // No more macros found.
        if (status == STATUS_CATASTROPHIC) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        /* Skip if a macro with the same name already exists or if its name is a command name */
        if (FindMacroDynamic(currMacro.name, mArray) != NULL || IsCommandName(currMacro.name)) {
            CleanUpMacro(&currMacro);
            continue;
        }
        if (addMacroToArray(mArray, &currMacro) != 0) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        memset(&currMacro, 0, sizeof(Macro));
    }
    
    fclose(file_fd);
    return 0;
}

/* Helper function to update a label’s entry/extern flags.
   It now works only if the label already exists in the table.
   If the label is undefined, the directive is ignored completely. */
int updateLabelDirective(LabelTable *table, const char *name, int isEntry, int isExternal) {
    Label *lbl = findLabel(table, name);
    if (!lbl) {
        fprintf(stderr, "Warning: Directive for undefined label '%s' ignored.\n", name);
        return STATUS_NO_RESULT;
    }
    /* Do not allow a label to be both entry and external. */
    if ((isEntry && lbl->isExternal) || (isExternal && lbl->isEntry)) {
        fprintf(stderr, "Error: Label '%s' cannot be both entry and external.\n", name);
        return STATUS_NO_RESULT;
    }
    lbl->isEntry |= isEntry;
    lbl->isExternal |= isExternal;
    return 0;
}

/* ParseLabels: reads the file line by line and processes label definitions,
   including handling .entry and .extern directives as specified:
   - Directives must be in the form ".entry LABEL" or "LABEL .entry"
   - They affect only labels that already exist.
   If the referenced label does not exist, the directive line is ignored.
   Returns 0 on success, else an error code. */
int ParseLabels(const char *filePath, LabelTable *table) {
    if (!filePath || !table)
        return STATUS_CATASTROPHIC;
    
    FILE *fp = fopen(filePath, "r");
    if (!fp)
        return STATUS_CATASTROPHIC;
    
    char line[MAX_LINE_LENGTH];
    int lineCounter = 0;
    
    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        lineCounter++;
        
        // Make a copy and trim whitespace
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, line, MAX_LINE_LENGTH);
        lineCopy[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(lineCopy);
        
        // Skip empty or comment lines
        if (lineCopy[0] == '\0' || lineCopy[0] == COMMENT_CHAR)
            continue;
        
        // Tokenize the line into up to two tokens (we only need directive lines in this case)
        char *tokens[2] = {NULL, NULL};
        int tokenCount = 0;
        char *token = strtok(lineCopy, " \t\n");
        while (token && tokenCount < 2) {
            TrimWhiteSpace(token);
            tokens[tokenCount++] = token;
            token = strtok(NULL, " \t\n");
        }
        if (tokenCount < 2)
            continue;
        
        /* Check if the line is a directive line in the form ".entry LABEL" or ".extern LABEL" */
        if ( (strcmp(tokens[0], ".entry") == 0) || (strcmp(tokens[0], ".extern") == 0) ) {
            int isEntry = (strcmp(tokens[0], ".entry") == 0) ? 1 : 0;
            int isExternal = (strcmp(tokens[0], ".extern") == 0) ? 1 : 0;
            updateLabelDirective(table, tokens[1], isEntry, isExternal);
            continue;
        }
        
        /* Otherwise, process lines that define labels normally.
           For a label definition, the first token must contain a colon. */
        char *colonPos = strchr(tokens[0], ':');
        if (colonPos) {
            *colonPos = '\0';
            TrimWhiteSpace(tokens[0]);
            if (!IsCommandName(tokens[0])) {
                printf("Detected label: %s at line %d\n", tokens[0], lineCounter);
                int status = addLabel(table, tokens[0], lineCounter, 0, 0);
                if (status != 0) {
                    fprintf(stderr, "Warning: Could not add label '%s' (status %d)\n", tokens[0], status);
                }
            }
            /* Check for a directive following the label on the same line */
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
                TrimWhiteSpace(token);
                if (strcmp(token, ".entry") == 0) {
                    updateLabelDirective(table, tokens[0], 1, 0);
                } else if (strcmp(token, ".extern") == 0) {
                    updateLabelDirective(table, tokens[0], 0, 1);
                }
            }
        }
        // All other lines are ignored.
    }
    
    fclose(fp);
    return 0;
}




/* parseDataDirective: Processes a line with a .data directive and adds its numeric values to DataImage */
int parseDataDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".data");
    if (!p) return -1;
    p += 5; // Skip ".data"
    while (isspace((unsigned char)*p)) p++;
    while (*p != '\0' && *p != '\n') {
        while (isspace((unsigned char)*p)) p++;
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
        while (isspace((unsigned char)*p)) p++;
        if (*p == ',') p++;
    }
    return 0;
}

/* parseStringDirective: Processes a line with a .string directive and adds its characters (as ASCII codes)
   plus a terminating 0 to DataImage */
int parseStringDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".string");
    if (!p) return -1;
    p += strlen(".string");
    while (isspace((unsigned char)*p)) p++;
    /* Accept either a standard double quote or a curly quote for the opening quote */
    if (*p != '"' && strncmp(p, "\xE2\x80\x9C", 3) != 0) {
        fprintf(stderr, "Error: .string directive missing opening quote\n");
        return -1;
    }
    if (*p == '"') {
        p++;
    } else {
        p += 3;
    }
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
    if (addDataValue(data, 0) != 0) {
        fprintf(stderr, "Error adding terminating 0 for string\n");
        return -1;
    }
    return 0;
}

/* A helper function to check if a line is an instruction.
   Here we assume that a non-empty line that does not contain a data directive,
   and does not consist solely of a label (ending with ':'), is a code instruction. */
int isInstructionLine(const char *line) {
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    // Trim leading and trailing whitespace (assume TrimWhiteSpace is defined)
    TrimWhiteSpace(temp);
    if (strlen(temp) == 0)
        return 0;
    // If the line ends with a colon, we treat it as a label-only line.
    size_t len = strlen(temp);
    if (temp[len - 1] == ':')
        return 0;
    // If the line contains a data or string directive, it is not an instruction.
    if (strstr(temp, ".data") != NULL || strstr(temp, ".string") != NULL)
        return 0;
    return 1;
}
