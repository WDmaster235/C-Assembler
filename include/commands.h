#ifndef COMMANDS_H
#define COMMANDS_H

#include "definitions.h"

extern const char *operations[NUM_COMMANDS];

int isNegative(int num);
int Mov(int *num1, int *num2);
int Cmp(int *num1, int *num2);
int Add(int *num1, int *num2);
int Sub(int *num1, int *num2);
int Clr(int *reg);
int Not(int *num);
int Inc(int *num);
int Dec(int *num);
int Jmp(int *address);
int Bne(int *address);
int Jsr(int *address);
int Red(char chr);
int Prn(char chr);
int Rts();
int Stop();

#endif
