#include <../include/commands.h>

const char *operations[NUM_COMMANDS] = {
    "mov", "cmp", "add", "sub", "lea", "clr",
    "not", "inc", "dec", "jmp", "bne", "jsr",
    "red", "prn", "rts", "stop"
};

int isNegative(int num) {
    return (num & (1 << 23)) != 0;
}

int Mov(int *num1, int *num2){
    num2 = num1;
    return 0;
}

int Cmp(int *num1, int *num2){
    if(num1 == num2)
        return 1;
    return 0;
}

int Add(int *num1, int *num2){
    // if()
    num2 += *num1;

    return 0;
}

int Sub(int *num1, int *num2){
    num2 -= *num1;

    return 0;
}

// int Lea(Label *label, int *reg){

// }

int Clr(int *reg){
    return 0;
}

int Not(int *num){
    return 0;
}

int Inc(int *num){
    return 0;
}

int Dec(int *num){
    return 0;
}

int Jmp(int *address){
    return 0;
}

int Bne(int *address){
    return 0;
}

int Jsr(int *address){
    return 0;
}

int Red(char chr){
    return 0;
}

int Prn(char chr){
    return 0;
}

int Rts(){
    return 0;
}

int Stop(){
    return 0;
}