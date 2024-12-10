#ifndef GBA_INSTRUCTION_TYPES_H
#define GBA_INSTRUCTION_TYPES_H

#include <string>

namespace GBA {

enum class InstructionType {
    DataProcessing,
    ProgramStatusRegisterTransferOut,
    ProgramStatusRegisterTransferIn,
    Multiply,
    MultiplyLong,
    SingleDataSwap,
    BranchAndExchange,
    HalfwordDataTransferRegister,
    HalfwordDataTransferImmediate,
    SingleDataTransfer,
    BlockDataTransfer,
    Branch,
    CoprocessorDataTransfer,
    CoprocessorDataOperation,
    CoprocessorRegisterTransfer,
    SoftwareInterrupt,
    Undefined,
};

enum class ThumbInstructionType {
    MoveShiftedRegister,
    AddSubtract,
    MoveCompareAddSubtractImmediate,
    ALUOperation,
    HighRegisterOperationBranchExchange,
    PCRelativeLoad,
    LoadStoreRegOffset,
    LoadStoreSignByteHalfword,
    LoadStoreImmediateOffset,
    LoadStoreHalfword,
    SPRelativeLoadStore,
    LoadAddress,
    AddOffsetToStackPointer,
    PushPopRegisters,
    MultipleLoadStore,
    ConditionalBranch,
    SoftwareInterrupt,
    UnconditionalBranch,
    LongBranchLink,
    Undefined,
};

constexpr const char* getInstructionTypeText(InstructionType instruction_type) {
    switch (instruction_type) {
    case InstructionType::DataProcessing:
        return "DataProcessing";
    case InstructionType::ProgramStatusRegisterTransferOut:
        return "ProgramStatusRegisterTransferOut";
    case InstructionType::ProgramStatusRegisterTransferIn:
        return "ProgramStatusRegisterTransferIn";
    case InstructionType::Multiply:
        return "Multiply";
    case InstructionType::MultiplyLong:
        return "MultiplyLong";
    case InstructionType::SingleDataSwap:
        return "SingleDataSwap";
    case InstructionType::BranchAndExchange:
        return "BranchAndExchange";
    case InstructionType::HalfwordDataTransferRegister:
        return "HalfwordDataTransferRegister";
    case InstructionType::HalfwordDataTransferImmediate:
        return "HalfwordDataTransferImmediate";
    case InstructionType::SingleDataTransfer:
        return "SingleDataTransfer";
    case InstructionType::BlockDataTransfer:
        return "BlockDataTransfer";
    case InstructionType::Branch:
        return "Branch";
    case InstructionType::CoprocessorDataTransfer:
        return "CoprocessorDataTransfer";
    case InstructionType::CoprocessorDataOperation:
        return "CoprocessorDataOperation";
    case InstructionType::CoprocessorRegisterTransfer:
        return "CoprocessorRegisterTransfer";
    case InstructionType::SoftwareInterrupt:
        return "SoftwareInterrupt";
    case InstructionType::Undefined:
    default:
        return "Undefined";
    }
}

}

#endif
