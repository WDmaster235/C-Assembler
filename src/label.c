#include "../include/label.h"
#include <stdlib.h>
#include <string.h>

int initLabelTable(LabelTable *table) {
    if (!table) {
        return STATUS_CATASTROPHIC;
    }
    table->labels = malloc(INITIAL_LABEL_CAPACITY * sizeof(Label));
    if (!table->labels) {
        return STATUS_CATASTROPHIC;
    }
    table->count = 0;
    table->capacity = INITIAL_LABEL_CAPACITY;
    return 0;
}

int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal) {
    if (!table || !name) {
        return STATUS_CATASTROPHIC;
    }
    // Check for duplicate labels
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            return STATUS_NO_RESULT; // Duplicate label found
        }
    }
    // If the table is full, expand its capacity
    if (table->count >= table->capacity) {
        size_t newCapacity = table->capacity * 2;
        Label *newLabels = realloc(table->labels, newCapacity * sizeof(Label));
        if (!newLabels) {
            return STATUS_CATASTROPHIC;
        }
        table->labels = newLabels;
        table->capacity = newCapacity;
    }
    // Add the new label (copy name safely using MAX_SYMBOL_NAME)
    strncpy(table->labels[table->count].name, name, MAX_SYMBOL_NAME - 1);
    table->labels[table->count].name[MAX_SYMBOL_NAME - 1] = '\0';
    table->labels[table->count].address = address;
    table->labels[table->count].isEntry = isEntry;
    table->labels[table->count].isExternal = isExternal;
    table->count++;
    return 0;
}

Label *findLabel(LabelTable *table, const char *name) {
    if (!table || !name) {
        return NULL;
    }
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            return &table->labels[i];
        }
    }
    return NULL;
}

void printLabelTable(const LabelTable *table, FILE *fp) {
    if (!table || !fp) {
        return;
    }
    fprintf(fp, "Label Table:\n");
    for (size_t i = 0; i < table->count; i++) {
        fprintf(fp, "Name: %s, Address: %d, Entry: %d, External: %d\n",
                table->labels[i].name,
                table->labels[i].address,
                table->labels[i].isEntry,
                table->labels[i].isExternal);
    }
}

void freeLabelTable(LabelTable *table) {
    if (!table) {
        return;
    }
    free(table->labels);
    table->labels = NULL;
    table->count = 0;
    table->capacity = 0;
}
