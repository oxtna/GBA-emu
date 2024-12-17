#ifndef GBA_MEMORY_H
#define GBA_MEMORY_H

#include "common.h"
#include <cstdint>
#include <vector>

namespace GBA {

class Memory
{
  public:
  // This should be mapped somehow
  // https://github.com/ayvacs/gba.js.org/blob/main/iodineGBA/core/Memory.js#L4975
    static const size_t MaxSize = 4294967296;
    Memory();
    ~Memory();

    void loadBIOS(const std::vector<uint8_t>& bios);

    std::vector<uint8_t> memory;

  private:
};

}

#endif
