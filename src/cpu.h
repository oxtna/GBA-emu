#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "common.h"
#include "instruction_types.h"
#include "instruction_types_arguments.h"
#include "opcode.h"
#include <array>
#include "memory.h"
#include <iostream>
#include <utility>


namespace GBA {

class CPU
{
  public:
    enum class Mode {
        User,
        System,
        FastInterrupt,
        Supervisor,
        Abort,
        Interrupt,
        Undefined,
    };

    CPU();
    ~CPU() = default;

    InstructionType decodeArm(uint32_t instruction_code);
    // TODO: https://developer.arm.com/documentation/ddi0210/c/Programmer-s-Model/Reset
    void reset();

    Mode getMode() const;
    // Check if the instruction should be executed based on the condition field
    bool checkCondition(uint32_t intruction_code) const;

    DataProcessingArguments decodeDataProcessingArguments(uint32_t instruction_code, Opcode opcode);
    void dataProcessingArmLogicalOperationFlagsSetting(bool S, uint32_t Rd, uint32_t operation_result, bool carry);
    void dataProcessingArmArithmeticOperationFlagsSetting(bool S, uint32_t Rd_before_operation, uint32_t Rd, uint32_t result);
    void andArm(DataProcessingArguments arguments);
    void xorArm(DataProcessingArguments arguments);
    void subArm(DataProcessingArguments arguments);
    void rsbArm(DataProcessingArguments arguments);
    void addArm(DataProcessingArguments arguments);
    void adcArm(DataProcessingArguments arguments);
    void sbcArm(DataProcessingArguments arguments);
    void rscArm(DataProcessingArguments arguments);
    void tstArm(DataProcessingArguments arguments);
    void teqArm(DataProcessingArguments arguments);
    void cmpArm(DataProcessingArguments arguments);
    void cmnArm(DataProcessingArguments arguments);
    void orrArm(DataProcessingArguments arguments);
    void movArm(DataProcessingArguments arguments);
    void bicArm(DataProcessingArguments arguments);
    void mvnArm(DataProcessingArguments arguments);
    using DataProcessingInstructionType = void (CPU::*)(DataProcessingArguments);
    std::array<DataProcessingInstructionType, 16> data_processing_instruction_type;
  
    std::pair<uint32_t, bool> calculateOperand2(uint32_t shifted_value, uint32_t shift_value, ShiftType shift_type);
    Opcode getOpcodeArm(uint32_t intruction_code) const;
    void callDataProcessingInstruction(uint32_t intruction_code);  // this function name needs to be changed

    void callMultiplyInstruction(uint32_t intruction_code);
    MultiplyArguments decodeMultiplyArguments(uint32_t instruction_code);

    void callMultiplyLongInstruction(uint32_t instruction_code);
    MultiplyLongArguments decodeMultiplyLongArguments(uint32_t instruction_code);
    void umullArm(MultiplyLongArguments arguments);
    void umlalArm(MultiplyLongArguments arguments);
    void smullArm(MultiplyLongArguments argumentse);
    void smlalArm(MultiplyLongArguments arguments);


    void callSingleDataTransferInstruction(uint32_t instruction_code);
    SingleDataTransferArguments decodeSingleDataTransferArguments(uint32_t instruction_code);
    void ldrArm(SingleDataTransferArguments arguments);
    void strArm(SingleDataTransferArguments arguments);

    void callHalfWordAndSignedDataTransferInstruction(uint32_t instruction_code);
    HalfWordAndSignedDataTransferArguments decodeHalfWordAndSignedDataTransferArguments(uint32_t instruction_code);
    void ldrhArm(HalfWordAndSignedDataTransferArguments arguments);
    void ldrshArm(HalfWordAndSignedDataTransferArguments arguments);
    void ldrsbArm(HalfWordAndSignedDataTransferArguments arguments);
    void strhArm(HalfWordAndSignedDataTransferArguments arguments);
    
    void swpArm(uint32_t instruction_code);
    
    // Stack Pointer, R13 by convention
    uint32_t& SP(Mode mode);
    const uint32_t& SP(Mode mode) const;

    // R14 is the Link Register
    uint32_t& LR(Mode mode);
    const uint32_t& LR(Mode mode) const;

    // R15 is the Program Counter
    // In ARM mode, bits 1 to 0 are undefined and must be ignored
    // In Thumb mode, bit 0 is undefined and must be ignored
    uint32_t& PC();
    const uint32_t& PC() const;

    // General purpose registers for User and System mode, R0 to R15
    uint32_t& R(uint32_t index);
    const uint32_t& R(uint32_t index) const;

    // General and banked registers for Fast Interrupt mode, R0 to R7, R8_FIQ to R14_FIQ, and R15
    uint32_t& R_FIQ(uint32_t index);
    const uint32_t& R_FIQ(uint32_t index) const;

    // General and banked registers for Supervisor mode, R0 to R12, R13_SVC to R14_SVC, and R15
    uint32_t& R_SVC(uint32_t index);
    const uint32_t& R_SVC(uint32_t index) const;

    // General and banked registers for Abort mode, R0 to R12, R13_ABT to R14_ABT, and R15
    uint32_t& R_ABT(uint32_t index);
    const uint32_t& R_ABT(uint32_t index) const;

    // General and banked registers for Interrupt mode, R0 to R12, R13_IRQ to R14_IRQ, and R15
    uint32_t& R_IRQ(uint32_t index);
    const uint32_t& R_IRQ(uint32_t index) const;

    // General and banked registers for Undefined mode, R0 to R12, R13_UND to R14_UND, and R15
    uint32_t& R_UND(uint32_t index);
    const uint32_t& R_UND(uint32_t index) const;

  private:
    enum class RegisterIndex {
        R0 = 0,
        R1 = 1,
        R2 = 2,
        R3 = 3,
        R4 = 4,
        R5 = 5,
        R6 = 6,
        R7 = 7,
        R8 = 8,
        R9 = 9,
        R10 = 10,
        R11 = 11,
        R12 = 12,
        R13 = 13,
        R14 = 14,
        R15 = 15,
        SP = R13,
        LR = R14,
        PC = R15,
        R8_FIQ = 16,
        R9_FIQ = 17,
        R10_FIQ = 18,
        R11_FIQ = 19,
        R12_FIQ = 20,
        R13_FIQ = 21,
        R14_FIQ = 22,
        SP_FIQ = R13_FIQ,
        LR_FIQ = R14_FIQ,
        R13_SVC = 23,
        R14_SVC = 24,
        SP_SVC = R13_SVC,
        LR_SVC = R14_SVC,
        R13_ABT = 25,
        R14_ABT = 26,
        SP_ABT = R13_ABT,
        LR_ABT = R14_ABT,
        R13_IRQ = 27,
        R14_IRQ = 28,
        SP_IRQ = R13_IRQ,
        LR_IRQ = R14_IRQ,
        R13_UND = 29,
        R14_UND = 30,
        SP_UND = R13_UND,
        LR_UND = R14_UND,
    };

    // All general and banked registers
    uint32_t registers[31];

    // Current Program Status Register
    uint32_t CPSR;

    // Saved Program Status Registers
    uint32_t SPSR_FIQ;
    uint32_t SPSR_SVC;
    uint32_t SPSR_ABT;
    uint32_t SPSR_IRQ;
    uint32_t SPSR_UND;
    Memory memory;
};
}

#endif
