#ifndef MACRO_H
#define MACRO_H

#include <stddef.h>
#include <stdio.h>
#include "definitions.h"
#include "io.h"

#define MAX_MACRO_NAME 32
#define MAX_MACRO_LINES 128

typedef struct s_macro {
    char *name;
    char **body;
    size_t line_count;
} Macro;

typedef struct {
    Macro *macros;
    size_t count;
    size_t capacity;
} MacroArray;

/* Dynamic Macro Array management */
int initMacroArray(MacroArray *mArray);
int addMacroToArray(MacroArray *mArray, Macro *macro);
Macro *FindMacroDynamic(const char *name, MacroArray *mArray);
void freeMacroArray(MacroArray *mArray);

/* Macro parsing */
int AddMacro(FILE *file_fd, Macro *macro);
int CleanUpMacro(Macro *macro);

/* Expand macros */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);
int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray);
int DeleteMacroDefinitions(const char *input_path, const char *output_path);

#endif
