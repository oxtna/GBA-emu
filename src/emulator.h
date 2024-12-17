#ifndef GBA_EMULATOR_H
#define GBA_EMULATOR_H

#include "common.h"
#include "cpu.h"
#include "memory.h"
#include <vector>

namespace GBA {

class Emulator
{
  public:
    Emulator();
    Emulator(const std::vector<uint8_t>& bios);
    ~Emulator();

    void loadBIOS(const std::vector<uint8_t>& bios);
    void step();

  private:
    CPU cpu;
};

}

#endif
