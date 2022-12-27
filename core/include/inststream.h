#ifndef INSTSTREAM_H
#define INSTSTREAM_H

#include "instruction.h"

typedef struct
{
    uint32_t length;
    uint32_t current;
    Instruction *instructions;
} InstructionStream;

InstructionStream *inststream_new(uint32_t length);

void inststream_free(InstructionStream *inststream);

#endif