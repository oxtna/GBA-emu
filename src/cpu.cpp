#include "cpu.h"
#include <iostream>


GBA::CPU::CPU() : registers{}, CPSR{}, SPSR_FIQ{}, SPSR_SVC{}, SPSR_ABT{}, SPSR_IRQ{}, SPSR_UND{} {
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::AND)] = &GBA::CPU::ANDArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::XOR)] = &GBA::CPU::XORArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::SUB)] = &GBA::CPU::SUBArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::RSB)] = &GBA::CPU::RSBArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ADD)] = &GBA::CPU::ADDArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ADC)] = &GBA::CPU::ADCArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::SBC)] = &GBA::CPU::SBCArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::RSC)] = &GBA::CPU::RSCArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::TST)] = &GBA::CPU::TSTArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::TEQ)] = &GBA::CPU::TEQArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::CMP)] = &GBA::CPU::CMPArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::CMN)] = &GBA::CPU::CMNArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ORR)] = &GBA::CPU::ORRArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::MOV)] = &GBA::CPU::MOVArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::BIC)] = &GBA::CPU::BICArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::MVN)] = &GBA::CPU::MVNArm;
}

// Opcode bits increase right to left, leftmost being the most significant
GBA::InstructionType GBA::CPU::decodeArm(uint32_t opcode) {
    uint32_t bits_27_to_25 = (opcode >> 25) & 0x7;
    switch (bits_27_to_25) {
    case 0b001:
        return GBA::InstructionType::DataProcessing;
    case 0b000: {
        uint32_t bits_27_to_4 = (opcode >> 4) & 0xFFFFFF;
        uint32_t bits_24_to_22 = (opcode >> 22) & 0x7;
        uint32_t bits_11_to_8 = (opcode >> 8) & 0xF;
        uint32_t bits_7_to_4 = (opcode >> 4) & 0xF;
        bool bit_22 = (opcode >> 22) & 0x1;
        if (bits_7_to_4 == 0b1001) {
            if (bits_24_to_22 == 0b000)
                return GBA::InstructionType::Multiply;
            else if ((bits_24_to_22 & 0b110) == 0b010)
                return GBA::InstructionType::MultiplyLong;
            else if ((bits_24_to_22 & 0b110) == 0b100)
                return GBA::InstructionType::SingleDataSwap;
        }
        if (bits_7_to_4 >= 0b1001) {
            if (!bit_22 && bits_11_to_8 == 0b0000)
                return GBA::InstructionType::HalfwordDataTransferRegister;
            else if (bit_22)
                return GBA::InstructionType::HalfwordDataTransferImmediate;
            else
                return GBA::InstructionType::Undefined;
        }
        else if (bits_27_to_4 == 0x12FFF1) {
            return GBA::InstructionType::BranchAndExchange;
        }
        return GBA::InstructionType::Undefined;
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
        bool bit_24 = (opcode >> 24) & 0x1;
        bool bit_4 = (opcode >> 4) & 0x1;
        if (bit_24)
            return GBA::InstructionType::SoftwareInterrupt;
        else if (bit_4)
            return GBA::InstructionType::CoprocessorDataOperation;
        else
            return GBA::InstructionType::CoprocessorRegisterTransfer;
    }
    default:
        return GBA::InstructionType::Undefined;
    }
}

void GBA::CPU::reset() {
    // TODO: see declaration in header file
    R_SVC(14) = PC();   // Overwrites R14_svc and SPSR_svc by copying the current values of the PC and CPSR into them
    SPSR_SVC = CPSR;    
    CPSR = 0xD3;        // sets 4:0 bits to 0b10011 and I and F bits to 1 (IRQ and FIQ disabled) T bit to 0 (ARM mode)
    PC() = 0;           // sets the PC to 0
    // TODO
    // Reverts to ARM state if necessary and resumes execution.
}

GBA::CPU::Mode GBA::CPU::getMode() const {
    uint32_t mode = CPSR & 0x1F;
    switch (mode) {
    case 0b10000:
        return GBA::CPU::Mode::User;
    case 0b10001:
        return GBA::CPU::Mode::FastInterrupt;
    case 0b10010:
        return GBA::CPU::Mode::Interrupt;
    case 0b10011:
        return GBA::CPU::Mode::Supervisor;
    case 0b10111:
        return GBA::CPU::Mode::Abort;
    case 0b11011:
        return GBA::CPU::Mode::Undefined;
    case 0b11111:
        return GBA::CPU::Mode::System;
    }

    //TODO: invalid mode error handling
    throw;
}

void GBA::CPU::ANDArm(uint32_t instruction_code){}
void GBA::CPU::XORArm(uint32_t instruction_code){}
void GBA::CPU::SUBArm(uint32_t instruction_code){}
void GBA::CPU::RSBArm(uint32_t instruction_code){}
void GBA::CPU::ADCArm(uint32_t instruction_code){}
void GBA::CPU::SBCArm(uint32_t instruction_code){}
void GBA::CPU::RSCArm(uint32_t instruction_code){}
void GBA::CPU::TSTArm(uint32_t instruction_code){}
void GBA::CPU::TEQArm(uint32_t instruction_code){}
void GBA::CPU::CMPArm(uint32_t instruction_code){}
void GBA::CPU::CMNArm(uint32_t instruction_code){}
void GBA::CPU::ORRArm(uint32_t instruction_code){}
void GBA::CPU::MOVArm(uint32_t instruction_code){}
void GBA::CPU::BICArm(uint32_t instruction_code){}
void GBA::CPU::MVNArm(uint32_t instruction_code){}

bool GBA::CPU::checkCondition(uint32_t instruction_code) const{
    uint32_t condition = (instruction_code >> 28) & 0xF;
    uint32_t N = (CPSR >> 31) & 0x1;
    uint32_t Z = (CPSR >> 30) & 0x1;
    uint32_t C = (CPSR >> 29) & 0x1;
    uint32_t V = (CPSR >> 28) & 0x1;
    switch (condition) {
    case 0b0000: return Z == 1;
    case 0b0001: return Z == 0;
    case 0b0010: return C == 1;
    case 0b0011: return C == 0;
    case 0b0100: return N == 1;
    case 0b0101: return N == 0;
    case 0b0110: return V == 1;
    case 0b0111: return V == 0;
    case 0b1000: return C == 1 && Z == 0;
    case 0b1001: return C == 0 || Z == 1;   
    case 0b1010: return N == V;
    case 0b1011: return N != V;
    case 0b1100: return Z == 0 && N == V;
    case 0b1101: return Z == 1 || N != V;
    case 0b1110: return true;
    case 0b1111: throw; //This is never used/reserved flag so I am not sure what to do here
    default: throw; //TODO: invalid condition error handling
    }
}

GBA::Opcode GBA::CPU::getOpcodeArm(uint32_t instruction_code) const {
    return static_cast<GBA::Opcode>((instruction_code >> 21) & 0xF);
}

void GBA::CPU::callDataProcessingInstruction(uint32_t instruction_code)
{
    if(!checkCondition(instruction_code))
        return; //If the condition is not met, do nothing, later check what should happen if anything
    
    (this->*data_processing_instruction_type[
        static_cast<int>(getOpcodeArm(instruction_code))    // Do not touch it will break the code
        ])(instruction_code);

    /* // This part needs to be moved to seperate function and will be used by instructions functions
    bool S = (instruction_code >> 20) & 0x1;    // status bit (if 1, the instruction updates the CPSR)
    uint32_t Rn = (instruction_code >> 16) & 0xF;
    uint32_t Rd = (instruction_code >> 12) & 0xF;
    uint32_t operand2 = instruction_code & 0xFFF; //temporary

    */
    
    throw; //TODO: implement all the other cases
}

void GBA::CPU::ADDArm(uint32_t instruction_code)
{
    // S status bit (if 1, the instruction updates the CPSR)
    bool bit_20 = (instruction_code >> 20) & 0x1;    
    uint32_t Rn = (instruction_code >> 16) & 0xF;
    uint32_t Rd = (instruction_code >> 12) & 0xF;
    // I immediate bit (if 1, the operand is an immediate value)
    bool bit_25 = (instruction_code >> 25) & 0x1;
    uint32_t operand2 = instruction_code & 0xFFF;
    if(bit_25)
    {
        // 8-bit immediate value
        uint32_t bits_0_to_7 = operand2 & 0xFF;
        // Rotate right value bits 
        uint32_t bits_8_to_11 = (operand2 >> 8) & 0xF;
        operand2 = (bits_0_to_7 >> bits_8_to_11) | (bits_0_to_7 << (32 - bits_8_to_11));
    }
    else
    {
        bool bit_4 = (instruction_code >> 4) & 0x1;
        // register to shift
        uint32_t Rm = instruction_code & 0xF;
        // shift type
        uint32_t bits_5_to_6 = (instruction_code >> 5) & 0x3;
        // shift amount
        uint32_t shift_amount;
        if(bit_4)
            shift_amount = (instruction_code >> 7) & 0x1F;
        else
            shift_amount = R((operand2 >> 8) & 0xF);

        switch (bits_5_to_6)
        {
        case 0b00: // Logical shift left
        {   
            operand2 = R(Rm) << shift_amount;
            break;
        }
        case 0b01: // Logical shift right
        {
            operand2 = R(Rm) >> shift_amount;
            break;
        }
        case 0b10: // Arithmetic shift right
        {
            
            break;
        }
        case 0b11: // Rotate right
        {           
            operand2 = (R(Rm) >> shift_amount) | (R(Rm) << (32 - shift_amount));
            break;
        }
        default:
            throw; //TODO: invalid shift type error handling
        }
    }
    R(Rd) = R(Rn) + operand2;
}

uint32_t& GBA::CPU::SP(GBA::CPU::Mode mode) {
    if (mode == GBA::CPU::Mode::User || mode == GBA::CPU::Mode::System) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP)];
    }
    else if (mode == GBA::CPU::Mode::FastInterrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_FIQ)];
    }
    else if (mode == GBA::CPU::Mode::Supervisor) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_SVC)];
    }
    else if (mode == GBA::CPU::Mode::Abort) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_ABT)];
    }
    else if (mode == GBA::CPU::Mode::Interrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_IRQ)];
    }
    else if (mode == GBA::CPU::Mode::Undefined) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_UND)];
    }
    // TODO: invalid mode error handling
    throw;
}

const uint32_t& GBA::CPU::SP(GBA::CPU::Mode mode) const {
    if (mode == GBA::CPU::Mode::User || mode == GBA::CPU::Mode::System) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP)];
    }
    else if (mode == GBA::CPU::Mode::FastInterrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_FIQ)];
    }
    else if (mode == GBA::CPU::Mode::Supervisor) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_SVC)];
    }
    else if (mode == GBA::CPU::Mode::Abort) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_ABT)];
    }
    else if (mode == GBA::CPU::Mode::Interrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_IRQ)];
    }
    else if (mode == GBA::CPU::Mode::Undefined) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::SP_UND)];
    }
    // TODO: invalid mode error handling
    throw;
}

uint32_t& GBA::CPU::LR(GBA::CPU::Mode mode) {
    if (mode == GBA::CPU::Mode::User || mode == GBA::CPU::Mode::System) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR)];
    }
    else if (mode == GBA::CPU::Mode::FastInterrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_FIQ)];
    }
    else if (mode == GBA::CPU::Mode::Supervisor) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_SVC)];
    }
    else if (mode == GBA::CPU::Mode::Abort) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_ABT)];
    }
    else if (mode == GBA::CPU::Mode::Interrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_IRQ)];
    }
    else if (mode == GBA::CPU::Mode::Undefined) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_UND)];
    }
    // TODO: invalid mode error handling
    throw;
}

const uint32_t& GBA::CPU::LR(GBA::CPU::Mode mode) const {
    if (mode == GBA::CPU::Mode::User || mode == GBA::CPU::Mode::System) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR)];
    }
    else if (mode == GBA::CPU::Mode::FastInterrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_FIQ)];
    }
    else if (mode == GBA::CPU::Mode::Supervisor) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_SVC)];
    }
    else if (mode == GBA::CPU::Mode::Abort) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_ABT)];
    }
    else if (mode == GBA::CPU::Mode::Interrupt) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_IRQ)];
    }
    else if (mode == GBA::CPU::Mode::Undefined) {
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::LR_UND)];
    }
    // TODO: invalid mode error handling
    throw;
}

uint32_t& GBA::CPU::PC() {
    return registers[static_cast<int>(GBA::CPU::RegisterIndex::PC)];
}

const uint32_t& GBA::CPU::PC() const {
    return registers[static_cast<int>(GBA::CPU::RegisterIndex::PC)];
}

uint32_t& GBA::CPU::R(uint32_t index) {
    // TODO: error handling
    if (index > 15) {
        throw;
    }
    return registers[index];
}

const uint32_t& GBA::CPU::R(uint32_t index) const {
    // TODO: error handling
    if (index > 15) {
        throw;
    }
    return registers[index];
}

uint32_t& GBA::CPU::R_FIQ(uint32_t index) {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return registers[index];
    case 8:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R8_FIQ)];
    case 9:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R9_FIQ)];
    case 10:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R10_FIQ)];
    case 11:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R11_FIQ)];
    case 12:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R12_FIQ)];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_FIQ)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_FIQ)];
    default:
        // TODO: error handling
        throw;
    }
}

const uint32_t& GBA::CPU::R_FIQ(uint32_t index) const {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return registers[index];
    case 8:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R8_FIQ)];
    case 9:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R9_FIQ)];
    case 10:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R10_FIQ)];
    case 11:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R11_FIQ)];
    case 12:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R12_FIQ)];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_FIQ)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_FIQ)];
    default:
        // TODO: error handling
        throw;
    }
}

uint32_t& GBA::CPU::R_SVC(uint32_t index) {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_SVC)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_SVC)];
    default:
        // TODO: error handling
        throw;
    }
}

const uint32_t& GBA::CPU::R_SVC(uint32_t index) const {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_SVC)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_SVC)];
    default:
        // TODO: error handling
        throw;
    }
}

uint32_t& GBA::CPU::R_ABT(uint32_t index) {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_ABT)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_ABT)];
    default:
        // TODO: error handling
        throw;
    }
}

const uint32_t& GBA::CPU::R_ABT(uint32_t index) const {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_ABT)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_ABT)];
    default:
        // TODO: error handling
        throw;
    }
}

uint32_t& GBA::CPU::R_IRQ(uint32_t index) {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_IRQ)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_IRQ)];
    default:
        // TODO: error handling
        throw;
    }
}

const uint32_t& GBA::CPU::R_IRQ(uint32_t index) const {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_IRQ)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_IRQ)];
    default:
        // TODO: error handling
        throw;
    }
}

uint32_t& GBA::CPU::R_UND(uint32_t index) {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_UND)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_UND)];
    default:
        // TODO: error handling
        throw;
    }
}

const uint32_t& GBA::CPU::R_UND(uint32_t index) const {
    switch (index) {
    case 15:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return registers[index];
    case 13:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R13_UND)];
    case 14:
        return registers[static_cast<int>(GBA::CPU::RegisterIndex::R14_UND)];
    default:
        // TODO: error handling
        throw;
    }
}

