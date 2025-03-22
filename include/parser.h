#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>
#include "definitions.h"
#include "commands.h"
#include "macro.h"
#include "data.h"
#include "label.h"

/* Trim leading and trailing whitespace from a string */
void TrimWhiteSpace(char *str);

/* Returns 1 if the given macro name matches one of the command names, 0 otherwise */
int IsCommandName(char *macro_name);

/* Returns the index of the comment character in the line, or -1 if not found */
int IsCommentLine(char *line, size_t line_length);

/* Parses macros from the given file into a dynamic MacroArray.
   Returns 0 on success, else an error code. */
int ParseMacrosDynamic(const char *file_path, MacroArray *mArray);

/* Parses labels from the file at filePath and adds them to the LabelTable.
   A label is assumed to be the first token of a line ending with ':'.
   Returns 0 on success, else an error code. */
int ParseLabels(const char *filePath, LabelTable *table);

/* Parses a line containing a .data directive and adds the numbers into the DataImage.
   Returns 0 on success, non-zero on error. */
int parseDataDirective(const char *line, DataImage *data);

/* Parses a line containing a .string directive and adds the ASCII codes of the string (plus a terminating 0)
   into the DataImage.
   Returns 0 on success, non-zero on error. */
int parseStringDirective(const char *line, DataImage *data);
int isRegister(const char *token);

#endif
