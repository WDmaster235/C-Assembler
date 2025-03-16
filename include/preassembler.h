#ifndef PREASSEMBLER_H
#define PREASSEMBLER_H

#include "macro.h"
#include "definitions.h"

<<<<<<< HEAD
/* Dynamic macro parsing functions */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);
int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray);
Macro *DeepCopyMacro(Macro *src);
#endif
=======
// Returns 0 upon success, else ERRORCODE
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count);

int IsCommandName(char *macro);

// Returns 0 upon success, else ERRORCODE
int ExpandMacros(const char *input_path, const char *output_path);

#endif
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
