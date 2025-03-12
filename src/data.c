#include "data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

int initDataImage(DataImage *data) {
    if (!data) return -1;
    data->values = malloc(INITIAL_DATA_CAPACITY * sizeof(int));
    if (!data->values) return -1;
    data->count = 0;
    data->capacity = INITIAL_DATA_CAPACITY;
    return 0;
}

int addDataValue(DataImage *data, int value) {
    if (data->count >= data->capacity) {
        size_t newCapacity = data->capacity * 2;
        int *temp = realloc(data->values, newCapacity * sizeof(int));
        if (!temp) return -1;
        data->values = temp;
        data->capacity = newCapacity;
    }
    data->values[data->count++] = value;
    return 0;
}



void freeDataImage(DataImage *data) {
    if (data && data->values) {
        free(data->values);
        data->values = NULL;
        data->count = 0;
        data->capacity = 0;
    }
}

