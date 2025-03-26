#include "../include/encoder.h"

/* Encode .data/.string directive words */
void encodeDataWordNoARE(int value, char *bin) {
    uint32_t val24 = ((uint32_t)value << 3); /* Shift left by 3 bits, ARE=000 */
    intToBinary(val24, 24, bin);
}

/* Encode main command words */
void encodeCommandWord(const CommandWord *cmd,
                       int srcReg, int srcAddr,
                       int dstReg, int dstAddr,
                       char *bin) {
    uint32_t cw = 0;
    cw |= (cmd->opcode & 0x3F) << 18;   /* opcode bits [23-18] */
    cw |= (srcAddr & 0x3) << 16;        /* src addressing bits [17-16] */
    cw |= (srcReg & 0x7) << 13;         /* src register bits [15-13] */
    cw |= (dstAddr & 0x3) << 11;        /* dst addressing bits [12-11] */
    cw |= (dstReg & 0x7) << 8;          /* dst register bits [10-8] */
    cw |= (cmd->funct & 0x1F) << 3;     /* funct bits [7-3] */
    cw |= A;                            /* ARE absolute bits [2-0]=100 */
    intToBinary(cw, 24, bin);
}

/* Encode operands referencing labels (direct or relative) */
uint32_t encodeLabelOperand(const LabelTable *lblTable,
                            const char *labelName,
                            int currentIC,
                            int isRelative,
                            int *areOut) {
    Label *lbl = findLabel((LabelTable*)lblTable, labelName);
    if (!lbl) {
        fprintf(stderr, "Error: Undefined label '%s'\n", labelName);
        *areOut = 0;
        return 0;
    }
    if (lbl->isExternal) {
        *areOut = 1; /* external (E=001) */
        return 0;
    } else {
        *areOut = 2; /* relocatable (R=010) */
        return isRelative ? (lbl->address - currentIC) & 0x1FFFFF : lbl->address & 0x1FFFFF;
    }
}

/* Handle Label Operand encoding */
void handleLabelOperandC(
    const char *labelToken,
    int isRelativeMode,
    int currentIC,
    int *addressCounter,
    LabelTable *lblTable,
    MachineWord *words,
    int *wordIndex,
    WordType *types,
    ExtRefArray *extRefs
) {
    int areVal;
    uint32_t val21 = encodeLabelOperand(lblTable, labelToken,
                                        currentIC, isRelativeMode, &areVal);

    Label *lbl = findLabel(lblTable, labelToken);
    if (lbl && lbl->isExternal) {
        AddExtRef(extRefs, lbl->name, *addressCounter);
    }

    char bin[25];
    uint32_t val24 = (val21 << 3) | (areVal & 7); /* Shift left by 3, include ARE bits */
    intToBinary(val24, 24, bin);

    words[*wordIndex].mnemonic[0] = '\0';
    strcpy(words[*wordIndex].bin, bin);
    types[*wordIndex] = COMMAND_WORD;
    words[*wordIndex].address = *addressCounter;

    (*wordIndex)++;
    (*addressCounter)++;
}

/* Convert integer to binary string representation */
void intToBinary(uint32_t value, int bits, char *dest) {
    dest[bits] = '\0';
    for (int i = bits - 1; i >= 0; i--) {
        dest[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}
