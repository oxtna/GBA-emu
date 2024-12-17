#include "memory.h"
#include <cstring>

GBA::Memory::Memory() : memory(MaxSize) {
}

GBA::Memory::~Memory() {
}

void GBA::Memory::loadBIOS(const std::vector<uint8_t>& bios) {
    std::memcpy(memory.data(), bios.data(), bios.size());
}
