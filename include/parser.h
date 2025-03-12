#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "commands.h"
#include "macro.h"
#include "definitions.h"
#include "data.h"
#include "label.h"


// List of operation names

//Returns 1 if true and 0 if not
//Checks if the macro's name is an existing command name
int IsCommandName(char *macro_name);

// Returns index of char where comment begins, or -1 if no comment
// Index begins from zero
int IsCommentLine(char *line, size_t line_length);

int ParseLabels(const char *filePath, LabelTable *table);

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