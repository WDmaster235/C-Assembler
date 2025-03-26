#include "commands.h"

/* Actual CommandWord definitions stored statically */
static const CommandWord command_list[NUM_COMMANDS] = {
    {"mov", 0, 0, 0 , 0, 0, 0, 0, OPCODE_MOV},
    {"cmp", 0, 0, 0 , 0, 0, 0, 0, OPCODE_CMP},
    {"add", 0, 0, FUNCT_ADD , 0, 0, 0, 0, OPCODE_ADD},
    {"sub", 0, 0, FUNCT_SUB, 0, 0, 0, 0, OPCODE_SUB},
    {"lea", 0, 0, 0, 0, 0, 0, 0, OPCODE_LEA},
    {"clr", 1, 0, FUNCT_CLR, 0, 0, 0, 0, OPCODE_CLR},
    {"not", 1, 0, FUNCT_NOT, 0, 0, 0, 0, OPCODE_NOT},
    {"inc", 1, 0, FUNCT_INC, 0, 0, 0, 0, OPCODE_INC},
    {"dec", 1, 0, FUNCT_DEC, 0, 0, 0, 0, OPCODE_DEC},
    {"jmp", 1, 0, FUNCT_JMP, 0, 0, 0, 0, OPCODE_JMP},
    {"bne", 1, 0, FUNCT_BNE, 0, 0, 0, 0, OPCODE_BNE},
    {"jsr", 1, 0, FUNCT_JSR, 0, 0, 0, 0, OPCODE_JSR},
    {"red", 1, 0, 0, 0, 0, 0, 0, OPCODE_RED},
    {"prn", 1, 0, 0, 0, 0, 0, 0, OPCODE_PRN},
    {"rts", 1, 0, 0, 0, 0, 0, 0, OPCODE_RTS},
    {"stop",1, 0, 0, 0, 0, 0, 0, OPCODE_STOP}
};

/* Array of pointers to the CommandWord structs */
const CommandWord *commands[NUM_COMMANDS] = {
    &command_list[0], &command_list[1], &command_list[2], &command_list[3],
    &command_list[4], &command_list[5], &command_list[6], &command_list[7],
    &command_list[8], &command_list[9], &command_list[10], &command_list[11],
    &command_list[12], &command_list[13], &command_list[14], &command_list[15]
};

/* Mnemonic names for quick lookup */
const char *operations[NUM_COMMANDS] = {
    "mov", "cmp", "add", "sub", "lea", "clr",
    "not", "inc", "dec", "jmp", "bne", "jsr",
    "red", "prn", "rts", "stop"
};
