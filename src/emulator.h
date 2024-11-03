#ifndef GBA_EMULATOR_H
#define GBA_EMULATOR_H

#include "common.h"
#include "cpu.h"
#include "memory.h"

namespace GBA {

class Emulator
{
  public:
    Emulator();
    Emulator(UniquePtr<uint8_t> rom);
    ~Emulator();

    void loadROM(UniquePtr<uint8_t> rom);
    bool run();

  private:
    Memory memory;
    CPU cpu;
    UniquePtr<uint8_t> rom;
};

}

#endif
