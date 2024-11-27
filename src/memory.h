#ifndef GBA_MEMORY_H
#define GBA_MEMORY_H

#include "common.h"
#include <vector>
#include <cstdint>

namespace GBA {

class Memory
{
  public:
    Memory();
    ~Memory();
    std::vector<uint8_t>memory;
  private:
  
};

}

#endif
