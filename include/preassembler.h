#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include "macro.h"
#include "definitions.h"

/* Dynamic macro parsing functions */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);
int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray);
Macro *DeepCopyMacro(Macro *src);
#endif
