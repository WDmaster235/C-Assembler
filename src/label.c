#include "label.h"
#include "parser.h"  // For IsCommandName
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Initialize label table */
int initLabelTable(LabelTable *table) {
    if (!table) return STATUS_CATASTROPHIC;
    table->labels = malloc(INITIAL_LABEL_CAPACITY * sizeof(Label));
    if (!table->labels) return STATUS_CATASTROPHIC;
    table->count = 0;
    table->capacity = INITIAL_LABEL_CAPACITY;
    return 0;
}

/* Add a label to the table */
int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal) {
    if (!table || !name) return STATUS_CATASTROPHIC;

    /* Disallow labels that match a command name */
    if (IsCommandName((char *)name)) {
        fprintf(stderr, "Error: Label '%s' cannot be a command name.\n", name);
        return STATUS_NO_RESULT;
    }
    /* Check for duplicates */
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            fprintf(stderr, "Warning: Duplicate label '%s'\n", name);
            return STATUS_NO_RESULT;
        }
    }

    /* Expand array if needed */
    if (table->count >= table->capacity) {
        size_t newCapacity = table->capacity * 2;
        Label *newLabels = realloc(table->labels, newCapacity * sizeof(Label));
        if (!newLabels) return STATUS_CATASTROPHIC;
        table->labels = newLabels;
        table->capacity = newCapacity;
    }

    /* Insert new label */
    strncpy(table->labels[table->count].name, name, MAX_SYMBOL_NAME - 1);
    table->labels[table->count].name[MAX_SYMBOL_NAME - 1] = '\0';
    table->labels[table->count].address = address;
    table->labels[table->count].isEntry = isEntry;
    table->labels[table->count].isExternal = isExternal;
    table->count++;
    return 0;
}

/* Find a label by name */
Label *findLabel(LabelTable *table, const char *name) {
    if (!table || !name) return NULL;
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            return &table->labels[i];
        }
    }
    return NULL;
}

/* Print label table for debugging */
void printLabelTable(const LabelTable *table, FILE *fp) {
    if (!table || !fp) return;
    for (size_t i = 0; i < table->count; i++) {
        fprintf(fp, "Name: %s, Address: %d, Entry: %d, External: %d\n",
                table->labels[i].name,
                table->labels[i].address,
                table->labels[i].isEntry,
                table->labels[i].isExternal);
    }
}

/* Free the label table */
void freeLabelTable(LabelTable *table) {
    if (!table) return;
    if (table->labels) free(table->labels);
    table->labels = NULL;
    table->count = 0;
    table->capacity = 0;
}

/* Write all entry labels to a file */
int writeEntryFile(const char *outputPath, const LabelTable *table) {
    FILE *fp = fopen(outputPath, "w");
    if (!fp) return STATUS_CATASTROPHIC;
    for (size_t i = 0; i < table->count; i++) {
        if (table->labels[i].isEntry) {
            fprintf(fp, "%s %d\n", table->labels[i].name, table->labels[i].address);
        }
    }
    fclose(fp);
    return 0;
}

/* Write all extern labels to a file */
int writeExternFile(const char *outputPath, const LabelTable *table) {
    FILE *fp = fopen(outputPath, "w");
    if (!fp) return STATUS_CATASTROPHIC;
    for (size_t i = 0; i < table->count; i++) {
        if (table->labels[i].isExternal) {
            fprintf(fp, "%s\n", table->labels[i].name);
        }
    }
    fclose(fp);
    return 0;
}
