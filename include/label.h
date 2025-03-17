#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <stddef.h>
#include "definitions.h"

#ifndef INITIAL_LABEL_CAPACITY
#define INITIAL_LABEL_CAPACITY 16
#endif

typedef struct {
    char name[MAX_SYMBOL_NAME];
    int address;
    int isEntry;
    int isExternal;
} Label;

typedef struct {
    Label *labels;
    size_t count;
    size_t capacity;
} LabelTable;

int initLabelTable(LabelTable *table);
int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal);
Label *findLabel(LabelTable *table, const char *name);
void printLabelTable(const LabelTable *table, FILE *fp);
void freeLabelTable(LabelTable *table);

#endif /* LABEL_H */
