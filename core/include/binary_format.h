#ifndef BINARY_FORMAT_H
#define BINARY_FORMAT_H

#include <stdio.h>

#include "constantpool.h"
#include "inststream.h"

void binform_write_constantpool(FILE *file, ConstantPool *constpool);

void binform_write_instructions(FILE *file, InstructionStream *inststream);

ConstantPool *binform_read_constantpool(FILE *file);

InstructionStream *binform_read_instructions(FILE *file);

#endif