#include "../include/label.h"



int initLabelTable(LabelTable *table) {
    if (!table) return STATUS_CATASTROPHIC;
    table->labels = malloc(INITIAL_LABEL_CAPACITY * sizeof(Label));
    if (!table->labels) return STATUS_CATASTROPHIC;
    table->count = 0;
    table->capacity = INITIAL_LABEL_CAPACITY;
    return 0;
}

int updateLabelDirective(LabelTable *table, const char *name, int isEntry, int isExternal) {
    Label *lbl = findLabel(table, name);
    if (!lbl) {
        fprintf(stderr, "Error: Directive '%s' for undefined label '%s' ignored.\n",
                isEntry ? ".entry" : ".extern", name);
        return STATUS_NO_RESULT;
    }

    if ((isEntry && lbl->isExternal) || (isExternal && lbl->isEntry)) {
        fprintf(stderr, "Error: Label '%s' cannot be both entry and external.\n", name);
        return STATUS_NO_RESULT;
    }

    lbl->isEntry |= isEntry;
    lbl->isExternal |= isExternal;
    return 0;
}

int addLabel(LabelTable *table, const char *name, int address, int isEntry, int isExternal) {
    if (!table || !name) return STATUS_CATASTROPHIC;

    if (IsCommandName((char *)name)) {
        fprintf(stderr, "Error: Label '%s' cannot be a command name.\n", name);
        return STATUS_NO_RESULT;
    }
    size_t i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            fprintf(stderr, "Warning: Duplicate label '%s'\n", name);
            return STATUS_NO_RESULT;
        }
    }

    if (table->count >= table->capacity) {
        size_t newCapacity = table->capacity * 2;
        Label *newLabels = realloc(table->labels, newCapacity * sizeof(Label));
        if (!newLabels) return STATUS_CATASTROPHIC;
        table->labels = newLabels;
        table->capacity = newCapacity;
    }

    strncpy(table->labels[table->count].name, name, MAX_SYMBOL_NAME - 1);
    table->labels[table->count].name[MAX_SYMBOL_NAME - 1] = '\0';
    table->labels[table->count].address = address;
    table->labels[table->count].isEntry = isEntry;
    table->labels[table->count].isExternal = isExternal;
    table->count++;
    return 0;
}

Label *findLabel(LabelTable *table, const char *name) {
    if (!table || !name) return NULL;
    size_t i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->labels[i].name, name) == 0) {
            return &table->labels[i];
        }
    }
    return NULL;
}

void printLabelTable(const LabelTable *table, FILE *fp) {
    if (!table || !fp) return;
    size_t i;
    for (i = 0; i < table->count; i++) {
        fprintf(fp, "Name: %s, Address: %d, Entry: %d, External: %d\n",
                table->labels[i].name,
                table->labels[i].address,
                table->labels[i].isEntry,
                table->labels[i].isExternal);
    }
}

void freeLabelTable(LabelTable *table) {
    if (!table) return;
    if (table->labels) free(table->labels);
    table->labels = NULL;
    table->count = 0;
    table->capacity = 0;
}

int writeEntryFile(const char *outputPath, const LabelTable *table) {
    FILE *fp = fopen(outputPath, "w");
    if (!fp) return STATUS_CATASTROPHIC;
    size_t i;
    for (i = 0; i < table->count; i++) {
        if (table->labels[i].isEntry) {
            fprintf(fp, "%s %d\n", table->labels[i].name, table->labels[i].address);
        }
    }
    fclose(fp);
    return 0;
}

int writeExternFile(const char *outputPath, const LabelTable *table) {
    FILE *fp = fopen(outputPath, "w");
    if (!fp) return STATUS_CATASTROPHIC;
    size_t i;
    for (i = 0; i < table->count; i++) {
        if (table->labels[i].isExternal) {
            fprintf(fp, "%s\n", table->labels[i].name);
        }
    }
    fclose(fp);
    return 0;
}

/************************************************************
 * 1) Data structures for referencing external labels
 ************************************************************/

void initExtRefArray(ExtRefArray *arr) {
    arr->count = 0;
    arr->capacity = 8;
    arr->items = malloc(arr->capacity * sizeof(ExternalRef));
}

void freeExtRefArray(ExtRefArray *arr) {
    if (!arr) return;
    if (arr->items) free(arr->items);
    arr->items = NULL;
    arr->capacity = 0;
    arr->count = 0;
}

void AddExtRef(ExtRefArray *arr, const char *label, int address) {
    if (!arr || !label) return;
    if (arr->count >= arr->capacity) {
        size_t newCap = arr->capacity * 2;
        ExternalRef *tmp = realloc(arr->items, newCap * sizeof(ExternalRef));
        if (!tmp) {
            fprintf(stderr,"Error: out of memory in AddExtRef.\n");
            return;
        }
        arr->items = tmp;
        arr->capacity = newCap;
    }
    strncpy(arr->items[arr->count].labelName, label, MAX_SYMBOL_NAME-1);
    arr->items[arr->count].labelName[MAX_SYMBOL_NAME-1] = '\0';
    arr->items[arr->count].address = address;
    arr->count++;
}
