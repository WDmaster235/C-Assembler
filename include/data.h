#ifndef DATA_H
#define DATA_H

#include <stddef.h>

#define INITIAL_DATA_CAPACITY 64

typedef struct {
    int *values;       // Dynamic array holding the data values
    size_t count;      // Number of values currently stored
    size_t capacity;   // Allocated capacity for values
} DataImage;

/* Initializes a DataImage structure.
   Returns 0 on success, non-zero on error. */
int initDataImage(DataImage *data);

/* Adds a single integer value to the DataImage.
   Returns 0 on success, non-zero on error. */
int addDataValue(DataImage *data, int value);

/* Frees any allocated memory in the DataImage. */
void freeDataImage(DataImage *data);

#endif /* DATA_H */
