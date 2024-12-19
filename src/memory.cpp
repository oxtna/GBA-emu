#include "memory.h"
#include <cstring>

GBA::Memory::Memory() : memory(MaxSize) {
    for(size_t i = 0; i < DisplayBufferSize; i += 4) {
        memory[DisplayBufferAddress + i] = 0x00;        // R
        memory[DisplayBufferAddress + i + 1] = 0x00;    // G
        memory[DisplayBufferAddress + i + 2] = 0x00;    // B
        memory[DisplayBufferAddress + i + 3] = 0xFF;    // A
    }
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
