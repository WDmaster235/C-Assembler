#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdint.h>
#include "definitions.h"

#define E 1
#define R 2
#define A 4

typedef struct {
    char *name;
    uint8_t oneOperand;      // 1 if the command takes one operand, 0 if two operands
    uint8_t are;             // should be 0 initially (we force A later for command words)
    uint8_t funct;           // e.g., 1 for add
    uint8_t dst_reg;
    uint8_t dst_addressing;
    uint8_t src_reg;
    uint8_t src_addressing;
    uint8_t opcode;          // e.g., 2 for add
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
