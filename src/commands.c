#include "commands.h"
#include <stdint.h>

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

} Word;

const CommandWord *commands[NUM_COMMANDS] = {
    &(CommandWord){"mov", 0, 0, 0, 0, 0, 0, 0},
    &(CommandWord){"cmp", 0, 0, 0, 0, 0, 0, 1},
    &(CommandWord){"add", 0, 1, 0, 0, 0, 0, 2},
    &(CommandWord){"sub", 0, 2, 0, 0, 0, 0, 2},
    &(CommandWord){"lea", 0, 0, 0, 0, 0, 0, 4},
    &(CommandWord){"clr", 0, 1, 0, 0, 0, 0, 5},
    &(CommandWord){"not", 0, 2, 0, 0, 0, 0, 5},
    &(CommandWord){"inc", 0, 3, 0, 0, 0, 0, 5},
    &(CommandWord){"dec", 0, 4, 0, 0, 0, 0, 5},
    &(CommandWord){"jmp", 0, 1, 0, 0, 0, 0, 9},
    &(CommandWord){"bne", 0, 2, 0, 0, 0, 0, 9},
    &(CommandWord){"jsr", 0, 0, 0, 0, 0, 0, 11},
    &(CommandWord){"red", 0, 0, 0, 0, 0, 0, 12},
    &(CommandWord){"prn", 0, 0, 0, 0, 0, 0, 13},
    &(CommandWord){"rts", 0, 0, 0, 0, 0, 0, 14},
    &(CommandWord){"stop", 0, 0, 0, 0, 0, 0, 15},
};

const char *operations[NUM_COMMANDS] = {
    "mov", "cmp", "add", "sub", "lea", "clr",
    "not", "inc", "dec", "jmp", "bne", "jsr",
    "red", "prn", "rts", "stop"
};


