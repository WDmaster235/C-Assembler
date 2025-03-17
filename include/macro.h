#ifndef MACRO_H
#define MACRO_H

#include <stddef.h>
#include <stdio.h>
#include "definitions.h"
#include "io.h"

#define MAX_MACRO_NAME    32
#define MAX_MACRO_LINES   128

typedef struct s_macro {
    char *name;
    char **body;
    size_t line_count;
} Macro;

/* Dynamic array for macros */
typedef struct {
    Macro *macros;
    size_t count;
    size_t capacity;
} MacroArray;

/* Dynamic Macro Array management functions */
int initMacroArray(MacroArray *mArray);
int addMacroToArray(MacroArray *mArray, Macro *macro);
Macro *FindMacroDynamic(const char *name, MacroArray *mArray);
void freeMacroArray(MacroArray *mArray);

/* Fixed macro parsing functions (used to populate the dynamic array) */
int AddMacro(FILE *file_fd, Macro *macro);
int CleanUpMacro(Macro *macro);

/* New functions for dynamic macros: */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);

/* ExpandMacros: expands macro calls (by replacing a line that exactly matches a macro name with its body)
   and skips macro definition blocks. */
int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray);

/* DeleteMacroDefinitions: produces a temporary file with all macro definitions (from MACRO_START to MACRO_END) removed. */
int DeleteMacroDefinitions(const char *input_path, const char *output_path);

#endif
