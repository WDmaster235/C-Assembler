#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "commands.h"
#include "macro.h"
#include "definitions.h"
#include "data.h"
#include "label.h"

/* Checks if a given macro name matches an existing command. Returns 1 if true, 0 if not. */
int IsCommandName(char *macro_name);

/* Returns the index of the comment in the line, or -1 if none exists. */
int IsCommentLine(char *line, size_t line_length);

/* Parse labels from a file and store them in the label table. */
int ParseLabels(const char *filePath, LabelTable *table);

/* Parses a line containing a .data directive and adds the numbers to the DataImage. */
int parseDataDirective(const char *line, DataImage *data);

/* Parses a line containing a .string directive and adds the characters (ASCII codes)
   to the DataImage, appending a terminating 0. */
int parseStringDirective(const char *line, DataImage *data);



#endif
