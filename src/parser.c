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
    // Remove leading
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    // Remove trailing
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

/* IsCommandName: returns 1 if the given token is one of the command names, 0 otherwise */
int IsCommandName(char *macro) {
    if (!macro) return 0;
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
        // Skip if a macro with the same name already exists or if its name is a command name
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
   If the label is undefined, the directive is ignored. */
int updateLabelDirective(LabelTable *table, const char *name, int isEntry, int isExternal) {
    Label *lbl = findLabel(table, name);
    if (!lbl) {
        fprintf(stderr, "Warning: Directive for undefined label '%s' ignored.\n", name);
        return STATUS_NO_RESULT;
    }
    if ((isEntry && lbl->isExternal) || (isExternal && lbl->isEntry)) {
        fprintf(stderr, "Error: Label '%s' cannot be both entry and external.\n", name);
        return STATUS_NO_RESULT;
    }
    lbl->isEntry |= isEntry;
    lbl->isExternal |= isExternal;
    return 0;
}

/* ParseLabels: reads the file line by line and processes label definitions,
   including handling .entry and .extern directives as specified.
   In this minimal example, you might not call it, but it's here for completeness. */
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
        char lineCopy[MAX_LINE_LENGTH];
        strncpy(lineCopy, line, MAX_LINE_LENGTH);
        lineCopy[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(lineCopy);
        
        if (lineCopy[0] == '\0' || lineCopy[0] == COMMENT_CHAR)
            continue;
        
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
        
        if ((strcmp(tokens[0], ".entry") == 0) || (strcmp(tokens[0], ".extern") == 0)) {
            int isEntry = (strcmp(tokens[0], ".entry") == 0) ? 1 : 0;
            int isExternal = (strcmp(tokens[0], ".extern") == 0) ? 1 : 0;
            updateLabelDirective(table, tokens[1], isEntry, isExternal);
            continue;
        }
        
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
    }
    
    fclose(fp);
    return 0;
}

/* parseDataDirective and parseStringDirective, if used, 
   can store numeric/ASCII data in DataImage. 
   Not used in this minimal example. */
int parseDataDirective(const char *line, DataImage *data) {
    return 0; // stub
}
int parseStringDirective(const char *line, DataImage *data) {
    return 0; // stub
}
