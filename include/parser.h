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

<<<<<<< HEAD
/* Parses a line containing a .data directive and adds the numbers to the DataImage. */
int parseDataDirective(const char *line, DataImage *data);

/* Parses a line containing a .string directive and adds the characters (ASCII codes)
   to the DataImage, appending a terminating 0. */
int parseStringDirective(const char *line, DataImage *data);



#endif
=======
/*  Parses a line containing a .data directive and adds the numbers to the DataImage.
    The line should contain the directive followed by one or more comma-separated integers.
    Returns 0 on success, non-zero on error. */
int parseDataDirective(const char *line, DataImage *data);

/*  Parses a line containing a .string directive and adds the string characters to the DataImage.
    The string is enclosed in double quotes. Each character's ASCII code is stored,
    followed by a terminating 0.
    Returns 0 on success, non-zero on error. */
int parseStringDirective(const char *line, DataImage *data);

#endif
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
