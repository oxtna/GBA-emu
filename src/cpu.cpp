#include "cpu.h"
#include <iostream>

GBA::CPU::CPU() : registers{} {
}

GBA::CPU::~CPU() {
}

GBA::InstructionType GBA::CPU::decodeArm(uint32_t opcode) {
    uint32_t cond = (opcode >> 28);
    uint32_t a = (opcode >> 25) & 0x7;
    switch (a) {
    case 0b001: 
        return GBA::InstructionType::DataProcessing;
    case 0b000: {
        uint32_t c = (opcode >> 22) & 0x7; // temp variable name bites 24:22
        uint32_t d = (opcode >> 4) & 0xF;// temp variable name bites 7:4
        uint32_t e = (opcode >> 8) & 0xF;// temp variable name bites 11:8
        uint32_t f = (opcode >> 22) & 0x1;// temp variable name bite 22
        if(d == 0b1001)
        {
            if(c == 0b000)
                return GBA::InstructionType::Multiply;
            else if(c == 0b010 || c == 0b011)
                return GBA::InstructionType::MultiplyLong;
            else if(c == 0b100 || c == 0b101)
                return GBA::InstructionType::SingleDataSwap;
        }
        if(d >= 0b1001)
        {
            if(e == 0b0000 && f == 0b0)
                return GBA::InstructionType::HalfwordDataTransferRegister;
            else if(f == 0b1)
                return GBA::InstructionType::HalfwordDataTransferImmediate;
        }
        else
        {
            uint32_t branch_exchange_check = (opcode >> 4) & 0xFFFFFF;
            if(branch_exchange_check == 0x12FFF1)
                return GBA::InstructionType::BranchAndExchange;
        }
        break;
    }
    case 0b011:
        return GBA::InstructionType::SingleDataTransfer;
    case 0b100:
        return GBA::InstructionType::BlockDataTransfer;
    case 0b101:
        return GBA::InstructionType::Branch;
    case 0b110:
        return GBA::InstructionType::CoprocessorDataTransfer;
    case 0b111: {
        bool b = (opcode >> 24) & 0x1;
        if(b)
            return GBA::InstructionType::SoftwareInterrupt;
        else
        {
            bool c = (opcode >> 4) & 0x1;
            if(c)
                return GBA::InstructionType::CoprocessorDataOperation;
            else
                return GBA::InstructionType::CoprocessorRegisterTransfer;
            
            break;
        }
    }
    default:
        break;
    }

    return GBA::InstructionType::Undefined;
}