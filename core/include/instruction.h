#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stddef.h>

#define JUMP_BIT 0x80

#define PUSH 0x0
#define PUSH_STRING 0x1
#define PUSH_VAR 0x2
#define PUSH_FIELD 0x3

#define POP 0x4
#define POP_VAR 0x5
#define POP_FIELD 0x6

#define ADD 0x7
#define SUB 0x8
#define MUL 0x9
#define DIV 0xA

#define CALL 0xB
#define RETURN 0xC

#define NEW 0xD

#define DUP 0xE

#define JUMP 0x80
#define JUMP_EQ 0x81
#define JUMP_NE 0x82
#define JUMP_LT 0x83
#define JUMP_LE 0x84
#define JUMP_GT 0x85
#define JUMP_GE 0x86

typedef struct
{
    uint8_t opcode;
    uint32_t operand;
} Instruction;

#endif