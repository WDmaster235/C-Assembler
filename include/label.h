#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <stddef.h>
#include "../include/definitions.h"

#ifndef INITIAL_LABEL_CAPACITY
#define INITIAL_LABEL_CAPACITY 16
#endif

typedef struct {
    char name[MAX_SYMBOL_NAME];  // Using MAX_SYMBOL_NAME from definitions.h
    int address;                 // The address the label refers to
    int isEntry;                 // 1 if the label is marked as entry, 0 otherwise
    int isExternal;              // 1 if the label is external, 0 otherwise
} Label;

typedef struct {
    Label *labels;     // Dynamically allocated array of labels
    size_t count;      // Number of labels currently in the table
    size_t capacity;   // Current capacity of the labels array
} LabelTable;

// Initializes a label table. Returns 0 on success or STATUS_CATASTROPHIC on error.
int initLabelTable(LabelTable *table);

// Adds a new label to the table.
// Returns 0 on success, STATUS_NO_RESULT if a duplicate label is found,
// or STATUS_CATASTROPHIC on allocation or other error.
int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal);

// Searches for a label by name.
// Returns a pointer to the label if found, or NULL if not.
Label *findLabel(LabelTable *table, const char *name);

// Prints the entire label table to the specified file.
void printLabelTable(const LabelTable *table, FILE *fp);

// Frees any resources allocated for the label table.
void freeLabelTable(LabelTable *table);

#endif /* LABEL_H */
