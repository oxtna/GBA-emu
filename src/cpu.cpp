#include "cpu.h"

GBA::CPU::CPU() : registers{} {
}

GBA::CPU::~CPU() {
}

GBA::Opcode GBA::CPU::decodeArm(uint32_t opcode) {
    uint32_t cond = (opcode >> 28);
    uint32_t a = (opcode >> 25) & 0x7;
    if(a == 1)
        return static_cast<Opcode>((opcode >> 21) & 0xF);
    if(a == 0)
    {
        
    }

}