#include "emulator.h"

GBA::Emulator::Emulator() : memory{}, cpu{}, rom{} {
}

GBA::Emulator::Emulator(GBA::UniquePtr<uint8_t> rom) : memory{}, cpu{}, rom(std::move(rom)) {
}

GBA::Emulator::~Emulator() {
}

void GBA::Emulator::loadROM(GBA::UniquePtr<uint8_t> rom) {
    // TODO: reset registers, memory and display
    this->rom = std::move(rom);
}

bool GBA::Emulator::run() {
    // TODO
    return true;
}

