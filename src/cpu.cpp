#include "cpu.h"

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
        arguments.shifted_value = operand2 & 0xFF;
        arguments.shift_type = GBA::ShiftType::RotateRight;
        arguments.shift_value = (operand2 >> 8) & 0xF;
    }
    else {
        bool bit_4 = (instruction_code >> 4) & 0x1;
        // register to shift
        uint32_t Rm = instruction_code & 0xF;
        arguments.shifted_value = R(Rm);
        // shift type
        arguments.shift_type = static_cast<GBA::ShiftType>((instruction_code >> 5) & 0x3);
        // shift amount
        if (bit_4)
            arguments.shift_value = R((operand2 >> 8) & 0xF); 
        else
            arguments.shift_value = (instruction_code >> 7) & 0x1F;

        }
    return arguments;
}

void GBA::CPU::dataProcessingArmLogicalOperationFlagsSetting(bool S, uint32_t Rd, uint32_t operation_result, bool carry) 
{ // for example TST does not save its result in R(Rd) that's why we need it
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

    if(carry) // setting carry flag
        CPSR |= (1 << 29);
    else 
        CPSR &= ~(1 << 29);

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

    // TODO: setting carry flag
    // TODO: setting overflow flag
    
}

std::pair<uint32_t, bool> GBA::CPU::calculateOperand2(uint32_t shifted_value, uint32_t shift_value, ShiftType shift_type) {
    if(shift_value == 0)
        return {shifted_value, (CPSR >> 28) & 1}; // return current carry flag value

    switch (shift_type) {
    case GBA::ShiftType::LogicalLeft:{
        bool carry_out = (shifted_value >> (32 - shift_value)) & 0x1;
        return {shifted_value << shift_value, carry_out};
    }
    case GBA::ShiftType::LogicalRight:{
        bool carry_out = (shifted_value >> (shift_value - 1)) & 0x1;
        return {shifted_value >> shift_value, carry_out};
    }
    case GBA::ShiftType::ArithmeticRight:{
        int32_t signed_Rm = *reinterpret_cast<int32_t*>(&shifted_value);
        bool carry_out = (shifted_value >> (shift_value - 1)) & 0x1;
            for (int32_t i = 0; i < shift_value; i++) {
                if (signed_Rm < 0 && signed_Rm % 2 == 1) {
                    signed_Rm /= 2;
                    signed_Rm--;
                }
                else
                    signed_Rm /= 2;
            }
        return {*reinterpret_cast<uint32_t*>(&shifted_value), carry_out};
    }
    case GBA::ShiftType::RotateRight:{
        uint32_t carry_out = (shifted_value >> (shift_value - 1)) & 0x1;
        return {(shifted_value >> shift_value) | (shifted_value << (32 - shift_value)), carry_out};
    }
    default:
        throw;  // TODO: invalid shift type error handling
    }
}

void GBA::CPU::andArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) & operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::xorArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) ^ operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::subArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) - operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::rsbArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first - R(arguments.Rn);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::addArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) + operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::adcArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) + operand2.first + ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::sbcArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) - operand2.first - ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::rscArm(DataProcessingArguments arguments) {
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first - R(arguments.Rn) - ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd));
}

void GBA::CPU::tstArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) & operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result, operand2.second);
}

void GBA::CPU::teqArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) ^ operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result, operand2.second);
}

void GBA::CPU::cmpArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) - operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, R(arguments.Rn), arguments.Rd, result);
}

void GBA::CPU::cmnArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) + operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(arguments.S, R(arguments.Rn), arguments.Rd, result);
}

void GBA::CPU::orrArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) | operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::movArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::bicArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) & ~operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::mvnArm(DataProcessingArguments arguments) {
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = ~operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
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

    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, getOpcodeArm(instruction_code));
    (this->*data_processing_instruction_type[static_cast<int>(getOpcodeArm(instruction_code))])(arguments);
}

GBA::MultiplyArguments GBA::CPU::decodeMultiplyArguments(uint32_t instruction_code) {
    GBA::MultiplyArguments arguments;
    arguments.A = (instruction_code >> 20) & 0x1;
    arguments.S = (instruction_code >> 19) & 0x1;
    arguments.Rd = (instruction_code >> 15) & 0xF;
    arguments.Rn = (instruction_code >> 11) & 0xF;
    arguments.Rs = (instruction_code >> 7) & 0xF;
    arguments.Rm = instruction_code & 0xF;
    return arguments;
}

void GBA::CPU::callMultiplyInstruction(uint32_t instruction_code) {
    if(!checkCondition(instruction_code))
        return;

    GBA::MultiplyArguments arguments = decodeMultiplyArguments(instruction_code);
    if(arguments.A)
        R(arguments.Rd) = R(arguments.Rm) * R(arguments.Rs) + R(arguments.Rn);
    else
        R(arguments.Rd) = R(arguments.Rm) * R(arguments.Rs);

    if(arguments.S && arguments.Rd != 15) {
        if(R(arguments.Rd) & (1 << 31))
            CPSR |= (1 << 31);
        else
            CPSR &= ~(1 << 31);

        if(R(arguments.Rd) == 0)
            CPSR |= (1 << 30);
        else
            CPSR &= ~(1 << 30);

        CPSR &= ~(1 << 29); // Clearing carry flag it instruction says it should be set to meaningless value 
    }
}

GBA::MultiplyLongArguments GBA::CPU::decodeMultiplyLongArguments(uint32_t instruction_code) {
    GBA::MultiplyLongArguments arguments;
    arguments.U = (instruction_code >> 22) & 0x1;
    arguments.A = (instruction_code >> 21) & 0x1;
    arguments.S = (instruction_code >> 20) & 0x1;
    arguments.RdHi = (instruction_code >> 16) & 0xF;
    arguments.RdLo = (instruction_code >> 12) & 0xF;
    arguments.Rm = (instruction_code >> 8) & 0xF;
    arguments.Rs = instruction_code & 0xF;
    return arguments;
}

void GBA::CPU::umullArm(MultiplyLongArguments arguments){
    uint64_t result = R(arguments.Rm) * R(arguments.Rs);
    R(arguments.RdLo) = result & 0xFFFFFFFF;
    R(arguments.RdHi) = result >> 32;
}

void GBA::CPU::umlalArm(MultiplyLongArguments arguments){
    uint64_t accumulator = (static_cast<uint64_t>(R(arguments.RdHi)) << 32) | R(arguments.RdLo);
    uint64_t result = R(arguments.Rm) * R(arguments.Rs) + accumulator;
    R(arguments.RdLo) = result & 0xFFFFFFFF;
    R(arguments.RdHi) = (result >> 32) & 0xFFFFFFFF;
}

void GBA::CPU::smullArm(MultiplyLongArguments arguments){
    int64_t result = static_cast<int64_t>(arguments.Rm) * static_cast<int64_t>(R(arguments.Rs));
    int32_t temp = static_cast<int32_t>(result & 0xFFFFFFFF);
    R(arguments.RdLo) = *reinterpret_cast<uint32_t*>(&temp);
    temp = static_cast<int32_t>((result >> 32) & 0xFFFFFFFF);
    R(arguments.RdHi) = *reinterpret_cast<uint32_t*>(&temp);
}

void GBA::CPU::smlalArm(MultiplyLongArguments arguments){
    int64_t accumulator = (static_cast<int64_t>(R(arguments.RdHi)) << 32) | R(arguments.RdLo);
    int64_t result = static_cast<int64_t>(R(arguments.Rm)) * static_cast<int64_t>(R(arguments.Rs)) + accumulator;
    int32_t temp = static_cast<int32_t>(result & 0xFFFFFFFF);
    R(arguments.RdLo) = *reinterpret_cast<uint32_t*>(&temp);
    temp = static_cast<int32_t>((result >> 32) & 0xFFFFFFFF);
    R(arguments.RdHi) = *reinterpret_cast<uint32_t*>(&temp);
}

void GBA::CPU::callMultiplyLongInstruction(uint32_t instruction_code) {
    if(!checkCondition(instruction_code))
        return;

    GBA::MultiplyLongArguments arguments = decodeMultiplyLongArguments(instruction_code);
    if(arguments.U){
        if(arguments.A)
            umullArm(arguments);
        else
            smullArm(arguments);
    }
    else{
        if(arguments.A)
            umlalArm(arguments);
        else
            smlalArm(arguments);
    }

    if(arguments.S && arguments.RdHi != 15 && arguments.RdLo != 15){
        if(R(arguments.RdHi) & (1 << 31))
            CPSR |= (1 << 31);
        else
            CPSR &= ~(1 << 31);

        if(R(arguments.RdHi) == 0 && R(arguments.RdLo) == 0)
            CPSR |= (1 << 30);
        else
            CPSR &= ~(1 << 30);

        CPSR &= ~(1 << 29); // Clearing carry flag it instruction says it should be set to meaningless value 
        CPSR &= ~(1 << 28); // Clearing overflow flag it instruction says it should be set to meaningless value
    }  
}

GBA::SingleDataTransferArguments GBA::CPU::decodeSingleDataTransferArguments(uint32_t instruction_code) {
    GBA::SingleDataTransferArguments arguments;
    arguments.I = (instruction_code >> 25) & 0x1;
    arguments.P = (instruction_code >> 24) & 0x1;
    arguments.U = (instruction_code >> 23) & 0x1;
    arguments.B = (instruction_code >> 22) & 0x1;
    arguments.W = (instruction_code >> 21) & 0x1;
    arguments.L = (instruction_code >> 20) & 0x1;
    arguments.Rn = (instruction_code >> 16) & 0xF;
    arguments.Rd = (instruction_code >> 12) & 0xF;
    arguments.offset = instruction_code & 0xFFF;
    return arguments;
}

void GBA::CPU::ldrArm(SingleDataTransferArguments arguments){
    uint32_t address = R(arguments.Rn);
    uint32_t offset = 0;
    if(arguments.I)
    {
        uint32_t shifted_value = R(arguments.offset & 0xF);
        ShiftType shift_type = static_cast<ShiftType>((arguments.offset >> 5) & 0x3);
        bool bit_4 = (arguments.offset >> 4) & 0x1;
        uint32_t shift_value = 0;
        if (bit_4)
            shift_value = R((arguments.offset >> 8) & 0xF);
        else
            shift_value = (arguments.offset >> 7) & 0x1F;

        auto operand2 = calculateOperand2(shifted_value, shift_value, shift_type);
        offset = operand2.first;
    }
    else
    {
        offset = arguments.offset;
    }

    if(arguments.U)
        address += offset;
    else
        address -= offset;

    if(arguments.P) {// pre indexing
        if(arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    if(arguments.B)
        R(arguments.Rd) = memory.memory[address];
    else
    {
        R(arguments.Rd) = memory.memory[address];
        R(arguments.Rd) |= memory.memory[address + 1] << 8;
    }

    if(arguments.W == 1)
    {
        if(arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::strArm(SingleDataTransferArguments arguments){
    throw;
}

// LDR R0, [R1, #4]
// LDR R0, [R1, #4]!
// LDR R0, [R1], #4
void GBA::CPU::callSingleDataTransferInstruction(uint32_t instruction_code){
    if(!checkCondition(instruction_code))
        return;
    
    GBA::SingleDataTransferArguments arguments = decodeSingleDataTransferArguments(instruction_code);
    if(arguments.L)
        ldrArm(arguments);
    else
        strArm(arguments);
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

