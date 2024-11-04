#ifndef GBA_INSTRUCTION_TYPES_H
#define GBA_INSTRUCTION_TYPES_H
namespace GBA
{
    enum class InstructionType
    {
        DataProcessing,
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
        Undefined
    };
}
#endif