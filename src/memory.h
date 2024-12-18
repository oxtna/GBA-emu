#ifndef GBA_MEMORY_H
#define GBA_MEMORY_H

#include "common.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace GBA {

class Memory
{
  public:
    // This should be mapped somehow
    // https://github.com/ayvacs/gba.js.org/blob/main/iodineGBA/core/Memory.js#L4975
    static const size_t MaxSize = 0xFFFFFFFF;
    static const size_t DisplayBufferAddress = 1 << 30;
    static const size_t DisplayBufferSize = 240 * 160 * 4;
    Memory();
    ~Memory();

    void loadBIOS(const std::vector<uint8_t>& bios);

    std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator> getDisplayBuffer() const;

    std::vector<uint8_t> memory;

  private:
};

}

#endif
