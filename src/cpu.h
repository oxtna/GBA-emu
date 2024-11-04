#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "common.h"
#include "opcode.h"
#include "instruction_types.h"

namespace GBA {

class CPU
{
  public:
    CPU();
    ~CPU();
    InstructionType decodeArm(uint32_t opcode);

  private:
    uint32_t registers[16];
};

}

#endif
