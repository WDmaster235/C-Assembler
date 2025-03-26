#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#define MAX_LINE_LENGTH 256
#define INPUT_FILE_EXTENSION       ".as"
#define OBJECT_FILE_EXTENSION      ".ob"
#define EXTENDED_FILE_EXTENSION    ".am"
#define ENTRIES_FILE_EXTENSION     ".ent"
#define EXTERNALS_FILE_EXTENSION   ".ext"


#ifdef TEST_MODE

#define INPUT_FP                   "tests/input"
#define EXTENDED_OUTPUT_FP         "output/extended"
#define OBJECT_OUTPUT_FP           "output/object"
#define ENTRIES_OUTPUT_FP          "output/entries"
#define EXTERNALS_OUTPUT_FP        "output/externals"

#endif


#define MAX_LINE_LENGTH   128


#define OPCODE_MOV     0
#define OPCODE_CMP     1
#define OPCODE_ADD     2
#define OPCODE_SUB     2
#define OPCODE_LEA     4
#define OPCODE_CLR     5
#define OPCODE_NOT     5
#define OPCODE_INC     5
#define OPCODE_DEC     5
#define OPCODE_JMP     9
#define OPCODE_BNE     9
#define OPCODE_JSR     9
#define OPCODE_RED    12
#define OPCODE_PRN    13
#define OPCODE_RTS    14
#define OPCODE_STOP   15


#define FUNCT_ADD      1
#define FUNCT_SUB      2
#define FUNCT_CLR      1
#define FUNCT_NOT      2
#define FUNCT_INC      3
#define FUNCT_DEC      4
#define FUNCT_JMP      1
#define FUNCT_BNE      2
#define FUNCT_JSR      3


#define ADDRESS_IMMEDIATE      0  
#define ADDRESS_DIRECT         1
#define ADDRESS_RELATIVE       2 
#define ADDRESS_REGISTER       3


#define STATUS_CATASTROPHIC   -1  
#define STATUS_NO_RESULT       1  
#define STATUS_WRONG           2


#define MAX_SYMBOLS            128
#define MAX_SYMBOL_NAME        32


#define WORD_SIZE              24 
#define OUTPUT_BASE            6  

#define COMMENT_CHAR   ';'       
#define LABEL_DELIM    ':'       
#define MACRO_START    "mcro"    
#define MACRO_END      "mcroend" 


#define MAX_TAG_MEMORY 1024


#define MAX_LABELS 128


#define MAX_REGISTER 8
#define MAX_SYMBOL_NAME 32
#define MAX_LINE_LENGTH 256
#define NUM_COMMANDS    16
#define MAX_MNEMONIC_LENGTH 20

#endif 
