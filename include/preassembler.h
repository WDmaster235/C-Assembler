#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "macro.h"

// Returns 0 upon success, else ERRORCODE
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count);

int IsCommandName(char *macro);

// Returns 0 upon success, else ERRORCODE
int ExpandMacros(const char *input_path, const char *output_path);

#endif