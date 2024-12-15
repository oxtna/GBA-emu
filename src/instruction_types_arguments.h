#ifndef GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#define GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#include "shift_types.h"
#include <cstdint>

namespace GBA {

struct DataProcessingArguments
{
    bool S;  // status bit (if 1, the instruction updates the CPSR)
    uint32_t Rn;
    uint32_t Rd;
    uint32_t shifted_value;
    uint32_t shift_value;
    ShiftType shift_type;
    uint32_t PC;  // address of the instruction

    DataProcessingArguments() = default;
    DataProcessingArguments(
        bool S, uint32_t _Rd, uint32_t _Rn, uint32_t _shifted_value, uint32_t _shift_value, ShiftType _shift_type,
        uint32_t _PC)
        : S(S),
          Rn(_Rn),
          Rd(_Rd),
          shifted_value(_shifted_value),
          shift_value(_shift_value),
          shift_type(_shift_type),
          PC(_PC){};
};

struct MultiplyArguments
{
    bool A;  // accumulate bit
    bool S;  // status bit
    uint32_t Rd;
    uint32_t Rn;
    uint32_t Rs;
    uint32_t Rm;
    uint32_t PC;  // address of the instruction

    MultiplyArguments() = default;
    MultiplyArguments(bool _S, bool _A, uint32_t _Rd, uint32_t _Rn, uint32_t _Rs, uint32_t _Rm, uint32_t _PC)
        : A(_A), S(_S), Rd(_Rd), Rn(_Rn), Rs(_Rs), Rm(_Rm), PC(_PC){};
};

struct MultiplyLongArguments
{
    bool U;  // unsigned bit
    bool A;  // accumulate bit
    bool S;  // status bit
    uint32_t RdHi;
    uint32_t RdLo;
    uint32_t Rm;
    uint32_t Rs;
    uint32_t PC;  // address of the instruction
};

struct SingleDataTransferArguments
{
    bool I;  // immediate offset bit
    bool P;  // pre/post indexing bit
    bool U;  // up/down bit
    bool B;  // byte/word bit
    bool W;  // write-back bit
    bool L;  // load/store bit
    uint32_t Rn;
    uint32_t Rd;
    uint32_t offset;
    uint32_t PC;  // address of the instruction
};

struct HalfWordAndSignedDataTransferArguments
{
    bool P;  // pre/post indexing bit
    bool U;  // up/down bit
    bool W;  // write-back bit
    bool L;  // load/store bit
    bool S;  // sign bit
    bool H;  // halfword/signed data bit
    uint32_t Rn;
    uint32_t Rd;
    uint32_t offset;
    uint32_t PC;  // address of the instruction

    HalfWordAndSignedDataTransferArguments() = default;
    HalfWordAndSignedDataTransferArguments(
        bool _P, bool _U, bool _W, bool _L, bool _S, bool _H, uint32_t _Rn, uint32_t _Rd, uint32_t _offset,
        uint32_t _PC)
        : P(_P), U(_U), W(_W), L(_L), S(_S), H(_H), Rn(_Rn), Rd(_Rd), offset(_offset), PC(_PC){};
};

struct BlockDataTransferArguments
{
    bool P;              // pre/post indexing bit
    bool U;              // up/down bit
    bool S;              // program status register and force user bit
    bool W;              // write-back bit
    bool L;              // load/store bit
    uint32_t Rn;         // base register
    uint32_t registers;  // register list bitfield
    uint32_t PC;         // address of the instruction

    BlockDataTransferArguments() = default;
    BlockDataTransferArguments(
        bool _P, bool _U, bool _S, bool _W, bool _L, uint32_t _Rn, uint32_t _registers, uint32_t _PC)
        : P(_P), U(_U), S(_S), W(_W), L(_L), Rn(_Rn), registers(_registers), PC(_PC){};
};

struct SingleDataSwapArguments
{
    uint32_t Rn;
    uint32_t Rd;
    uint32_t PC;  // address of the instruction
};

struct CoprocessorDataOperationsArguments
{
    uint32_t CP_opc;
    uint32_t CRn;
    uint32_t CRd;
    uint32_t CP_num;
    uint32_t CP_inf;
    uint32_t C_Rm;
    uint32_t PC;  // address of the instruction
};

struct CoprocessorDataTransfersArguments
{
    bool P;  // pre/post indexing bit
    bool U;  // up/down bit
    bool N;  // transfer length bit
    bool W;  // write-back bit
    bool L;  // load/store bit
    uint32_t Rn;
    uint32_t CRd;
    uint32_t CP_num;
    uint32_t Offset;
    uint32_t PC;  // address of the instruction
};

struct CoprocessorRegisterTransfersArguments
{
    uint32_t CP_opc;
    bool L;  // load/store bit
    uint32_t C_Rn;
    uint32_t Rd;
    uint32_t CP_num;
    uint32_t CP_inf;
    uint32_t C_Rm;
    uint32_t PC;  // address of the instruction
};

struct MoveShifterRegisterThumbArguments
{
    uint32_t Rd;
    uint32_t Rs;
    uint32_t shift_value;
    ShiftType shift_type;
    uint32_t PC;  // address of the instruction
};

struct AddSubtractThumbArguments
{
    bool I;  // immediate bit
    bool op;
    uint32_t Rd;
    uint32_t Rs;
    uint32_t operand2;
    uint32_t PC;  // address of the instruction
};

struct MoveCompareAddSubtractImmediateThumbArguments
{
    uint32_t Rd;
    uint32_t op;
    uint32_t offset;
    uint32_t PC;  // address of the instruction
};

struct ALUoperationThumbArguments
{
    uint32_t Rd;
    uint32_t Rs;
    uint32_t op;
    uint32_t PC;  // address of the instruction
};

struct HiRegisterOperationsBranchExchangeArguments
{
    uint32_t H1;
    uint32_t H2;
    uint32_t Rd;
    uint32_t Rs;
    uint32_t op;
    uint32_t PC;  // address of the instruction
};

struct LoadStoreRegOffsetArguments
{
    bool B;  // byte/word bit
    bool L;  // load/store bit
    uint32_t Rd;
    uint32_t Rb;
    uint32_t Ro;
    uint32_t PC;  // address of the instruction
};

struct LoadStoreImmediateOffsetArguments
{
    bool B;  // byte/word bit
    bool L;  // load/store bit
    uint32_t Rb;
    uint32_t Rd;
    uint32_t offset;
    uint32_t PC;  // address of the instruction
};

struct LoadStoreSignExtendedByteHalfwordArguments
{
    bool H;  // halfword/byte bit
    bool S;  // sign bit
    uint32_t Rb;
    uint32_t Rd;
    uint32_t Ro;
    uint32_t PC;  // address of the instruction
};

struct LoadStoreHalfwordArguments
{
    bool L;  // load/store bit
    uint32_t Rb;
    uint32_t Rd;
    uint32_t offset;
    uint32_t PC;  // address of the instruction
};

struct SPRelativeLoadStoreArguments
{
    bool L;  // load/store bit
    uint32_t Rd;
    uint32_t offset;
    uint32_t PC;  // address of the instruction
};

struct LoadAddressArguments
{
    uint32_t Rd;
    uint32_t offset;
    bool SP;
    uint32_t PC;  // address of the instruction
};

struct PushPopRegistersArguments
{
    bool R;  // register list bit
    bool L;  // load/store bit
    uint32_t Rlist;
    uint32_t PC;  // address of the instruction
};

}

#endif
