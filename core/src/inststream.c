#include "config.h"
#include "inststream.h"

InstructionStream *inststream_new(uint32_t length)
{
    InstructionStream *inststream = (InstructionStream *)config._malloc(sizeof(InstructionStream));
    inststream->length = length;
    inststream->current = 0;
    inststream->instructions = (Instruction *)config._malloc(length * sizeof(Instruction));
}

void inststream_free(InstructionStream *inststream)
{
    config._free(inststream->instructions);
    inststream->instructions = NULL;
    config._free(inststream);
}
