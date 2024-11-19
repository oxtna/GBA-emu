#include "../cpu.h"
#include <array>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

using namespace GBA;

InstructionType getInstructionType(uint32_t instruction) {
    CPU cpu;
    return cpu.decodeArm(instruction);
}

int main() {
    std::vector<std::pair<uint32_t, InstructionType>> test_cases = {
        {0x00000000, InstructionType::DataProcessing},
        {0xF3FFFFFF, InstructionType::DataProcessing},
        {0x010F0000, InstructionType::ProgramStatusRegisterTransferOut},
        {0x014F0000, InstructionType::ProgramStatusRegisterTransferOut},
        {0xF10F0000, InstructionType::ProgramStatusRegisterTransferOut},
        {0xF10FF000, InstructionType::ProgramStatusRegisterTransferOut},
        {0xF14F0000, InstructionType::ProgramStatusRegisterTransferOut},
        {0xF14FF000, InstructionType::ProgramStatusRegisterTransferOut},
        {0x0129F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0129F00F, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0169F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0169F00F, InstructionType::ProgramStatusRegisterTransferIn},
        {0xF129F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0xF129F00F, InstructionType::ProgramStatusRegisterTransferIn},
        {0xF169F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0xF169F00F, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0128F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0128FFFF, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0168F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0328F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0328FFFF, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0368F000, InstructionType::ProgramStatusRegisterTransferIn},
        {0x0368FFFF, InstructionType::ProgramStatusRegisterTransferIn},
        {0x00000090, InstructionType::Multiply},
        {0xF03FFF9F, InstructionType::Multiply},
        {0x00800090, InstructionType::MultiplyLong},
        {0xF0FFFF9F, InstructionType::MultiplyLong},
        {0x01000090, InstructionType::SingleDataSwap},
        {0xF14FF09F, InstructionType::SingleDataSwap},
        {0x012FFF10, InstructionType::BranchAndExchange},
        {0xF12FFF1F, InstructionType::BranchAndExchange},
        {0x000000B0, InstructionType::HalfwordDataTransferRegister},
        {0x000000D0, InstructionType::HalfwordDataTransferRegister},
        {0x000000F0, InstructionType::HalfwordDataTransferRegister},
        {0xF1BFF0BF, InstructionType::HalfwordDataTransferRegister},
        {0xF1BFF0DF, InstructionType::HalfwordDataTransferRegister},
        {0xF1BFF0FF, InstructionType::HalfwordDataTransferRegister},
        {0x004000B0, InstructionType::HalfwordDataTransferImmediate},
        {0x004000D0, InstructionType::HalfwordDataTransferImmediate},
        {0x004000F0, InstructionType::HalfwordDataTransferImmediate},
        {0xF1FFFFBF, InstructionType::HalfwordDataTransferImmediate},
        {0xF1FFFFDF, InstructionType::HalfwordDataTransferImmediate},
        {0xF1FFFFFF, InstructionType::HalfwordDataTransferImmediate},
        {0x04000000, InstructionType::SingleDataTransfer},
        {0x04000010, InstructionType::SingleDataTransfer},
        {0x05000000, InstructionType::SingleDataTransfer},
        {0x05000010, InstructionType::SingleDataTransfer},
        {0x06000000, InstructionType::SingleDataTransfer},
        {0x07000000, InstructionType::SingleDataTransfer},
        {0xF7FFFFEF, InstructionType::SingleDataTransfer},
        {0x08000000, InstructionType::BlockDataTransfer},
        {0xF9FFFFFF, InstructionType::BlockDataTransfer},
        {0x0A000000, InstructionType::Branch},
        {0xFBFFFFFF, InstructionType::Branch},
        {0x0C000000, InstructionType::CoprocessorDataTransfer},
        {0xFDFFFFFF, InstructionType::CoprocessorDataTransfer},
        {0x0E000000, InstructionType::CoprocessorDataOperation},
        {0xFEFFFFEF, InstructionType::CoprocessorDataOperation},
        {0x0E000010, InstructionType::CoprocessorRegisterTransfer},
        {0xFEFFFFFF, InstructionType::CoprocessorRegisterTransfer},
        {0x0F000000, InstructionType::SoftwareInterrupt},
        {0xFFFFFFFF, InstructionType::SoftwareInterrupt},
        {0x06000010, InstructionType::Undefined},
        {0xF7FFFFFF, InstructionType::Undefined},
    };

    bool failed = false;
    for (const auto& test_case : test_cases) {
        const auto& [instruction, expected_instruction_type] = test_case;
        auto instruction_type = getInstructionType(instruction);
        if (instruction_type != expected_instruction_type) {
            failed = true;
            std::cerr << "Test failed for instruction 0x" << std::hex << std::uppercase << std::setw(8)
                      << std::setfill('0') << instruction << std::nouppercase << std::dec << ": expected '"
                      << getInstructionTypeText(expected_instruction_type) << "' but got '"
                      << getInstructionTypeText(instruction_type) << "' instead\n";
        }
    }

    return failed ? 1 : 0;
}

