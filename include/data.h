#ifndef DATA_H
#define DATA_H

#include <stddef.h>

#define INITIAL_DATA_CAPACITY 64

typedef struct {
    int *values;       // Dynamic array holding the data values
    size_t count;      // Number of values currently stored
    size_t capacity;   // Allocated capacity for values
} DataImage;

int initDataImage(DataImage *data);
int addDataValue(DataImage *data, int value);
int parseDataDirective(const char *line, DataImage *data);
int parseStringDirective(const char *line, DataImage *data);
void freeDataImage(DataImage *data);

#endif
