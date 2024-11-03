#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "common.h"
#include "opcode.h"

namespace GBA {

class CPU
{
  public:
    CPU();
    ~CPU();
    Opcode decodeArm(uint32_t opcode);

  private:
    uint32_t registers[16];
};

}

#endif
