#include "cpu.h"
#include "instruction_types_arguments.h"
#include "opcode.h"
#include <iomanip>

// todo: 0xff4f0fe3 is failing

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

void GBA::CPU::loadBIOS(const std::vector<uint8_t>& bios) {
    this->memory.loadBIOS(bios);
}

std::pair<std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator> GBA::CPU::getDisplay() const {
    return memory.getDisplayBuffer();
}

void GBA::CPU::step() {
    if (inArm()) {
        uint32_t pc = PC();
        uint32_t instruction_code = memory.memory[pc];
        instruction_code |= memory.memory[pc + 1] << 8;
        instruction_code |= memory.memory[pc + 2] << 16;
        instruction_code |= memory.memory[pc + 3] << 24;
        PC() += 4;
        switch (decodeArm(instruction_code)) {
        case InstructionType::DataProcessing:
        case InstructionType::ProgramStatusRegisterTransferOut:
        case InstructionType::ProgramStatusRegisterTransferIn:
            callDataProcessingInstruction(instruction_code, pc);
            break;
        case InstructionType::Multiply:
            callMultiplyInstruction(instruction_code, pc);
            break;
        case InstructionType::MultiplyLong:
            callMultiplyLongInstruction(instruction_code, pc);
            break;
        case InstructionType::SingleDataSwap:
            callSingleDataSwapInstruction(instruction_code, pc);
            break;
        case InstructionType::BranchAndExchange:
            callBranchAndExchangeInstruction(instruction_code, pc);
            break;
        case InstructionType::HalfwordDataTransferRegister:
        case InstructionType::HalfwordDataTransferImmediate:
            callHalfWordAndSignedDataTransferInstruction(instruction_code, pc);
            break;
        case InstructionType::SingleDataTransfer:
            callSingleDataTransferInstruction(instruction_code, pc);
            break;
        case InstructionType::BlockDataTransfer:
            callBlockDataTransferInstruction(instruction_code, pc);
            break;
        case InstructionType::Branch:
            callBranchInstruction(instruction_code, pc);
            break;
        case InstructionType::CoprocessorDataTransfer:
            // call(instruction_code);
            throw;
            break;
        case InstructionType::CoprocessorDataOperation:
            // call(instruction_code);
            throw;
            break;
        case InstructionType::CoprocessorRegisterTransfer:
            // call(instruction_code);
            throw;
            break;
        case InstructionType::SoftwareInterrupt:
            callSoftwareInterruptInstruction(instruction_code, pc);
            break;
        case InstructionType::Undefined:
            // call(instruction_code);
        default:
            throw;
        }
    }
}

// 16-bit Thumb instructions types

constexpr bool isAddSubstractThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF800) == 0x1800;
}

constexpr bool isMoveShiftedRegisterThumb(uint16_t instruction_code) {
    return (instruction_code & 0xE000) == 0x0000;
}

constexpr bool isMoveCompareAddSubtractImmediateThumb(uint16_t instruction_code) {
    return (instruction_code & 0xE000) == 0x2000;
}

constexpr bool isAluOperationThumb(uint16_t instruction_code) {
    return (instruction_code & 0xFC00) == 0x4000;
}

constexpr bool isHiRegisterOperationsBranchExchangeThumb(uint16_t instruction_code) {
    return (instruction_code & 0xFC00) == 0x4400;
}

constexpr bool isPCRelativeLoadThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF800) == 0x4800;
}

constexpr bool isLoadStoreWithRegisterOffsetThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF200) == 0x5000;
}

constexpr bool isLoadStoreSignByteHalfwordThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF200) == 0x5200;
}

constexpr bool isLoadStoreImmediateOffsetThumb(uint16_t instruction_code) {
    return (instruction_code & 0xE000) == 0x6000;
}

constexpr bool isLoadStoreHalfwordThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0x8000;
}

constexpr bool isSPRelativeLoadStoreThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0x9000;
}

constexpr bool isLoadAddressThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0xA000;
}

constexpr bool isAddOffsetToStackPointerThumb(uint16_t instruction_code) {
    return (instruction_code & 0xFF00) == 0xB000;
}

constexpr bool isPushPopRegistersThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF600) == 0xB400;
}

constexpr bool isMultipleLoadStoreThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0xC000;
}

constexpr bool isSoftwareInterruptThumb(uint16_t instruction_code) {
    return (instruction_code & 0xFF00) == 0xDF00;
}

constexpr bool isConditionalBranchThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0xD000;
}

constexpr bool isUnconditionalBranchThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF800) == 0xE000;
}

constexpr bool isLongBranchLinkThumb(uint16_t instruction_code) {
    return (instruction_code & 0xF000) == 0xF000;
}

// 32-bit ARM instructions types

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

GBA::InstructionType GBA::CPU::decodeArm(uint32_t instruction_code) const {
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

void GBA::CPU::setMode(GBA::CPU::Mode mode) {
    CPSR &= 0xFFFFFFE0;
    switch (mode) {
    case Mode::User:
        CPSR |= 0x10;
        break;
    case Mode::FastInterrupt:
        CPSR |= 0x11;
        break;
    case Mode::Interrupt:
        CPSR |= 0x12;
        break;
    case Mode::Supervisor:
        CPSR |= 0x13;
        break;
    case Mode::Abort:
        CPSR |= 0x17;
        break;
    case Mode::Undefined:
        CPSR |= 0x1B;
        break;
    case Mode::System:
        CPSR |= 0x1F;
        break;
    default:
        throw;
    }
}

GBA::DataProcessingArguments
    GBA::CPU::decodeDataProcessingArguments(uint32_t instruction_code, Opcode opcode, uint32_t pc) {
    GBA::DataProcessingArguments arguments;
    if (opcode == GBA::Opcode::TEQ || opcode == GBA::Opcode::CMP || opcode == GBA::Opcode::CMN ||
        opcode == GBA::Opcode::TST) {
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
        arguments.shift_value =
            ((operand2 >> 8) & 0xF) * 2;  // "This value is zero extended to 32 bits, and then subject to a rotate right
                                          // by *twice* the value in the rotate field."
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
    arguments.PC = pc;
    return arguments;
}

void GBA::CPU::dataProcessingArmLogicalOperationFlagsSetting(
    bool S, uint32_t Rd, uint32_t operation_result,
    bool carry) {        // for example TST does not save its result in R(Rd) that's why we need it
    if (!S || Rd == 15)  // TODO setting carry flag
        return;
    if (operation_result & (1 << 31))  // setting negative flag
        CPSR |= (1 << 31);
    else
        CPSR &= ~(1 << 31);

    if (R(operation_result) == 0)  // setting zero flag
        CPSR |= (1 << 30);
    else
        CPSR &= ~(1 << 30);

    if (carry)  // setting carry flag
        CPSR |= (1 << 29);
    else
        CPSR &= ~(1 << 29);
}

void GBA::CPU::dataProcessingArmArithmeticOperationFlagsSetting(
    bool S, uint32_t Rd_before_operation, uint32_t Rd, uint32_t result, uint32_t operand1, uint32_t operand2,
    bool isAdd) {
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

    // setting carry flag
    if (isAdd) {
        if (operand1 > 0xFFFFFFFF - operand2)
            CPSR |= (1 << 29);
        else
            CPSR &= ~(1 << 29);
    }
    else {
        if (operand1 < operand2)
            CPSR |= (1 << 29);
        else
            CPSR &= ~(1 << 29);
    }

    // setting overflow flag
    if (((operand1 ^ operand2) & (1 << 31)) == 0 &&
        ((Rd_before_operation ^ result) & (1 << 31)) != 0)  // + + = - or - - = + then overlow flag is set
        CPSR |= (1 << 28);
    else
        CPSR &= ~(1 << 28);
}

std::pair<uint32_t, bool>
    GBA::CPU::calculateOperand2(uint32_t shifted_value, uint32_t shift_value, ShiftType shift_type) {
    if (shift_value == 0)
        return {shifted_value, (CPSR >> 28) & 1};  // return current carry flag value

    switch (shift_type) {
    case GBA::ShiftType::LogicalLeft: {
        bool carry_out = (shifted_value >> (32 - shift_value)) & 0x1;
        return {shifted_value << shift_value, carry_out};
    }
    case GBA::ShiftType::LogicalRight: {
        bool carry_out = (shifted_value >> (shift_value - 1)) & 0x1;
        return {shifted_value >> shift_value, carry_out};
    }
    case GBA::ShiftType::ArithmeticRight: {
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
    case GBA::ShiftType::RotateRight: {
        uint32_t carry_out = (shifted_value >> (shift_value - 1)) & 0x1;
        return {(shifted_value >> shift_value) | (shifted_value << (32 - shift_value)), carry_out};
    }
    default:
        throw;  // TODO: invalid shift type error handling
    }
}

void GBA::CPU::andArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: and\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) & operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::xorArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: xor\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) ^ operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::subArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: sub\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) - operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), R(arguments.Rn), operand2.first, false);
}

void GBA::CPU::rsbArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: rsb\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first - R(arguments.Rn);
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), operand2.first, R(arguments.Rd), false);
}

void GBA::CPU::addArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: add\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) + operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), R(arguments.Rn), operand2.first, true);
}

void GBA::CPU::adcArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: adc\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) + operand2.first + ((CPSR >> 29) & 0x1);
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), R(arguments.Rn), operand2.first, true);
}

void GBA::CPU::sbcArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: sbc\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) - operand2.first + ((CPSR >> 29) & 0x1) - 1;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), R(arguments.Rn), operand2.first, false);
}

void GBA::CPU::rscArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: rsc\n";
    uint32_t Rd_before_operation = R(arguments.Rd);
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first - R(arguments.Rn) + ((CPSR >> 29) & 0x1) - 1;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, Rd_before_operation, arguments.Rd, R(arguments.Rd), operand2.first, R(arguments.Rd), false);
}

void GBA::CPU::tstArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: tst\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) & operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result, operand2.second);
}

void GBA::CPU::teqArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: teq\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) ^ operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, result, operand2.second);
}

void GBA::CPU::cmpArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: cmp\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) - operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, R(arguments.Rn), arguments.Rd, result, R(arguments.Rn), operand2.first, false);
    // TODO check if arguments.Rd should be arguments.Rn
}

void GBA::CPU::cmnArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: cmn\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    uint32_t result = R(arguments.Rn) + operand2.first;
    dataProcessingArmArithmeticOperationFlagsSetting(
        arguments.S, R(arguments.Rn), arguments.Rd, result, R(arguments.Rn), operand2.first, true);
    // TODO check if arguments.Rd should be arguments.Rn
}

void GBA::CPU::orrArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: orr\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) | operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::movArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: mov\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::retArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: mov (ret)\n";
    PC() = LR(getMode());
    if (arguments.S) {
        switch (getMode()) {
        case Mode::FastInterrupt:
            CPSR = SPSR_FIQ;
            break;
        case Mode::Supervisor:
            CPSR = SPSR_SVC;
            break;
        case Mode::Abort:
            CPSR = SPSR_ABT;
            break;
        case Mode::Interrupt:
            CPSR = SPSR_IRQ;
            break;
        case Mode::User:
        case Mode::System:
            // todo todo
            break;
        case Mode::Undefined:
        default:
            throw;
        }
    }
}

void GBA::CPU::bicArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: bic\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = R(arguments.Rn) & ~operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

void GBA::CPU::mvnArm(DataProcessingArguments arguments) {
    std::cout << "# Instruction type: mvn\n";
    auto operand2 = calculateOperand2(arguments.shifted_value, arguments.shift_value, arguments.shift_type);
    R(arguments.Rd) = ~operand2.first;
    dataProcessingArmLogicalOperationFlagsSetting(arguments.S, arguments.Rd, R(arguments.Rd), operand2.second);
}

bool GBA::CPU::checkCondition(uint32_t instruction_code) const {
    std::cout << std::hex << std::setw(8) << std::setfill('0') << instruction_code << std::dec << '\n';
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

void GBA::CPU::callDataProcessingInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;  // If the condition is not met, do nothing, later check what should happen if anything

    auto opcode = getOpcodeArm(instruction_code);
    GBA::DataProcessingArguments arguments = decodeDataProcessingArguments(instruction_code, opcode, pc);
    // todo: fix this ugly code
    if (opcode == GBA::Opcode::MOV && ((instruction_code >> 12) & 0xF) == 0xF && (instruction_code & 0xF) == 0xE) {
        retArm(arguments);
    }
    else {
        (this->*data_processing_instruction_type[static_cast<int>(getOpcodeArm(instruction_code))])(arguments);
    }
}

GBA::MultiplyArguments GBA::CPU::decodeMultiplyArguments(uint32_t instruction_code, uint32_t pc) {
    GBA::MultiplyArguments arguments;
    arguments.A = (instruction_code >> 20) & 0x1;
    arguments.S = (instruction_code >> 19) & 0x1;
    arguments.Rd = (instruction_code >> 15) & 0xF;
    arguments.Rn = (instruction_code >> 11) & 0xF;
    arguments.Rs = (instruction_code >> 7) & 0xF;
    arguments.Rm = instruction_code & 0xF;
    arguments.PC = pc;
    return arguments;
}

void GBA::CPU::multiplyArmFlagSetting(bool S, uint32_t Rd) {
    if (S && Rd != 15) {
        if (R(Rd) & (1 << 31))
            CPSR |= (1 << 31);
        else
            CPSR &= ~(1 << 31);

        if (R(Rd) == 0)
            CPSR |= (1 << 30);
        else
            CPSR &= ~(1 << 30);

        CPSR &= ~(1 << 29);  // Clearing carry flag it instruction says it should be set to meaningless value
    }
}

void GBA::CPU::mulArm(MultiplyArguments arguments) {

    R(arguments.Rd) = R(arguments.Rm) * R(arguments.Rs);
    multiplyArmFlagSetting(arguments.S, arguments.Rd);
}

void GBA::CPU::mlaArm(MultiplyArguments arguments) {
    R(arguments.Rd) = R(arguments.Rm) * R(arguments.Rs) + R(arguments.Rn);
    multiplyArmFlagSetting(arguments.S, arguments.Rd);
}

void GBA::CPU::callMultiplyInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    // TODO: remove debug statements
    std::cout << "# Instruction type: Multiply\n";
    GBA::MultiplyArguments arguments = decodeMultiplyArguments(instruction_code, pc);
    if (arguments.A)
        mlaArm(arguments);
    else
        mulArm(arguments);
}

GBA::MultiplyLongArguments GBA::CPU::decodeMultiplyLongArguments(uint32_t instruction_code, uint32_t pc) {
    GBA::MultiplyLongArguments arguments;
    arguments.U = (instruction_code >> 22) & 0x1;
    arguments.A = (instruction_code >> 21) & 0x1;
    arguments.S = (instruction_code >> 20) & 0x1;
    arguments.RdHi = (instruction_code >> 16) & 0xF;
    arguments.RdLo = (instruction_code >> 12) & 0xF;
    arguments.Rm = (instruction_code >> 8) & 0xF;
    arguments.Rs = instruction_code & 0xF;
    arguments.PC = pc;
    return arguments;
}

void GBA::CPU::umullArm(MultiplyLongArguments arguments) {
    uint64_t result = R(arguments.Rm) * R(arguments.Rs);
    R(arguments.RdLo) = result & 0xFFFFFFFF;
    R(arguments.RdHi) = result >> 32;
}

void GBA::CPU::umlalArm(MultiplyLongArguments arguments) {
    uint64_t accumulator = (static_cast<uint64_t>(R(arguments.RdHi)) << 32) | R(arguments.RdLo);
    uint64_t result = R(arguments.Rm) * R(arguments.Rs) + accumulator;
    R(arguments.RdLo) = result & 0xFFFFFFFF;
    R(arguments.RdHi) = (result >> 32) & 0xFFFFFFFF;
}

void GBA::CPU::smullArm(MultiplyLongArguments arguments) {
    int64_t result = static_cast<int64_t>(arguments.Rm) * static_cast<int64_t>(R(arguments.Rs));
    int32_t temp = static_cast<int32_t>(result & 0xFFFFFFFF);
    R(arguments.RdLo) = *reinterpret_cast<uint32_t*>(&temp);
    temp = static_cast<int32_t>((result >> 32) & 0xFFFFFFFF);
    R(arguments.RdHi) = *reinterpret_cast<uint32_t*>(&temp);
}

void GBA::CPU::smlalArm(MultiplyLongArguments arguments) {
    int64_t accumulator = (static_cast<int64_t>(R(arguments.RdHi)) << 32) | R(arguments.RdLo);
    int64_t result = static_cast<int64_t>(R(arguments.Rm)) * static_cast<int64_t>(R(arguments.Rs)) + accumulator;
    int32_t temp = static_cast<int32_t>(result & 0xFFFFFFFF);
    R(arguments.RdLo) = *reinterpret_cast<uint32_t*>(&temp);
    temp = static_cast<int32_t>((result >> 32) & 0xFFFFFFFF);
    R(arguments.RdHi) = *reinterpret_cast<uint32_t*>(&temp);
}

void GBA::CPU::callMultiplyLongInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Multiply Long\n";
    GBA::MultiplyLongArguments arguments = decodeMultiplyLongArguments(instruction_code, pc);
    if (arguments.U) {
        if (arguments.A)
            umullArm(arguments);
        else
            smullArm(arguments);
    }
    else {
        if (arguments.A)
            umlalArm(arguments);
        else
            smlalArm(arguments);
    }

    if (arguments.S && arguments.RdHi != 15 && arguments.RdLo != 15) {
        if (R(arguments.RdHi) & (1 << 31))
            CPSR |= (1 << 31);
        else
            CPSR &= ~(1 << 31);

        if (R(arguments.RdHi) == 0 && R(arguments.RdLo) == 0)
            CPSR |= (1 << 30);
        else
            CPSR &= ~(1 << 30);

        CPSR &= ~(1 << 29);  // Clearing carry flag it instruction says it should be set to meaningless value
        CPSR &= ~(1 << 28);  // Clearing overflow flag it instruction says it should be set to meaningless value
    }
}

GBA::SingleDataTransferArguments GBA::CPU::decodeSingleDataTransferArguments(uint32_t instruction_code, uint32_t pc) {
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

void GBA::CPU::ldrArm(SingleDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);
    uint32_t offset = 0;
    if (arguments.I) {
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
    else {
        offset = arguments.offset;
    }

    if (arguments.P) {  // pre indexing
        if (arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    if (arguments.B)
        R(arguments.Rd) = memory.memory[address];
    else {
        R(arguments.Rd) = memory.memory[address];
        R(arguments.Rd) |= memory.memory[address + 1] << 8;
        R(arguments.Rd) |= memory.memory[address + 2] << 16;
        R(arguments.Rd) |= memory.memory[address + 3] << 24;
    }

    if (arguments.W == 1) {
        if (arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::strArm(SingleDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);
    uint32_t offset;

    if (arguments.I) {
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
    else {
        offset = arguments.offset;
    }

    if (arguments.P) {  // pre indexing
        if (arguments.U)
            address += offset;
        else
            address -= offset;
    }

    if (arguments.B)
        memory.memory[address] = R(arguments.Rd) & 0xFF;
    else {
        memory.memory[address] = R(arguments.Rd) & 0xFF;
        memory.memory[address + 1] = (R(arguments.Rd) >> 8) & 0xFF;
        memory.memory[address + 2] = (R(arguments.Rd) >> 16) & 0xFF;
        memory.memory[address + 3] = (R(arguments.Rd) >> 24) & 0xFF;
    }

    if (arguments.W == 1) {
        if (arguments.U)
            R(arguments.Rn) += offset;
        else
            R(arguments.Rn) -= offset;
    }
}

// LDR R0, [R1, #4]
// LDR R0, [R1, #4]!
// LDR R0, [R1], #4
void GBA::CPU::callSingleDataTransferInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Single Data Transfer\n";
    GBA::SingleDataTransferArguments arguments = decodeSingleDataTransferArguments(instruction_code, pc);
    if (arguments.L)
        ldrArm(arguments);
    else
        strArm(arguments);
}

GBA::HalfWordAndSignedDataTransferArguments
    GBA::CPU::decodeHalfWordAndSignedDataTransferArguments(uint32_t instruction_code, uint32_t pc) {
    HalfWordAndSignedDataTransferArguments arguments;
    arguments.P = (instruction_code >> 24) & 0x1;
    arguments.U = (instruction_code >> 23) & 0x1;
    arguments.W = (instruction_code >> 21) & 0x1;
    arguments.L = (instruction_code >> 20) & 0x1;
    arguments.Rn = (instruction_code >> 16) & 0xF;
    arguments.Rd = (instruction_code >> 12) & 0xF;
    arguments.S = (instruction_code >> 6) & 0x1;
    arguments.H = (instruction_code >> 5) & 0x1;
    arguments.PC = pc;
    bool bit_22 = (instruction_code >> 22) & 0x1;
    if (bit_22) {
        uint32_t bits_8_11 = (instruction_code >> 8) & 0xF;
        uint32_t bits_0_3 = instruction_code & 0xF;
        arguments.offset = (bits_8_11 << 4) | bits_0_3;
    }
    else
        arguments.offset = R(instruction_code & 0xF);
    return arguments;
}

void GBA::CPU::ldrhArm(HalfWordAndSignedDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);

    if (arguments.P) {
        if (arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    R(arguments.Rd) = memory.memory[address];
    R(arguments.Rd) |= memory.memory[address + 1] << 8;

    if (arguments.W) {
        if (arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::strhArm(HalfWordAndSignedDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);

    if (arguments.P) {
        if (arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    memory.memory[address] = R(arguments.Rd) & 0xFF;
    memory.memory[address + 1] = (R(arguments.Rd) >> 8) & 0xFF;

    if (arguments.W) {
        if (arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::ldrsbArm(HalfWordAndSignedDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);

    if (arguments.P) {
        if (arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    // load single byte and sign-extend it
    R(arguments.Rd) = memory.memory[address];
    if (R(arguments.Rd) & (1 << 7))
        R(arguments.Rd) |= 0xFFFFFF00;
    else
        R(arguments.Rd) &= 0x000000FF;

    if (arguments.W) {
        if (arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::ldrshArm(HalfWordAndSignedDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn);

    if (arguments.P) {
        if (arguments.U)
            address += arguments.offset;
        else
            address -= arguments.offset;
    }

    R(arguments.Rd) = memory.memory[address];
    R(arguments.Rd) |= memory.memory[address + 1] << 8;
    if (R(arguments.Rd) & (1 << 15))
        R(arguments.Rd) |= 0xFFFF0000;
    else
        R(arguments.Rd) &= 0x0000FFFF;

    if (arguments.W) {
        if (arguments.U)
            R(arguments.Rn) += arguments.offset;
        else
            R(arguments.Rn) -= arguments.offset;
    }
}

void GBA::CPU::callHalfWordAndSignedDataTransferInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Halfword Data Transfer\n";
    GBA::HalfWordAndSignedDataTransferArguments arguments =
        decodeHalfWordAndSignedDataTransferArguments(instruction_code, pc);
    if (arguments.L == 0b0)
        strhArm(arguments);
    else {
        uint32_t bits_S_H = arguments.S << 1 | arguments.H;
        if (bits_S_H == 0b01)
            ldrhArm(arguments);
        else if (bits_S_H == 0b10)
            ldrsbArm(arguments);
        else if (bits_S_H == 0b11)
            ldrshArm(arguments);
        else
            throw;  // 00 == SWP instruction like this should not exist
    }
}

void GBA::CPU::callSingleDataSwapInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Single Data Swap\n";
    swpArm(instruction_code);
}

void GBA::CPU::swpArm(uint32_t instruction_code) {
    uint32_t Rm = instruction_code & 0xF;
    uint32_t Rd = (instruction_code >> 12) & 0xF;
    uint32_t Rn = (instruction_code >> 16) & 0xF;
    uint32_t address = R(Rn);
    bool bit_22 = (instruction_code >> 22) & 0x1;
    uint32_t Rm_value = R(Rm);
    if (bit_22) {
        R(Rd) = memory.memory[address];
    }
    else {
        R(Rd) = memory.memory[address];
        R(Rd) |= memory.memory[address + 1] << 8;
        R(Rd) |= memory.memory[address + 2] << 16;
        R(Rd) |= memory.memory[address + 3] << 24;
    }

    if (bit_22) {
        memory.memory[address] = Rm_value & 0xFF;
    }
    else {
        memory.memory[address] = Rm_value & 0xFF;
        memory.memory[address + 1] = (Rm_value >> 8) & 0xFF;
        memory.memory[address + 2] = (Rm_value >> 16) & 0xFF;
        memory.memory[address + 3] = (Rm_value >> 24) & 0xFF;
    }
}

void GBA::CPU::callBlockDataTransferInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Block Data Transfer\n";
    BlockDataTransferArguments arguments = decodeBlockDataTransferInstruction(instruction_code, pc);
    if (arguments.L == 0b1) {
        ldmArm(arguments);
    }
    else {
        stmArm(arguments);
    }
}

GBA::BlockDataTransferArguments GBA::CPU::decodeBlockDataTransferInstruction(uint32_t instruction_code, uint32_t pc) {
    BlockDataTransferArguments arguments;
    arguments.P = (instruction_code >> 24) & 0x1;     // Pre/Post indexing bit
    arguments.U = (instruction_code >> 23) & 0x1;     // Up/Down bit
    arguments.S = (instruction_code >> 22) & 0x1;     // program status register & force user bit
    arguments.W = (instruction_code >> 21) & 0x1;     // write-back bit
    arguments.L = (instruction_code >> 20) & 0x1;     // Load/Store bit
    arguments.Rn = (instruction_code >> 16) & 0xF;    // Base register
    arguments.registers = instruction_code & 0xFFFF;  // Register list bitfield
    return arguments;
}

void GBA::CPU::ldmArm(GBA::BlockDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn) & 0xFFFFFFE0;  // address must be word-aligned
    for (uint32_t i = 0; i < 15; i++) {
        if ((arguments.registers & (0b1 << i)) != 0) {
            if (arguments.S == 0b0) {
                R(i) = memory.memory[address];
                R(i) |= memory.memory[address + 1] << 8;
                R(i) |= memory.memory[address + 2] << 16;
                R(i) |= memory.memory[address + 3] << 24;
            }
            else {
                R_USRSYS(i) = memory.memory[address];
                R_USRSYS(i) |= memory.memory[address + 1] << 8;
                R_USRSYS(i) |= memory.memory[address + 2] << 16;
                R_USRSYS(i) |= memory.memory[address + 3] << 24;
            }
            if (arguments.U == 0b1) {
                address += 4;
            }
            else {
                address -= 4;
            }
        }
    }
    if ((arguments.registers & (0b1 << 15)) != 0) {
        PC() = memory.memory[address];
        PC() |= memory.memory[address + 1] << 8;
        PC() |= memory.memory[address + 2] << 16;
        PC() |= memory.memory[address + 3] << 24;
        if (arguments.S == 0b1) {  // mode change
            switch (getMode()) {
            case Mode::User:
            case Mode::System:
                break;
            case Mode::FastInterrupt:
                CPSR = SPSR_FIQ;
                break;
            case Mode::Supervisor:
                CPSR = SPSR_SVC;
                break;
            case Mode::Abort:
                CPSR = SPSR_ABT;
                break;
            case Mode::Interrupt:
                CPSR = SPSR_IRQ;
                break;
            case Mode::Undefined:
                CPSR = SPSR_UND;
                break;
            default:
                throw;
            }
        }
    }
}

void GBA::CPU::stmArm(GBA::BlockDataTransferArguments arguments) {
    uint32_t address = R(arguments.Rn) & 0xFFFFFFE0;  // check this
    for (uint32_t i = 0; i < 16; i++) {
        if ((arguments.registers & (0b1 << i)) != 0) {
            if (arguments.S == 0b0) {
                memory.memory[address] = R(i) & 0xFF;
                memory.memory[address + 1] = (R(i) >> 8) & 0xFF;
                memory.memory[address + 2] = (R(i) >> 16) & 0xFF;
                memory.memory[address + 3] = (R(i) >> 24) & 0xFF;
            }
            else {  // User bank transfer
                memory.memory[address] = R_USRSYS(i) & 0xFF;
                memory.memory[address + 1] = (R_USRSYS(i) >> 8) & 0xFF;
                memory.memory[address + 2] = (R_USRSYS(i) >> 16) & 0xFF;
                memory.memory[address + 3] = (R_USRSYS(i) >> 24) & 0xFF;
            }
            if (arguments.U == 0b1) {
                address += 4;
            }
            else {
                address -= 4;
            }
        }
    }
}

void GBA::CPU::callSoftwareInterruptInstruction(uint32_t instruction_code, uint32_t pc) {
    std::cout << "# Instruction type: Software Interrupt\n";
    LR(Mode::Supervisor) = pc + 4;
    PC() = 0x08;
    SPSR_SVC = CPSR;
    setMode(Mode::Supervisor);
}

void GBA::CPU::bxArm(uint32_t instruction_code) {
    uint32_t Rn = R(instruction_code & 0xF);
    PC() = Rn;
    // Check: Arm or Thumb
    if (Rn & 0x1)
        CPSR |= (1 << 5);
    else
        CPSR &= ~(1 << 5);
}

void GBA::CPU::callBranchAndExchangeInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Branch Exchange\n";
    bxArm(instruction_code);
}

void GBA::CPU::blArm(uint32_t instruction_code, uint32_t pc) {
    uint32_t offset = instruction_code & 0xFFFFFF;
    if (offset & 0x800000)  // sign extend to 32 bits
        offset |= 0xFF000000;

    R(14) = pc + 4;
    PC() = (offset << 2) + pc + 8;  // acount for prefetch
}

void GBA::CPU::bArm(uint32_t instruction_code, uint32_t pc) {
    int32_t offset = instruction_code & 0xFFFFFF;
    if (offset & 0x800000)  // sign extend to 32 bits
        offset |= 0xFF000000;

    PC() = (offset << 2) + pc + 8;  // account for prefetch
}

void GBA::CPU::callBranchInstruction(uint32_t instruction_code, uint32_t pc) {
    if (!checkCondition(instruction_code))
        return;

    std::cout << "# Instruction type: Branch\n";
    if ((instruction_code >> 24) & 0b1) {
        blArm(instruction_code, pc);
    }
    else {
        bArm(instruction_code, pc);
    }
}

GBA::ThumbInstructionType GBA::CPU::decodeThumb(uint16_t instruction_code) {
    if (isAddSubstractThumb(instruction_code))
        return GBA::ThumbInstructionType::AddSubtract;
    if (isMoveShiftedRegisterThumb(instruction_code))
        return GBA::ThumbInstructionType::MoveShiftedRegister;
    if (isMoveCompareAddSubtractImmediateThumb(instruction_code))
        return GBA::ThumbInstructionType::MoveCompareAddSubtractImmediate;
    if (isAluOperationThumb(instruction_code))
        return GBA::ThumbInstructionType::ALUOperation;
    if (isHiRegisterOperationsBranchExchangeThumb(instruction_code))
        return GBA::ThumbInstructionType::HighRegisterOperationBranchExchange;
    if (isPCRelativeLoadThumb(instruction_code))
        return GBA::ThumbInstructionType::PCRelativeLoad;
    if (isLoadStoreWithRegisterOffsetThumb(instruction_code))
        return GBA::ThumbInstructionType::LoadStoreRegOffset;
    if (isLoadStoreSignByteHalfwordThumb(instruction_code))
        return GBA::ThumbInstructionType::LoadStoreSignByteHalfword;
    if (isLoadStoreImmediateOffsetThumb(instruction_code))
        return GBA::ThumbInstructionType::LoadStoreImmediateOffset;
    if (isLoadStoreHalfwordThumb(instruction_code))
        return GBA::ThumbInstructionType::LoadStoreHalfword;
    if (isSPRelativeLoadStoreThumb(instruction_code))
        return GBA::ThumbInstructionType::SPRelativeLoadStore;
    if (isLoadAddressThumb(instruction_code))
        return GBA::ThumbInstructionType::LoadAddress;
    if (isAddOffsetToStackPointerThumb(instruction_code))
        return GBA::ThumbInstructionType::AddOffsetToStackPointer;
    if (isPushPopRegistersThumb(instruction_code))
        return GBA::ThumbInstructionType::PushPopRegisters;
    if (isMultipleLoadStoreThumb(instruction_code))
        return GBA::ThumbInstructionType::MultipleLoadStore;
    if (isSoftwareInterruptThumb(instruction_code))
        return GBA::ThumbInstructionType::SoftwareInterrupt;
    if (isConditionalBranchThumb(instruction_code))
        return GBA::ThumbInstructionType::ConditionalBranch;
    if (isUnconditionalBranchThumb(instruction_code))
        return GBA::ThumbInstructionType::UnconditionalBranch;
    if (isLongBranchLinkThumb(instruction_code))
        return GBA::ThumbInstructionType::LongBranchLink;

    return GBA::ThumbInstructionType::Undefined;
}

GBA::MoveShifterRegisterThumbArguments GBA::CPU::decodeMoveShiftedRegisterThumbArguments(uint16_t instruction_code) {
    GBA::MoveShifterRegisterThumbArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rs = (instruction_code >> 3) & 0x7;
    arguments.shift_type = static_cast<GBA::ShiftType>((instruction_code >> 11) & 0x3);
    arguments.shift_value = (instruction_code >> 6) & 0x1F;
    return arguments;
}

void GBA::CPU::lslThumb(MoveShifterRegisterThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rs), arguments.shift_value, GBA::ShiftType::LogicalLeft, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::lsrThumb(MoveShifterRegisterThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rs), arguments.shift_value, GBA::ShiftType::LogicalRight, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::asrThumb(MoveShifterRegisterThumbArguments arguments) {

    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rs), arguments.shift_value, GBA::ShiftType::ArithmeticRight, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::callMoveShiftedRegisterThumbInstruction(uint16_t instruction_code) {
    MoveShifterRegisterThumbArguments arguments = decodeMoveShiftedRegisterThumbArguments(instruction_code);
    if (arguments.shift_type == GBA::ShiftType::LogicalLeft)
        lslThumb(arguments);
    else if (arguments.shift_type == GBA::ShiftType::LogicalRight)
        lsrThumb(arguments);
    else if (arguments.shift_type == GBA::ShiftType::ArithmeticRight)
        asrThumb(arguments);
    else
        return;  // TODO: invalid shift type error handling
}

GBA::AddSubtractThumbArguments GBA::CPU::decodeAddSubtractThumbArguments(uint16_t instruction_code) {
    GBA::AddSubtractThumbArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rs = (instruction_code >> 3) & 0x7;
    arguments.operand2 = (instruction_code >> 6) & 0x7;
    arguments.I = (instruction_code >> 10) & 0x1;
    arguments.op = (instruction_code >> 9) & 0x1;
    return arguments;
}

void GBA::CPU::addThumb(AddSubtractThumbArguments arguments) {
    if (arguments.I) {
        DataProcessingArguments data_processing_arguments(
            1, arguments.Rd, arguments.Rs, arguments.operand2, 0, GBA::ShiftType::LogicalLeft, PC());
        addArm(data_processing_arguments);
    }
    else {
        DataProcessingArguments data_processing_arguments(
            1, arguments.Rd, arguments.Rs, R(arguments.operand2), 0, GBA::ShiftType::LogicalLeft, PC());
        addArm(data_processing_arguments);
    }
}

void GBA::CPU::subThumb(AddSubtractThumbArguments arguments) {
    if (arguments.I) {
        DataProcessingArguments data_processing_arguments(
            1, arguments.Rd, arguments.Rs, arguments.operand2, 0, GBA::ShiftType::LogicalLeft, PC());
        subArm(data_processing_arguments);
    }
    else {
        DataProcessingArguments data_processing_arguments(
            1, arguments.Rd, arguments.Rs, R(arguments.operand2), 0, GBA::ShiftType::LogicalLeft, PC());
        subArm(data_processing_arguments);
    }
}

void GBA::CPU::callAddSubtractThumbInstruction(uint16_t instruction_code) {
    AddSubtractThumbArguments arguments = decodeAddSubtractThumbArguments(instruction_code);
    if (arguments.op)
        addThumb(arguments);
    else
        subThumb(arguments);
}

GBA::MoveCompareAddSubtractImmediateThumbArguments
    GBA::CPU::decodeMoveCompareAddSubtractImmediateThumbArguments(uint16_t instruction_code) {
    GBA::MoveCompareAddSubtractImmediateThumbArguments arguments;
    arguments.Rd = (instruction_code >> 8) & 0x7;
    arguments.op = (instruction_code >> 11) & 0x3;
    arguments.offset = instruction_code & 0xFF;
    return arguments;
}

void GBA::CPU::movThumb(MoveCompareAddSubtractImmediateThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, arguments.offset, 0, GBA::ShiftType::LogicalLeft, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::cmpThumb(MoveCompareAddSubtractImmediateThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, 0, arguments.Rd, arguments.offset, 0, GBA::ShiftType::LogicalLeft, PC());
    cmpArm(data_processing_arguments);
}

void GBA::CPU::addThumb(MoveCompareAddSubtractImmediateThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, arguments.offset, 0, GBA::ShiftType::LogicalLeft, PC());
    addArm(data_processing_arguments);
}

void GBA::CPU::subThumb(MoveCompareAddSubtractImmediateThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, arguments.offset, 0, GBA::ShiftType::LogicalLeft, PC());
    subArm(data_processing_arguments);
}

void GBA::CPU::callMoveCompareAddSubtractImmediateThumbInstruction(uint16_t instruction_code) {
    MoveCompareAddSubtractImmediateThumbArguments arguments =
        decodeMoveCompareAddSubtractImmediateThumbArguments(instruction_code);
    if (arguments.op == 0b00)
        movThumb(arguments);
    else if (arguments.op == 0b01)
        cmpThumb(arguments);
    else if (arguments.op == 0b10)
        addThumb(arguments);
    else if (arguments.op == 0b11)
        subThumb(arguments);
    else
        return;  // TODO: invalid op error handling
}

GBA::ALUoperationThumbArguments GBA::CPU::decodeALUOperationThumbArguments(uint16_t instruction_code) {
    ALUoperationThumbArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rs = (instruction_code >> 3) & 0x7;
    arguments.op = (instruction_code >> 6) & 0xF;
    return arguments;
}

void GBA::CPU::andThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    andArm(data_processing_arguments);
}

void GBA::CPU::xorThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    xorArm(data_processing_arguments);
}

void GBA::CPU::lslThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rd), R(arguments.Rs), GBA::ShiftType::LogicalLeft, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::lsrThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rd), R(arguments.Rs), GBA::ShiftType::LogicalRight, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::asrThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rd), R(arguments.Rs), GBA::ShiftType::ArithmeticRight, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::adcThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    adcArm(data_processing_arguments);
}

void GBA::CPU::sbcThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    sbcArm(data_processing_arguments);
}

void GBA::CPU::rorThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rd), R(arguments.Rs), GBA::ShiftType::RotateRight, PC());
    movArm(data_processing_arguments);
}

void GBA::CPU::tstThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, 0, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    tstArm(data_processing_arguments);
}

void GBA::CPU::negThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rs, 0, 0, GBA::ShiftType::LogicalLeft, PC());
    rsbArm(data_processing_arguments);
}

void GBA::CPU::cmpThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, 0, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    cmpArm(data_processing_arguments);
}

void GBA::CPU::cmnThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, 0, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    cmnArm(data_processing_arguments);
}

void GBA::CPU::orrThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    orrArm(data_processing_arguments);
}

void GBA::CPU::mulThumb(ALUoperationThumbArguments arguments) {
    MultiplyArguments data_processing_arguments(1, 0, arguments.Rd, 0, arguments.Rs, arguments.Rd, PC());
    mulArm(data_processing_arguments);
}

void GBA::CPU::bicThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, arguments.Rd, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    bicArm(data_processing_arguments);
}

void GBA::CPU::mvnThumb(ALUoperationThumbArguments arguments) {
    DataProcessingArguments data_processing_arguments(
        1, arguments.Rd, 0, R(arguments.Rs), 0, GBA::ShiftType::LogicalLeft, PC());
    mvnArm(data_processing_arguments);
}

void GBA::CPU::callALUOperationThumbInstruction(uint16_t instruction_code) {
    ALUoperationThumbArguments arguments = decodeALUOperationThumbArguments(instruction_code);
    if (arguments.op == 0b0000)
        andThumb(arguments);
    else if (arguments.op == 0b0001)
        xorThumb(arguments);
    else if (arguments.op == 0b0010)
        lslThumb(arguments);
    else if (arguments.op == 0b0011)
        lsrThumb(arguments);
    else if (arguments.op == 0b0100)
        asrThumb(arguments);
    else if (arguments.op == 0b0101)
        adcThumb(arguments);
    else if (arguments.op == 0b0110)
        sbcThumb(arguments);
    else if (arguments.op == 0b0111)
        rorThumb(arguments);
    else if (arguments.op == 0b1000)
        tstThumb(arguments);
    else if (arguments.op == 0b1001)
        negThumb(arguments);
    else if (arguments.op == 0b1010)
        cmpThumb(arguments);
    else if (arguments.op == 0b1011)
        cmnThumb(arguments);
    else if (arguments.op == 0b1100)
        orrThumb(arguments);
    else if (arguments.op == 0b1101)
        mulThumb(arguments);
    else if (arguments.op == 0b1110)
        bicThumb(arguments);
    else if (arguments.op == 0b1111)
        mvnThumb(arguments);
    else
        return;  // TODO: invalid op error handling
}

GBA::HiRegisterOperationsBranchExchangeArguments
    GBA::CPU::decodeHiRegisterOperationBranchExchangeArguments(uint16_t instruction_code) {
    HiRegisterOperationsBranchExchangeArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rs = (instruction_code >> 3) & 0x7;
    arguments.H1 = (instruction_code >> 6) & 0x1;
    arguments.H2 = (instruction_code >> 7) & 0x1;
    arguments.op = (instruction_code >> 8) & 0x3;
    return arguments;
}

void GBA::CPU::addHiRegisterOperationBranchExchange(HiRegisterOperationsBranchExchangeArguments arguments) {
    uint32_t source_number, destination_number;
    if (arguments.H1)
        source_number = 8 + arguments.Rs;
    else
        source_number = arguments.Rs;

    if (arguments.H2)
        destination_number = 8 + arguments.Rd;
    else
        destination_number = arguments.Rd;

    R(destination_number) += R(source_number);
}

void GBA::CPU::cmpHiRegisterOperationBranchExchange(HiRegisterOperationsBranchExchangeArguments arguments) {
    uint32_t source_number, destination_number;
    if (arguments.H1)
        source_number = 8 + arguments.Rs;
    else
        source_number = arguments.Rs;

    if (arguments.H2)
        destination_number = 8 + arguments.Rd;
    else
        destination_number = arguments.Rd;

    DataProcessingArguments data_processing_arguments(
        1, 0, destination_number, R(source_number), 0, GBA::ShiftType::LogicalLeft, PC());
}

void GBA::CPU::movHiRegisterOperationBranchExchange(HiRegisterOperationsBranchExchangeArguments arguments) {
    uint32_t source_number, destination_number;
    if (arguments.H1)
        source_number = 8 + arguments.Rs;
    else
        source_number = arguments.Rs;

    if (arguments.H2)
        destination_number = 8 + arguments.Rd;
    else
        destination_number = arguments.Rd;

    R(destination_number) = R(source_number);
}

void GBA::CPU::bxHiRegisterOperationBranchExchange(HiRegisterOperationsBranchExchangeArguments arguments) {
    uint32_t source_number;
    if (arguments.H1)
        source_number = 8 + arguments.Rs;
    else
        source_number = arguments.Rs;

    bxArm(R(source_number));
}

void GBA::CPU::callHiRegisterOperationBranchExchangeInstruction(uint16_t instruction_code) {
    HiRegisterOperationsBranchExchangeArguments arguments =
        decodeHiRegisterOperationBranchExchangeArguments(instruction_code);
    if (arguments.op == 0b00)
        addHiRegisterOperationBranchExchange(arguments);
    else if (arguments.op == 0b01)
        cmpHiRegisterOperationBranchExchange(arguments);
    else if (arguments.op == 0b10)
        movHiRegisterOperationBranchExchange(arguments);
    else if (arguments.op == 0b11)
        bxHiRegisterOperationBranchExchange(arguments);
    else
        return;  // TODO: invalid op error handling
}

GBA::LoadStoreRegOffsetArguments GBA::CPU::decodeLoadStoreRegOffsetArguments(uint16_t instruction_code) {
    LoadStoreRegOffsetArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rb = (instruction_code >> 3) & 0x7;
    arguments.Ro = (instruction_code >> 6) & 0x7;
    arguments.B = (instruction_code >> 10) & 0x1;
    arguments.L = (instruction_code >> 11) & 0x1;
    return arguments;
}

void GBA::CPU::callLoadStoreRegOffset(uint16_t instruction_code) {
    LoadStoreRegOffsetArguments arguments = decodeLoadStoreRegOffsetArguments(instruction_code);
    uint32_t address = R(arguments.Rb) + R(arguments.Ro);
    if (arguments.L)
        ldrThumb(address, arguments.Rd, arguments.B);
    else
        strThumb(address, arguments.Rd, arguments.B);
}

GBA::LoadStoreImmediateOffsetArguments GBA::CPU::decodeLoadStoreImmediateOffsetArguments(uint16_t instruction_code) {
    LoadStoreImmediateOffsetArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rb = (instruction_code >> 3) & 0x7;
    arguments.offset =
        (instruction_code >> 6) & 0x1F;  // not sure because these instructions transfer byte or word values between
                                         // registers and memory using an immediate 5 or 7-bit offset.
    arguments.L = (instruction_code >> 11) & 0x1;
    arguments.B = (instruction_code >> 12) & 0x1;
    return arguments;
}

void GBA::CPU::callLoadStoreImmediateOffset(uint16_t instruction_code) {
    LoadStoreImmediateOffsetArguments arguments = decodeLoadStoreImmediateOffsetArguments(instruction_code);
    uint32_t address = R(arguments.Rb) + arguments.offset;
    if (arguments.L)
        ldrThumb(address, arguments.Rd, arguments.B);
    else
        strThumb(address, arguments.Rd, arguments.B);
}

void GBA::CPU::ldrThumb(uint32_t address, uint32_t Rd, bool B) {
    if (B) {
        R(Rd) = memory.memory[address];
    }
    else {
        R(Rd) = memory.memory[address];
        R(Rd) |= memory.memory[address + 1] << 8;
        R(Rd) |= memory.memory[address + 2] << 16;
        R(Rd) |= memory.memory[address + 3] << 24;
    }
}

void GBA::CPU::strThumb(uint32_t address, uint32_t Rd, bool B) {
    if (B) {
        memory.memory[address] = R(Rd) & 0xFF;
    }
    else {
        memory.memory[address] = R(Rd) & 0xFF;
        memory.memory[address + 1] = (R(Rd) >> 8) & 0xFF;
        memory.memory[address + 2] = (R(Rd) >> 16) & 0xFF;
        memory.memory[address + 3] = (R(Rd) >> 24) & 0xFF;
    }
}

GBA::LoadStoreSignExtendedByteHalfwordArguments
    GBA::CPU::decodeLoadStoreSignExtendedByteHalfwordArguments(uint16_t instruction_code) {
    LoadStoreSignExtendedByteHalfwordArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rb = (instruction_code >> 3) & 0x7;
    arguments.Ro = (instruction_code >> 6) & 0x7;
    arguments.S = (instruction_code >> 10) & 0x1;
    arguments.H = (instruction_code >> 11) & 0x1;
    return arguments;
}

void GBA::CPU::callLoadStoreSignExtendedByteHalfword(uint16_t instruction_code) {
    LoadStoreSignExtendedByteHalfwordArguments arguments =
        decodeLoadStoreSignExtendedByteHalfwordArguments(instruction_code);
    uint32_t address = R(arguments.Rb) + R(arguments.Ro);
    uint32_t S_H = arguments.S << 1 | arguments.H;
    if (S_H == 0b00) {
        HalfWordAndSignedDataTransferArguments halfword_arguments(
            0, 0, 0, 0, arguments.S, arguments.H, arguments.Rb, arguments.Rd, arguments.Ro, PC());
        strhArm(halfword_arguments);
    }
    else if (S_H == 0b01) {
        HalfWordAndSignedDataTransferArguments halfword_arguments(
            0, 0, 0, 1, arguments.S, arguments.H, arguments.Rb, arguments.Rd, arguments.Ro, PC());
        ldrhArm(halfword_arguments);
    }
    else if (S_H == 0b10) {
        HalfWordAndSignedDataTransferArguments halfword_arguments(
            0, 0, 0, 1, arguments.S, arguments.H, arguments.Rb, arguments.Rd, arguments.Ro, PC());
        ldrsbArm(halfword_arguments);
    }
    else if (S_H == 0b11) {
        HalfWordAndSignedDataTransferArguments halfword_arguments(
            0, 0, 0, 1, arguments.S, arguments.H, arguments.Rb, arguments.Rd, arguments.Ro, PC());
        ldrshArm(halfword_arguments);
    }
    else
        return;  // TODO: invalid S_H error handling
}

GBA::LoadStoreHalfwordArguments GBA::CPU::decodeLoadStoreHalfwordArguments(uint16_t instruction_code) {
    LoadStoreHalfwordArguments arguments;
    arguments.Rd = instruction_code & 0x7;
    arguments.Rb = (instruction_code >> 3) & 0x7;
    arguments.offset =
        (instruction_code >> 6) &
        0x1F << 1;  //#Imm is a full 6-bit address but must be halfword-aligned (ie with bit 0 set to 0) since
                    // the assembler places #Imm >> 1 in the Offset5
    arguments.L = (instruction_code >> 11) & 0x1;
    return arguments;
}

void GBA::CPU::callLoadStoreHalfword(uint16_t instruction_code) {
    LoadStoreHalfwordArguments arguments = decodeLoadStoreHalfwordArguments(instruction_code);
    uint32_t address = R(arguments.Rb) + arguments.offset;
    if (arguments.L) {
        R(arguments.Rd) = memory.memory[address];
        R(arguments.Rd) |= memory.memory[address + 1] << 8;
    }
    else {
        memory.memory[address] = R(arguments.Rd) & 0xFF;
        memory.memory[address + 1] = (R(arguments.Rd) >> 8) & 0xFF;
    }
}

GBA::SPRelativeLoadStoreArguments GBA::CPU::decodeSPRelativeLoadStoreArguments(uint16_t instruction_code) {
    SPRelativeLoadStoreArguments arguments;
    arguments.Rd = (instruction_code >> 8) & 0x7;
    arguments.offset = (instruction_code & 0xFF) << 2;
    arguments.L = (instruction_code >> 11) & 0x1;
    return arguments;
}

void GBA::CPU::callSPRelativeLoadStore(uint16_t instruction_code) {
    SPRelativeLoadStoreArguments arguments = decodeSPRelativeLoadStoreArguments(instruction_code);
    uint32_t address = SP(getMode()) + arguments.offset;  // TODO: check if SP is correct register in thumb mode
    if (arguments.L) {
        ldrThumb(address, arguments.Rd, 0);
    }
    else {
        strThumb(address, arguments.Rd, 0);
    }
}

GBA::LoadAddressArguments GBA::CPU::decodeLoadAddressArguments(uint16_t instruction_code) {
    LoadAddressArguments arguments;
    arguments.Rd = (instruction_code >> 8) & 0x7;
    arguments.offset = (instruction_code & 0xFF) << 2;
    arguments.SP = (instruction_code >> 11) & 0x1;
    return arguments;
}

void GBA::CPU::callLoadAddress(uint16_t instruction_code) {
    LoadAddressArguments arguments = decodeLoadAddressArguments(instruction_code);
    if (arguments.SP) {
        R(arguments.Rd) = SP(getMode()) + arguments.offset;  // TODO: check if SP is correct register in thumb mode
    }
    else {
        R(arguments.Rd) = PC() & 0xFFFFFFFD;  // it 1 of the PC is always read as 0.
        R(arguments.Rd) += arguments.offset;
    }
}

void GBA::CPU::callAddOffsetToStackPointer(uint16_t instruction_code) {
    uint32_t offset = (instruction_code & 0xFF);
    bool S = (instruction_code >> 7) & 0x1;
    int32_t signed_offset = (S << 7) | offset;

    if (S)
        signed_offset |= ~0xFF;  // extend sign bit

    SP(getMode()) += signed_offset << 2;
}

GBA::PushPopRegistersArguments GBA::CPU::decodePushPopRegistersArguments(uint16_t instruction_code) {
    PushPopRegistersArguments arguments;
    arguments.R = (instruction_code >> 8) & 0x1;
    arguments.L = (instruction_code >> 11) & 0x1;
    arguments.Rlist = instruction_code & 0xFF;
    return arguments;
}
void GBA::CPU::callPushPopRegisters(uint16_t instruction_code) {
    PushPopRegistersArguments arguments = decodePushPopRegistersArguments(instruction_code);
    uint32_t register_list = arguments.Rlist;
    uint32_t L_R = arguments.L << 1 | arguments.R;
    if (L_R == 0b01) {
        register_list |= 0x4000;  // set LR
    }
    else if (L_R == 0b11) {
        register_list |= 0x8000;  // set PC
    }

    if (arguments.L) {
        BlockDataTransferArguments block_arguments(0, 0, 0, 1, 1, 13, register_list, PC());
        ldmArm(block_arguments);
    }
    else {
        BlockDataTransferArguments block_arguments(0, 0, 0, 1, 0, 13, register_list, PC());
        stmArm(block_arguments);
    }
}

void GBA::CPU::callMultipleLoadStore(uint16_t instruction_code) {
    bool L = (instruction_code >> 11) & 0x1;
    uint32_t Rlist = instruction_code & 0xFF;
    uint32_t Rb = (instruction_code >> 8) & 0x7;
    if (L) {
        BlockDataTransferArguments block_arguments(0, 0, 0, 1, 1, Rb, Rlist, PC());
        ldmArm(block_arguments);
    }
    else {
        BlockDataTransferArguments block_arguments(0, 0, 0, 1, 0, Rb, Rlist, PC());
        stmArm(block_arguments);
    }
}

void GBA::CPU::callConditionalBranch(uint16_t instruction_code) {
    uint32_t offset = instruction_code & 0xFF;
    uint32_t condition = (instruction_code >> 8) & 0xF;
    if (checkCondition(condition)) {
        PC() += offset << 1;
        PC() -= 4;  // TODO check if correct I think it is because PC is 4 bytes ahead due to prefetch
    }
}

void GBA::CPU::callUnconitionalBranch(uint16_t instruction_code) {
    uint32_t offset = instruction_code & 0x7FF;
    PC() += offset << 1;
    PC() -= 4;  // TODO check if correct I think it is because PC is 4 bytes ahead due to prefetch
}

void GBA::CPU::callLongBranchLink(uint16_t instruction_code) {
    uint32_t offset = instruction_code & 0x7FF;
    uint32_t H = (instruction_code >> 11) & 0x1;
    if (H) {
        LR(getMode()) = offset << 1;
        uint32_t current_PC = PC();
        PC() = LR(getMode());
        LR(getMode()) = current_PC;  // there should be an address of the next instruction after the BL instruction
        LR(getMode()) |=
            0x00000001;  // he address of the instruction following the BL is placed in LR and bit 0 of LR is set.
    }
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
    switch (getMode()) {
    case Mode::User:
    case Mode::System:
        return R_USRSYS(index);
    case Mode::FastInterrupt:
        return R_FIQ(index);
    case Mode::Supervisor:
        return R_SVC(index);
    case Mode::Abort:
        return R_ABT(index);
    case Mode::Interrupt:
        return R_IRQ(index);
    case Mode::Undefined:
        return R_UND(index);
    default:
        throw;
    }
}

const uint32_t& GBA::CPU::R(uint32_t index) const {
    switch (getMode()) {
    case Mode::User:
    case Mode::System:
        return R_USRSYS(index);
    case Mode::FastInterrupt:
        return R_FIQ(index);
    case Mode::Supervisor:
        return R_SVC(index);
    case Mode::Abort:
        return R_ABT(index);
    case Mode::Interrupt:
        return R_IRQ(index);
    case Mode::Undefined:
        return R_UND(index);
    default:
        throw;
    }
}

uint32_t& GBA::CPU::R_USRSYS(uint32_t index) {
    // TODO: error handling
    if (index > 15) {
        throw;
    }
    return registers[index];
}

const uint32_t& GBA::CPU::R_USRSYS(uint32_t index) const {
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
