#include "instruction_types.h" 

using namespace GBA;
std::string getInstructionTypeString(InstructionType instruction_type)
{
    switch(instruction_type)
    {
        case InstructionType::DataProcessing:
            return "DataProcessing";
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
            return "Undefined";
    }
    return "Undefined";
}