#include "commands.h"

/* 
   This array of pointers to CommandWord structures
   defines the 16 commands. The opcode, funct, oneOperand, etc.
   must match the table from the instructions. 
*/

const CommandWord *commands[NUM_COMMANDS] = {
    &(CommandWord){"mov", 0, 0, 0 /*funct*/, 0, 0, 0, 0, OPCODE_MOV},
    &(CommandWord){"cmp", 0, 0, 0, 0, 0, 0, 0, OPCODE_CMP},
    &(CommandWord){"add", 0, 0, FUNCT_ADD, 0, 0, 0, 0, OPCODE_ADD},
    &(CommandWord){"sub", 0, 0, FUNCT_SUB, 0, 0, 0, 0, OPCODE_SUB},
    &(CommandWord){"lea", 0, 0, 0, 0, 0, 0, 0, OPCODE_LEA},
    &(CommandWord){"clr", 1, 0, FUNCT_CLR, 0, 0, 0, 0, OPCODE_CLR},
    &(CommandWord){"not", 1, 0, FUNCT_NOT, 0, 0, 0, 0, OPCODE_NOT},
    &(CommandWord){"inc", 1, 0, FUNCT_INC, 0, 0, 0, 0, OPCODE_INC},
    &(CommandWord){"dec", 1, 0, FUNCT_DEC, 0, 0, 0, 0, OPCODE_DEC},
    &(CommandWord){"jmp", 1, 0, FUNCT_JMP, 0, 0, 0, 0, OPCODE_JMP},
    &(CommandWord){"bne", 1, 0, FUNCT_BNE, 0, 0, 0, 0, OPCODE_BNE},
    &(CommandWord){"jsr", 1, 0, FUNCT_JSR, 0, 0, 0, 0, OPCODE_JSR},
    &(CommandWord){"red", 1, 0, 0, 0, 0, 0, 0, OPCODE_RED},
    &(CommandWord){"prn", 1, 0, 0, 0, 0, 0, 0, OPCODE_PRN},
    &(CommandWord){"rts", 1, 0, 0, 0, 0, 0, 0, OPCODE_RTS},
    &(CommandWord){"stop",1, 0, 0, 0, 0, 0, 0, OPCODE_STOP},
};

const char *operations[NUM_COMMANDS] = {
    "mov", "cmp", "add", "sub", "lea", "clr",
    "not", "inc", "dec", "jmp", "bne", "jsr",
    "red", "prn", "rts", "stop"
};
