#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

#define PUSH 0
#define PUSH_CONST 1
#define PUSH_VAR 2
#define PUSH_FIELD 3

#define POP 4
#define POP_VAR 5
#define POP_FIELD 6

#define ADD 7
#define SUB 8
#define MUL 9
#define DIV 10

#define JUMP 11
#define JUMP_EQ 12
#define JUMP_NE 13
#define JUMP_LT 14
#define JUMP_LE 15
#define JUMP_GT 16
#define JUMP_GE 17

#define CALL 18
#define RETURN 19

#define NEW 20

#define DUP 21

typedef struct
{
    uint8_t opcode;
    uint32_t operand;
} Instruction;

#endif