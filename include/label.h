#ifndef LABEL_H
#define LABEL_H

#include "parser.h"  
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

typedef struct {
    char labelName[MAX_SYMBOL_NAME];
    int address;  /* usage address where the code references this extern label */
} ExternalRef;

typedef struct {
    ExternalRef *items;
    size_t count;
    size_t capacity;
} ExtRefArray;

int updateLabelDirective(LabelTable *table, const char *name, int isEntry, int isExternal);
int initLabelTable(LabelTable *table);
int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal);
Label *findLabel(LabelTable *table, const char *name);
void printLabelTable(const LabelTable *table, FILE *fp);
void freeLabelTable(LabelTable *table);
void initExtRefArray(ExtRefArray *arr);
void freeExtRefArray(ExtRefArray *arr);
void AddExtRef(ExtRefArray *arr, const char *label, int address);

#endif 
