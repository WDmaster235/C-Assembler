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

int parseDataDirective(const char *line, DataImage *data) {
    // Look for the ".data" keyword in the line.
    const char *p = strstr(line, ".data");
    if (!p) return -1; // Not a data directive
    p += 5; // Move past ".data"
    
    // Skip any whitespace after ".data"
    while (isspace((unsigned char)*p)) {
        p++;
    }
    
    // Process the comma-separated list of numbers.
    while (*p != '\0' && *p != '\n') {
        // Skip any leading whitespace before each number.
        while (isspace((unsigned char)*p)) {
            p++;
        }
        
        if (*p == '\0' || *p == '\n') break;
        
        errno = 0;
        char *endptr;
        int value = (int)strtol(p, &endptr, 10);
        if (p == endptr) {
            fprintf(stderr, "Error: invalid number in .data directive: %s\n", p);
            return -1;
        }
        if (errno != 0) {
            perror("strtol");
            return -1;
        }
        
        if (addDataValue(data, value) != 0) {
            fprintf(stderr, "Error: unable to add data value %d\n", value);
            return -1;
        }
        
        p = endptr;
        
        // Skip any whitespace after the number.
        while (isspace((unsigned char)*p)) {
            p++;
        }
        
        // If a comma is found, skip it.
        if (*p == ',') {
            p++;
        }
    }
    
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
