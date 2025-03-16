#ifndef MACRO_H
#define MACRO_H

#include <stddef.h>
#include <stdio.h>
#include "definitions.h"
#include "io.h"

#define MAX_MACRO_NAME    32
#define MAX_MACRO_LINES   128
#define INITIAL_MACRO_CAPACITY 16

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
Macro *FindMacroDynamic(char *name, MacroArray *mArray);
void freeMacroArray(MacroArray *mArray);

int AddMacro(FILE *file_fd, Macro *macro);
int CleanUpMacro(Macro *macro);

/*Parse macros from a file into a dynamic MacroArray */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);

#endif
