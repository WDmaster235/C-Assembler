#ifndef ENCODER_H
#define ENCODER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "commands.h"
#include "label.h"
#include "definitions.h"

typedef struct {
    char bin[25];  /* 24-bit binary string */
    char mnemonic[MAX_MNEMONIC_LENGTH];
    int address;   /* final memory address for this word */
} MachineWord;

typedef enum {
    DATA_WORD,
    COMMAND_WORD
} WordType;

void encodeDataWordNoARE(int value, char *bin);

void encodeCommandWord(const CommandWord *cmd,
                       int srcReg, int srcAddr,
                       int dstReg, int dstAddr,
                       char *bin);

uint32_t encodeLabelOperand(const LabelTable *lblTable,
                            const char *labelName,
                            int currentIC,
                            int isRelative,
                            int *areOut);

void intToBinary(uint32_t value, int bits, char *dest);

#endif /* ENCODER_H */
