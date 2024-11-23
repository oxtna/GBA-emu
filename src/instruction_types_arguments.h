#ifndef GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#define GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#include<cstdint>

namespace GBA {
    struct DataProcessingArguments {
        bool S;   // status bit (if 1, the instruction updates the CPSR)
        uint32_t Rn;
        uint32_t Rd;
        uint32_t operand2;
    };
    
}

#endif