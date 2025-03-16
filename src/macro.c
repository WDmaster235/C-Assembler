#include "macro.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Initialize the dynamic macro array */
int initMacroArray(MacroArray *mArray) {
    if (!mArray) return STATUS_CATASTROPHIC;
    mArray->macros = malloc(INITIAL_MACRO_CAPACITY * sizeof(Macro));
    if (!mArray->macros) return STATUS_CATASTROPHIC;
    mArray->count = 0;
    mArray->capacity = INITIAL_MACRO_CAPACITY;
    return 0;
}

/* Add a macro to the dynamic array, expanding if necessary */
int addMacroToArray(MacroArray *mArray, Macro *macro) {
    if (!mArray || !macro) return STATUS_CATASTROPHIC;
    if (mArray->count >= mArray->capacity) {
        size_t newCapacity = mArray->capacity * 2;
        Macro *newArray = realloc(mArray->macros, newCapacity * sizeof(Macro));
        if (!newArray) return STATUS_CATASTROPHIC;
        mArray->macros = newArray;
        mArray->capacity = newCapacity;
    }
    mArray->macros[mArray->count] = *macro;  // copy the macro struct
    mArray->count++;
    return 0;
}

/* Find a macro by name in the dynamic array */
Macro *FindMacroDynamic(char *name, MacroArray *mArray) {
    if (!mArray || !name) return NULL;
    for (size_t i = 0; i < mArray->count; i++) {
        if (strncmp(mArray->macros[i].name, name, strlen(name)) == 0) {
            return &mArray->macros[i];
        }
    }
    return NULL;
}

/* Free the dynamic macro array */
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

/* Reads from file_fd to extract a macro definition.
   Returns 0 on success, STATUS_NO_RESULT if no more macros, or STATUS_CATASTROPHIC on error. */
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

/* Cleans up the memory allocated for a macro */
int CleanUpMacro(Macro *macro) {
    if (macro == NULL) return STATUS_CATASTROPHIC;
    free(macro->name);
    for (size_t i = 0; i < macro->line_count; i++) {
        free(macro->body[i]);
    }
    free(macro->body);
    return 0;
}

/*Parse macros from a file and add them to the dynamic MacroArray */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray) {
    if (file_path == NULL || mArray == NULL) return STATUS_CATASTROPHIC;
    
    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) return STATUS_CATASTROPHIC;
    
    Macro currMacro;
    memset(&currMacro, 0, sizeof(Macro));
    
    while (1) {
        int status = AddMacro(file_fd, &currMacro);
        if (status == STATUS_NO_RESULT) break;  // No more macros found
        if (status == STATUS_CATASTROPHIC) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }
        /* Check for duplicate macro names; if duplicate, clean up and continue */
        if (FindMacroDynamic(currMacro.name, mArray) != NULL) {
            CleanUpMacro(&currMacro);
            continue;
        }
        /* Also ignore macros whose name conflicts with an existing command */
        if (IsCommandName(currMacro.name)) {
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