#include "cpu.h"
#include <iostream>

GBA::CPU::CPU() : registers{}, CPSR{}, SPSR_FIQ{}, SPSR_SVC{}, SPSR_ABT{}, SPSR_IRQ{}, SPSR_UND{} {
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::AND)] = &GBA::CPU::andArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::XOR)] = &GBA::CPU::xorArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::SUB)] = &GBA::CPU::subArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::RSB)] = &GBA::CPU::rsbArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ADD)] = &GBA::CPU::addArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ADC)] = &GBA::CPU::adcArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::SBC)] = &GBA::CPU::sbcArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::RSC)] = &GBA::CPU::rscArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::TST)] = &GBA::CPU::tstArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::TEQ)] = &GBA::CPU::teqArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::CMP)] = &GBA::CPU::cmpArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::CMN)] = &GBA::CPU::cmnArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::ORR)] = &GBA::CPU::orrArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::MOV)] = &GBA::CPU::movArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::BIC)] = &GBA::CPU::bicArm;
    data_processing_instruction_type[static_cast<int>(GBA::Opcode::MVN)] = &GBA::CPU::mvnArm;
}

constexpr bool isBranchAndExchange(uint32_t instruction_code) {
    return (instruction_code & 0x0FFFFFF0) == 0x012FFF10;
}

constexpr bool isBlockDataTransfer(uint32_t instruction_code) {
    return (instruction_code & 0x0E000000) == 0x08000000;
}

constexpr bool isBranch(uint32_t instruction_code) {
    return (instruction_code & 0x0E000000) == 0x0A000000;
}

constexpr bool isSoftwareInterrupt(uint32_t instruction_code) {
    return (instruction_code & 0x0F000000) == 0x0F000000;
}

constexpr bool isUndefined(uint32_t instruction_code) {
    return (instruction_code & 0x0E000010) == 0x06000010;
}

constexpr bool isSingleDataTransfer(uint32_t instruction_code) {
    return (instruction_code & 0x0C000000) == 0x04000000;
}

constexpr bool isSingleDataSwap(uint32_t instruction_code) {
    return (instruction_code & 0x0FB00FF0) == 0x01000090;
}

constexpr bool isMultiply(uint32_t instruction_code) {
    return (instruction_code & 0x0FC000F0) == 0x00000090;
}

constexpr bool isMultiplyLong(uint32_t instruction_code) {
    return (instruction_code & 0x0F8000F0) == 0x00800090;
}

constexpr bool isHalfwordDataTransferRegister(uint32_t instruction_code) {
    return (instruction_code & 0x0E400F90) == 0x00000090;
}

constexpr bool isHalfwordDataTransferImmediate(uint32_t instruction_code) {
    return (instruction_code & 0x0E400090) == 0x00400090;
}

constexpr bool isProgramStatusRegisterTransferOut(uint32_t instruction_code) {
    return (instruction_code & 0x0FBF0000) == 0x010F0000;
}

constexpr bool isProgramStatusRegisterTransferIn(uint32_t instruction_code) {
    return (instruction_code & 0x0DB0F000) == 0x0120F000;
}

constexpr bool isDataProcessing(uint32_t instruction_code) {
    return (instruction_code & 0x0C000000) == 0x00000000;
}

constexpr bool isCoprocessorDataTransfer(uint32_t instruction_code) {
    return (instruction_code & 0x0E000000) == 0x0C000000;
}

constexpr bool isCoprocessorDataOperation(uint32_t instruction_code) {
    return (instruction_code & 0x0F000010) == 0x0E000000;
}

constexpr bool isCoprocessorRegisterTransfer(uint32_t instruction_code) {
    return (instruction_code & 0x0F000010) == 0x0E000010;
}

GBA::InstructionType GBA::CPU::decodeArm(uint32_t instruction_code) {
    if (isBranchAndExchange(instruction_code))
        return GBA::InstructionType::BranchAndExchange;
    if (isBlockDataTransfer(instruction_code))
        return GBA::InstructionType::BlockDataTransfer;
    if (isBranch(instruction_code))
        return GBA::InstructionType::Branch;
    if (isSoftwareInterrupt(instruction_code))
        return GBA::InstructionType::SoftwareInterrupt;
    if (isUndefined(instruction_code))
        return GBA::InstructionType::Undefined;
    if (isSingleDataTransfer(instruction_code))
        return GBA::InstructionType::SingleDataTransfer;
    if (isSingleDataSwap(instruction_code))
        return GBA::InstructionType::SingleDataSwap;
    if (isMultiply(instruction_code))
        return GBA::InstructionType::Multiply;
    if (isMultiplyLong(instruction_code))
        return GBA::InstructionType::MultiplyLong;
    if (isHalfwordDataTransferRegister(instruction_code))
        return GBA::InstructionType::HalfwordDataTransferRegister;
    if (isHalfwordDataTransferImmediate(instruction_code))
        return GBA::InstructionType::HalfwordDataTransferImmediate;
    if (isProgramStatusRegisterTransferOut(instruction_code))
        return GBA::InstructionType::ProgramStatusRegisterTransferOut;
    if (isProgramStatusRegisterTransferIn(instruction_code))
        return GBA::InstructionType::ProgramStatusRegisterTransferIn;
    if (isDataProcessing(instruction_code))
        return GBA::InstructionType::DataProcessing;
    if (isCoprocessorDataTransfer(instruction_code))
        return GBA::InstructionType::CoprocessorDataTransfer;
    if (isCoprocessorDataOperation(instruction_code))
        return GBA::InstructionType::CoprocessorDataOperation;
    if (isCoprocessorRegisterTransfer(instruction_code))
        return GBA::InstructionType::CoprocessorRegisterTransfer;
    else
        return GBA::InstructionType::Undefined;
}

void GBA::CPU::reset() {
    // TODO: see declaration in header file
    R_SVC(14) = PC();  // Overwrites R14_svc and SPSR_svc by copying the current values of the PC and CPSR into them
    SPSR_SVC = CPSR;
    CPSR = 0xD3;  // sets 4:0 bits to 0b10011 and I and F bits to 1 (IRQ and FIQ disabled) T bit to 0 (ARM mode)
    PC() = 0;     // sets the PC to 0
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
    default:
        // TODO: invalid mode error handling
        throw;
    }
}

GBA::DataProcessingArguments GBA::CPU::decodeDataProcessingArguments(uint32_t instruction_code, Opcode opcode) {
    GBA::DataProcessingArguments arguments;
    if(opcode == GBA::Opcode::TEQ || opcode == GBA::Opcode::CMP || opcode == GBA::Opcode::CMN || opcode == GBA::Opcode::TST) {
        arguments.S = 1;
    }
    else {
        arguments.S = (instruction_code >> 20) & 0x1;
    }
    arguments.Rn = (instruction_code >> 16) & 0xF;
    arguments.Rd = (instruction_code >> 12) & 0xF;
    // I immediate bit (if 1, the operand is an immediate value)
    bool bit_25 = (instruction_code >> 25) & 0x1;
    uint32_t operand2 = instruction_code & 0xFFF;
    if (bit_25) {
        // 8-bit immediate value
        uint32_t bits_7_to_0 = operand2 & 0xFF;
        // Rotate right value bits
        uint32_t bits_11_to_8 = (operand2 >> 8) & 0xF;
        operand2 = (bits_7_to_0 >> bits_11_to_8) | (bits_7_to_0 << (32 - bits_11_to_8));
    }
    else {
        bool bit_4 = (instruction_code >> 4) & 0x1;
        // register to shift
        uint32_t Rm = instruction_code & 0xF;
        // shift type
        uint32_t bits_6_to_5 = (instruction_code >> 5) & 0x3;
        // shift amount
        uint32_t shift_amount;
        if (bit_4)
            shift_amount = (instruction_code >> 7) & 0x1F;
        else
            shift_amount = R((operand2 >> 8) & 0xF);

        switch (bits_6_to_5) {
        case 0b00:  // Logical shift left
        {
            operand2 = R(Rm) << shift_amount;
            break;
        }
        case 0b01:  // Logical shift right
        {
            operand2 = R(Rm) >> shift_amount;
            break;
        }
        case 0b10:  // Arithmetic shift right
        {
            int32_t signed_Rm = R(Rm);
            for (int32_t i = 0; i < shift_amount; i++) {
                if (signed_Rm < 0 && signed_Rm % 2 == 1) {
                    signed_Rm /= 2;
                    signed_Rm--;
                }
                else
                    signed_Rm /= 2;
            }
            operand2 = signed_Rm;
            break;
        }
        case 0b11:  // Rotate right
        {
            operand2 = (R(Rm) >> shift_amount) | (R(Rm) << (32 - shift_amount));
            break;
        }
        default:
            throw;  // TODO: invalid shift type error handling
        }
    }
    arguments.operand2 = operand2;
    return arguments;
}

void GBA::CPU::dataProcessingArmLogicalOperationFlagsSetting(bool S, uint32_t Rd, uint32_t operation_result) // for exapmple TST does not save its result in R(Rd) that's why we need it
{
    if (!S || Rd == 15)   // TODO setting carry flag
        return;
    if(operation_result & (1 << 31)) // setting negative flag
        CPSR |= (1 << 31);
    else 
        CPSR &= ~(1 << 31);

    if(R(operation_result) == 0) // setting zero flag
        CPSR |= (1 << 30);
    else 
        CPSR &= ~(1 << 30);

}

void GBA::CPU::dataProcessingArmArithmeticOperationFlagsSetting(bool S, uint32_t Rd_before_operation, uint32_t Rd, uint32_t result) {
    if (!S || Rd == 15) 
        return; 
   
    if (result & (1 << 31))  // setting negative flag
        CPSR |= (1 << 31); 
    else 
        CPSR &= ~(1 << 31); 
    
    if (result == 0)  // setting zero flag
        CPSR |= (1 << 30); 
     else 
        CPSR &= ~(1 << 30);

    if (Rd_before_operation > Rd) // setting carry flag TODO: check if it is correct
        CPSR |= (1 << 29);
    else 
        CPSR &= ~(1 << 29);
    

    bool sign_bit_result = (result >> 31) & 1;
    bool sign_bit_Rd = (Rd >> 31) & 1;
    bool sign_bit_Rd_before = (Rd_before_operation >> 31) & 1;

    if ((sign_bit_Rd_before == sign_bit_Rd) && (sign_bit_Rd != sign_bit_result))  // setting overflow flag
        CPSR |= (1 << 28);
    else 
        CPSR &= ~(1 << 28);
    
}

void GBA::CPU::andArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::AND);
    R(arguments.Rd) = R(arguments.Rn) & arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::xorArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::XOR);
    R(arguments.Rd) = R(arguments.Rn) ^ arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::subArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::SUB);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = R(arguments.Rn) - arguments.operand2;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::rsbArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::RSB);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = arguments.operand2 - R(arguments.Rn);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::addArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::ADD);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = R(arguments.Rn) + arguments.operand2;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::adcArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::ADC);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = R(arguments.Rn) + arguments.operand2 + ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::sbcArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::SBC);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = R(arguments.Rn) - arguments.operand2 - ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::rscArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::RSC);
    uint32_t Rd_before_operation = R(arguments.Rd);
    R(arguments.Rd) = arguments.operand2 - R(arguments.Rn) - ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::tstArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::TST);
    uint32_t result = R(arguments.Rn) & arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result);
}

void GBA::CPU::teqArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::TEQ);
    uint32_t result = R(arguments.Rn) ^ arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result);
}

void GBA::CPU::cmpArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::CMP);
    uint32_t result = R(arguments.Rn) - arguments.operand2;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, R(arguments.Rn), arguments.Rd, result);
}

void GBA::CPU::cmnArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::CMN);
    uint32_t result = R(arguments.Rn) + arguments.operand2;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, R(arguments.Rn), arguments.Rd, result);
}

void GBA::CPU::orrArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::ORR);
    R(arguments.Rd) = R(arguments.Rn) | arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::movArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::MOV);
    R(arguments.Rd) = arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::bicArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::BIC);
    R(arguments.Rd) = R(arguments.Rn) & ~arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::mvnArm(uint32_t instruction_code) {
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, GBA::Opcode::MVN);
    R(arguments.Rd) = ~arguments.operand2;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd));
}

bool GBA::CPU::checkCondition(uint32_t instruction_code) const {
    uint32_t condition = (instruction_code >> 28) & 0xF;
    bool negative_flag = (CPSR >> 31) & 0x1;
    bool zero_flag = (CPSR >> 30) & 0x1;
    bool carry_flag = (CPSR >> 29) & 0x1;
    bool overflow_flag = (CPSR >> 28) & 0x1;
    switch (condition) {
    case 0b0000:
        return zero_flag;
    case 0b0001:
        return !zero_flag;
    case 0b0010:
        return carry_flag;
    case 0b0011:
        return !carry_flag;
    case 0b0100:
        return negative_flag;
    case 0b0101:
        return !negative_flag;
    case 0b0110:
        return overflow_flag;
    case 0b0111:
        return !overflow_flag;
    case 0b1000:
        return carry_flag && !zero_flag;
    case 0b1001:
        return !carry_flag || zero_flag;
    case 0b1010:
        return negative_flag == overflow_flag;
    case 0b1011:
        return negative_flag != overflow_flag;
    case 0b1100:
        return !zero_flag && negative_flag == overflow_flag;
    case 0b1101:
        return zero_flag || negative_flag != overflow_flag;
    case 0b1110:
        return true;
    case 0b1111:  // This is never used/reserved flag so I am not sure what to do here
    default:
        throw;  // TODO: invalid condition error handling
    }
}

GBA::Opcode GBA::CPU::getOpcodeArm(uint32_t instruction_code) const {
    return static_cast<GBA::Opcode>((instruction_code >> 21) & 0xF);
}

void GBA::CPU::callDataProcessingInstruction(uint32_t instruction_code) {
    if (!checkCondition(instruction_code))
        return;  // If the condition is not met, do nothing, later check what should happen if anything

    // Do not touch this, it will break the code
    // Call the correct data processing method
    (this->*data_processing_instruction_type[static_cast<int>(getOpcodeArm(instruction_code))])(instruction_code);

    /* // This part needs to be moved to seperate function and will be used by instructions functions
    bool S = (instruction_code >> 20) & 0x1;    // status bit (if 1, the instruction updates the CPSR)
    uint32_t Rn = (instruction_code >> 16) & 0xF;
    uint32_t Rd = (instruction_code >> 12) & 0xF;
    uint32_t operand2 = instruction_code & 0xFFF; //temporary

    */

    throw;  // TODO: implement all the other cases
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

