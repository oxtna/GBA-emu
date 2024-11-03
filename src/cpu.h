#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "common.h"

namespace GBA {

class CPU
{
  public:
    CPU();
    ~CPU();

  private:
    uint32_t registers[16];
};

}

#endif
