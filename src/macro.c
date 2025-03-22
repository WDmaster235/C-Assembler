#include "macro.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Example of the dynamic array init */
int initMacroArray(MacroArray *mArray) {
    if (!mArray) return -1;
    mArray->macros = malloc(sizeof(Macro) * 8);
    if (!mArray->macros) return -1;
    mArray->count = 0;
    mArray->capacity = 8;
    return 0;
}

int addMacroToArray(MacroArray *mArray, Macro *macro) {
    if (!mArray || !macro) return -1;
    if (mArray->count >= mArray->capacity) {
        size_t newCap = mArray->capacity * 2;
        Macro *temp = realloc(mArray->macros, sizeof(Macro)*newCap);
        if (!temp) return -1;
        mArray->macros = temp;
        mArray->capacity = newCap;
    }
    /* Copy the macro object */
    mArray->macros[mArray->count].name = macro->name;
    mArray->macros[mArray->count].body = macro->body;
    mArray->macros[mArray->count].line_count = macro->line_count;
    mArray->count++;
    return 0;
}

Macro *FindMacroDynamic(const char *name, MacroArray *mArray) {
    if (!name || !mArray) return NULL;
    for (size_t i = 0; i < mArray->count; i++) {
        if (strcmp(mArray->macros[i].name, name) == 0)
            return &mArray->macros[i];
    }
    return NULL;
}

void freeMacroArray(MacroArray *mArray) {
    if (!mArray) return;
    if (!mArray->macros) return;
    for (size_t i = 0; i < mArray->count; i++) {
        /* free each macro's name and body lines */
        if (mArray->macros[i].name) {
            free(mArray->macros[i].name);
            mArray->macros[i].name = NULL;
        }
        if (mArray->macros[i].body) {
            for (size_t j = 0; j < mArray->macros[i].line_count; j++) {
                if (mArray->macros[i].body[j]) {
                    free(mArray->macros[i].body[j]);
                }
            }
            free(mArray->macros[i].body);
            mArray->macros[i].body = NULL;
        }
    }
    free(mArray->macros);
    mArray->macros = NULL;
    mArray->count = 0;
    mArray->capacity = 0;
}


int CleanUpMacro(Macro *macro) {
    if (!macro) return 0;
    if (macro->name) {
        free(macro->name);
        macro->name = NULL;
    }
    if (macro->body) {
        for (size_t i = 0; i < macro->line_count; i++) {
            free(macro->body[i]);
        }
        free(macro->body);
        macro->body = NULL;
    }
    macro->line_count = 0;
    return 0;
}

int DeleteMacroDefinitions(const char *input_path, const char *output_path) {
    /* minimal stub */
    remove(output_path);
    FILE *fin = fopen(input_path, "r");
    FILE *fout = fopen(output_path, "w");
    if (!fin || !fout) {
        if (fin) fclose(fin);
        if (fout) fclose(fout);
        return -1;
    }
    /* Copy the file as-is, ignoring macro definitions if you want real logic */
    char line[512];
    while (fgets(line, sizeof(line), fin)) {
        fputs(line, fout);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}

int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray) {
    /* minimal stub just copies file */
    remove(output_path);
    FILE *fin = fopen(input_path, "r");
    FILE *fout = fopen(output_path, "w");
    if (!fin || !fout) {
        if (fin) fclose(fin);
        if (fout) fclose(fout);
        return -1;
    }
    char line[512];
    while (fgets(line, sizeof(line), fin)) {
        fputs(line, fout);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}

int AddMacro(FILE *file_fd, Macro *macro) {
    /* minimal stub */
    (void)file_fd;
    (void)macro;
    return STATUS_NO_RESULT; 
}