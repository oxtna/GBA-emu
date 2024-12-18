#include "memory.h"
#include <cstring>

GBA::Memory::Memory() : memory(MaxSize) {
}

GBA::Memory::~Memory() {
}

void GBA::Memory::loadBIOS(const std::vector<uint8_t>& bios) {
    std::memcpy(memory.data(), bios.data(), bios.size());
}

std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>
    GBA::Memory::getDisplayBuffer() const {
    return std::make_pair(
        memory.begin() + DisplayBufferAddress, memory.begin() + DisplayBufferAddress + DisplayBufferSize);
}
