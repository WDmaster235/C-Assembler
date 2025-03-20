#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdint.h>
#include "definitions.h"

#define E 1
#define R 2
#define A 4

typedef struct {
    char *name;
    uint8_t are : 3;
    uint8_t funct : 5;
    uint8_t dst_reg : 3;
    uint8_t dst_addressing : 2;
    uint8_t src_reg : 3;
    uint8_t src_addressing : 2;
    uint8_t opcode : 6;
} CommandWord;

typedef struct {
    uint8_t are : 3;
    uint32_t address : 21;
} LabelWord;

typedef struct {
    uint32_t number : 23;
} NumberWord;

// Declare the arrays as extern
extern const CommandWord *commands[NUM_COMMANDS];
extern const char *operations[NUM_COMMANDS];

#endif
