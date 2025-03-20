#include "commands.h"

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
