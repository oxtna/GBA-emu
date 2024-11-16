#include"../cpu.h"
#include<array>
#include<utility>
#include<vector>
#include<iostream>

using namespace GBA;

InstructionType getOpcode(uint32_t opcode)
{
    CPU cpu;
    return cpu.decodeArm(opcode);
}

int main()
{
    std::vector<std::pair<uint32_t, InstructionType>> test_cases = {
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),
        std::make_pair<uint32_t, InstructionType>(0x00000000, InstructionType::Undefined),

    };
    bool failed = false;
    for(auto test_case : test_cases)
    {
        auto instruction_type = getOpcode(test_case.first);
        if(instruction_type != test_case.second)
        {
            failed = true;
            std::cerr << "Test failed for opcode [ " << std::hex << test_case.first << std::dec << " ]: expected " << getInstructionTypeString(test_case.second) << " but got " << getInstructionTypeString(instruction_type) << " instead\n";
        }
    }

    return failed;
}