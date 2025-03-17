#include "parser.h"
#include "macro.h"  // For FindMacroDynamic and dynamic macro parsing
#include "commands.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* TrimWhiteSpace: removes leading and trailing whitespace */
void TrimWhiteSpace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

/* IsCommandName: returns 1 if the given macro name is one of the command names, 0 otherwise */
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
            break;  // No more macros
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

/* ParseLabels: Reads filePath line by line and adds any label (token ending with ':')
   to the label table. Returns 0 on success, else an error code. */
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
        char *trimmed = line;
        while (isspace((unsigned char)*trimmed)) {
            trimmed++;
        }
        if (*trimmed == '\0' || *trimmed == COMMENT_CHAR)
            continue;
        char *colon = strchr(trimmed, LABEL_DELIM);
        if (colon) {
            size_t labelLength = colon - trimmed;
            if (labelLength > 0 && labelLength < MAX_SYMBOL_NAME) {
                char labelName[MAX_SYMBOL_NAME];
                strncpy(labelName, trimmed, labelLength);
                labelName[labelLength] = '\0';
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

/* parseStringDirective: Processes a line with a .string directive and adds its characters (ASCII codes)
   plus a terminating 0 to DataImage */
int parseStringDirective(const char *line, DataImage *data) {
    const char *p = strstr(line, ".string");
    if (!p) return -1;
    p += strlen(".string");
    while (isspace((unsigned char)*p)) p++;
    /* Accept either standard " or curly “ for the opening quote */
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
