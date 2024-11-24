#ifndef GBA_MEMORY_H
#define GBA_MEMORY_H

#include "common.h"
#include <vector>

namespace GBA {

class Memory
{
  public:
    Memory();
    ~Memory();
    std::vector<std::byte>memory;
  private:
  
};

}

#endif
