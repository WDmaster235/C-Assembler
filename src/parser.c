#include "label.h"
#include "parser.h"
#include "macro.h"
#include "commands.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

/* Trim leading/trailing spaces */
void TrimWhiteSpace(char *str) {
    if (!str) return;
    /* Remove leading */
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    /* Remove trailing */
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

int IsCommandName(char *macro) {
    if (!macro) return 0;
    TrimWhiteSpace(macro);
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(macro, operations[i]) == 0)
            return 1;
    }
    return 0;
}

/* Return index of comment char or -1 if none */
int IsCommentLine(char *line, size_t line_length) {
    for (size_t i = 0; i < line_length; i++) {
        if (line[i] == COMMENT_CHAR)
            return i;
    }
    return -1;
}

/* Parse macros from file into MacroArray */
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
        if (status == STATUS_NO_RESULT) {
            // No more macros
            break;
        }
        if (status == STATUS_CATASTROPHIC) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        // Skip if a macro with same name already exists, or if name is a command
        if (FindMacroDynamic(currMacro.name, mArray) != NULL || IsCommandName(currMacro.name)) {
            CleanUpMacro(&currMacro);
            continue;
        }
        // Add to array
        if (addMacroToArray(mArray, &currMacro) != 0) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        memset(&currMacro, 0, sizeof(Macro));
    }

    fclose(file_fd);
    return 0;
}

/* Update label directive */
static int updateLabelDirective(LabelTable *table, const char *name, int isEntry, int isExternal) {
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

/* Example parse of .entry/.extern */
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
        // Possibly handle label definition
        char *colonPos = strchr(tokens[0], ':');
        if (colonPos) {
            *colonPos = '\0';
            TrimWhiteSpace(tokens[0]);
            if (!IsCommandName(tokens[0])) {
                // etc. ...
                addLabel(table, tokens[0], lineCounter, 0, 0);
            }
        }
    }

    fclose(fp);
    return 0;
}

/* parseDataDirective, parseStringDirective stubs, if needed */
int parseDataDirective(const char *line, DataImage *data) {
    return 0;
}
int parseStringDirective(const char *line, DataImage *data) {
    return 0;
}

/************************************************************************
 *  Add the missing definitions for countDataWords and countInstructionWords
 *  so the linker can find them
 ************************************************************************/

/* countDataWords:
 * If the line contains .data, we parse the integers to count them.
 * If it's .string, we count the length + 1 (null terminator).
 */
int countDataWords(const char *line)
{
    if (!line) return 0;
    if (strstr(line, ".data")) {
        /* count how many numbers. For instance:
           .data  5, -12, 100
           => 3 numbers => 3 words
         */
        const char *p = strstr(line, ".data") + 5;
        int count = 0;
        while (*p) {
            while (isspace((unsigned char)*p)) p++;
            if (*p == '\0') break;
            strtol(p, (char**)&p, 10);
            count++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == ',') p++;
        }
        return count;
    }
    else if (strstr(line, ".string")) {
        /* count the characters + 1 for '\0' */
        const char *start = strchr(line, '"');
        if (!start) return 0;
        start++;
        int count = 0;
        while (*start && *start != '"') {
            count++;
            start++;
        }
        return count + 1; /* add 1 for the null terminator */
    }
    return 0;
}

/* countInstructionWords:
 * a simplistic approach: 1 word for the command + up to 2 for extra (operands).
 */
int countInstructionWords(const char *line)
{
    if (!line) return 0;
    /* remove any leading label, then parse the first token => the command name. */
    char temp[MAX_LINE_LENGTH];
    strncpy(temp, line, MAX_LINE_LENGTH);
    temp[MAX_LINE_LENGTH - 1] = '\0';
    TrimWhiteSpace(temp);

    /* If there's a label, skip it */
    char *colon = strchr(temp, ':');
    char *instr = (colon) ? colon + 1 : temp;
    TrimWhiteSpace(instr);

    /* first token => command name */
    char *mn = strtok(instr, " \t,\n");
    if (!mn) return 0;

    /* see if it matches a known command */
    const CommandWord *cmdDef = NULL;
    for (int i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(mn, operations[i]) == 0) {
            cmdDef = commands[i];
            break;
        }
    }
    if (!cmdDef) return 0; /* not an instruction => no words */

    /* If it's a 1-operand command => possible +1 word for the operand */
    int used = 1; /* base word */
    if (cmdDef->oneOperand) {
        char *op = strtok(NULL, " \t,\n");
        if (!op) return used; /* no operand => only 1 word */
        /* if operand can cause an extra word => #, &label, or direct label => +1 */
        if (op[0] == '#' || op[0] == '&' || (!isRegister(op))) {
            used++;
        }
    } else {
        /* 2-operand command => possible up to +2 words for the two operands */
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");
        if (op1) {
            if (op1[0] == '#' || op1[0] == '&' || (!isRegister(op1))) {
                used++;
            }
        }
        if (op2) {
            if (op2[0] == '#' || op2[0] == '&' || (!isRegister(op2))) {
                used++;
            }
        }
    }
    return used;
}


int isRegister(const char *token)
{
    /* e.g. check if token is 'r' + digit 0..7 */
    if (!token) return 0;
    if (token[0] == 'r' &&
        token[1] >= '0' && token[1] <= '7' &&
        token[2] == '\0')
    {
        return 1; /* yes, it’s a register name */
    }
    return 0;
}

int countStringWords(const char *line)
{
    if (!line) return 0;
    if (strstr(line, ".string")) {
        const char *start = strchr(line, '"');
        if (!start) return 0;
        start++;
        int count = 0;
        while (*start && *start != '"') {
            count++;
            start++;
        }
        return count + 1; /* plus 1 for the null terminator */
    }
    return 0;
}
