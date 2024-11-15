#ifndef GBA_CPU_H
#define GBA_CPU_H

#include "common.h"
#include "instruction_types.h"
#include "opcode.h"
#include <array>

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

    InstructionType decodeArm(uint32_t opcode);
    // TODO: https://developer.arm.com/documentation/ddi0210/c/Programmer-s-Model/Reset
    void reset();

    Mode getMode() const;

    void ANDArm(uint32_t instruction_code);
    void XORArm(uint32_t instruction_code);
    void SUBArm(uint32_t instruction_code);
    void RSBArm(uint32_t instruction_code);
    void ADDArm(uint32_t instruction_code);
    void ADCArm(uint32_t instruction_code);
    void SBCArm(uint32_t instruction_code);
    void RSCArm(uint32_t instruction_code);
    void TSTArm(uint32_t instruction_code);
    void TEQArm(uint32_t instruction_code);
    void CMPArm(uint32_t instruction_code);
    void CMNArm(uint32_t instruction_code);
    void ORRArm(uint32_t instruction_code);
    void MOVArm(uint32_t instruction_code);
    void BICArm(uint32_t instruction_code);
    void MVNArm(uint32_t instruction_code);


    using DataProcessingInstructionType = void (CPU::*)(uint32_t);
    std::array<DataProcessingInstructionType, 16> data_processing_instruction_type;

    bool checkCondition(uint32_t intruction_code) const;
    Opcode getOpcodeArm(uint32_t intruction_code) const;
    void callDataProcessingInstruction(uint32_t intruction_code); // this function name needs to be changed

    


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
};
}

#endif
