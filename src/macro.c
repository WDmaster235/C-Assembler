#include "macro.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Dynamic Macro Array management functions */

int initMacroArray(MacroArray *mArray) {
    if (!mArray) return STATUS_CATASTROPHIC;
    mArray->macros = malloc(16 * sizeof(Macro)); // initial capacity = 16
    if (!mArray->macros) return STATUS_CATASTROPHIC;
    mArray->count = 0;
    mArray->capacity = 16;
    return 0;
}

int addMacroToArray(MacroArray *mArray, Macro *macro) {
    if (!mArray || !macro) return STATUS_CATASTROPHIC;
    if (mArray->count >= mArray->capacity) {
        size_t newCapacity = mArray->capacity * 2;
        Macro *newArray = realloc(mArray->macros, newCapacity * sizeof(Macro));
        if (!newArray) return STATUS_CATASTROPHIC;
        mArray->macros = newArray;
        mArray->capacity = newCapacity;
    }
    mArray->macros[mArray->count] = *macro;
    mArray->count++;
    return 0;
}

Macro *FindMacroDynamic(const char *name, MacroArray *mArray) {
    if (!mArray || !name) return NULL;
    for (size_t i = 0; i < mArray->count; i++) {
        if (strcmp(mArray->macros[i].name, name) == 0) {
            return &mArray->macros[i];
        }
    }
    return NULL;
}

void freeMacroArray(MacroArray *mArray) {
    if (!mArray) return;
    for (size_t i = 0; i < mArray->count; i++) {
        free(mArray->macros[i].name);
        for (size_t j = 0; j < mArray->macros[i].line_count; j++) {
            free(mArray->macros[i].body[j]);
        }
        free(mArray->macros[i].body);
    }
    free(mArray->macros);
    mArray->macros = NULL;
    mArray->count = 0;
    mArray->capacity = 0;
}

/* Fixed macro parsing functions */

int AddMacro(FILE *file_fd, Macro *macro) {
    if (file_fd == NULL || macro == NULL) return STATUS_CATASTROPHIC;
    char line[MAX_LINE_LENGTH] = {0};
    size_t inMacro = 0;
    size_t line_count = 0;
    while (fgets(line, MAX_LINE_LENGTH, file_fd) != NULL) {
        if (!inMacro) {
            if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0) {
                size_t name_offset = strlen(MACRO_START);
                size_t name_length = 0;
                if (!isblank(line[name_offset])) return STATUS_CATASTROPHIC;
                name_offset++;
                while (isblank(line[name_offset])) name_offset++;
                while (!isblank(line[name_offset + name_length]) && name_length < MAX_MACRO_NAME)
                    name_length++;
                macro->name = strndup(line + name_offset, name_length);
                if (macro->name == NULL) return STATUS_CATASTROPHIC;
                inMacro = 1;
            }
        } else {
            if (strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
                macro->line_count = line_count;
                return 0;
            }
            if (line_count < MAX_MACRO_LINES) {
                char **temp = realloc(macro->body, (line_count + 1) * sizeof(char *));
                if (temp == NULL) return STATUS_CATASTROPHIC;
                macro->body = temp;
                macro->body[line_count] = strndup(line, strlen(line));
                if (macro->body[line_count] == NULL) return STATUS_CATASTROPHIC;
                line_count++;
            }
        }
    }
    return STATUS_NO_RESULT;
}

int CleanUpMacro(Macro *macro) {
    if (macro == NULL) return STATUS_CATASTROPHIC;
    free(macro->name);
    for (size_t i = 0; i < macro->line_count; i++) {
        free(macro->body[i]);
    }
    free(macro->body);
    return 0;
}


/* DeleteMacroDefinitions: Creates a file (output_path) from input_path with all macro definition blocks removed. */
int DeleteMacroDefinitions(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "r");
    if (!in) return STATUS_CATASTROPHIC;
    
    FILE *out = fopen(output_path, "w");
    if (!out) {
        fclose(in);
        return STATUS_CATASTROPHIC;
    }
    
    char line[MAX_LINE_LENGTH] = {0};
    while (fgets(line, MAX_LINE_LENGTH, in)) {
        if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0) {
            // Skip lines until MACRO_END
            while (fgets(line, MAX_LINE_LENGTH, in)) {
                if (strncmp(line, MACRO_END, strlen(MACRO_END)) == 0)
                    break;
            }
            continue;
        }
        fprintf(out, "%s", line);
    }
    
    fclose(in);
    fclose(out);
    return 0;
}

int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray) {
    FILE *in = fopen(input_path, "r");
    if (!in) return STATUS_CATASTROPHIC;
    
    FILE *out = fopen(output_path, "w");
    if (!out) {
        fclose(in);
        return STATUS_CATASTROPHIC;
    }
    
    char line[MAX_LINE_LENGTH] = {0};
    while (fgets(line, MAX_LINE_LENGTH, in)) {
        // Create a trimmed copy of the line.
        char trimmed[MAX_LINE_LENGTH];
        strncpy(trimmed, line, MAX_LINE_LENGTH);
        trimmed[MAX_LINE_LENGTH - 1] = '\0';
        TrimWhiteSpace(trimmed);
        
        // If the trimmed line is empty, output the line as-is.
        if (strlen(trimmed) == 0) {
            fprintf(out, "%s", line);
            continue;
        }
        
        // Check if the trimmed line is a single token (no spaces or tabs).
        int isSingleToken = (strchr(trimmed, ' ') == NULL && strchr(trimmed, '\t') == NULL);
        
        if (isSingleToken) {
            Macro *macro = FindMacroDynamic(trimmed, mArray);
            if (macro != NULL) {
                // Expand the macro by outputting its body instead of the macro call.
                for (size_t i = 0; i < macro->line_count; i++) {
                    if (fprintf(out, "%s", macro->body[i]) < 0) {
                        fclose(in);
                        fclose(out);
                        return STATUS_CATASTROPHIC;
                    }
                }
                continue;  // Skip writing the original line.
            }
        }
        
        // Otherwise, write the original line.
        if (fprintf(out, "%s", line) < 0) {
            fclose(in);
            fclose(out);
            return STATUS_CATASTROPHIC;
        }
    }
    
    fclose(in);
    fclose(out);
    return 0;
}

/* DeepCopyMacro, if needed */
Macro *DeepCopyMacro(Macro *src) {
    if (src == NULL) return NULL;
    Macro *dst = malloc(sizeof(Macro));
    if (dst == NULL) return NULL;
    memset(dst, 0, sizeof(*dst));
    
    dst->name = strdup(src->name);
    dst->line_count = src->line_count;
    
    dst->body = malloc(dst->line_count * sizeof(char *));
    if (!dst->body) {
        free(dst->name);
        free(dst);
        return NULL;
    }
    for (size_t i = 0; i < dst->line_count; i++) {
        dst->body[i] = strdup(src->body[i]);
    }
    
    CleanUpMacro(src);
    return dst;
}
