#include "emulator.h"

GBA::Emulator::Emulator() : cpu{} {
    this->cpu.reset();
}

GBA::Emulator::Emulator(const std::vector<uint8_t>& bios) : cpu{} {
    this->cpu.loadBIOS(bios);
    this->cpu.reset();
}

GBA::Emulator::~Emulator() {
}

void GBA::Emulator::loadBIOS(const std::vector<uint8_t>& bios) {
    // TODO: reset registers, memory and display
    this->cpu.loadBIOS(bios);
    this->cpu.reset();
}

void GBA::Emulator::step() {
    this->cpu.step();
}
