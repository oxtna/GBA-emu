#ifndef GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#define GBA_INSTRUCTION_TYPES_ARGUMENTS_H
#include<cstdint>
#include "shift_types.h"

namespace GBA {
    struct DataProcessingArguments {
        bool S;   // status bit (if 1, the instruction updates the CPSR)
        uint32_t Rn;
        uint32_t Rd;
        uint32_t shifted_value;
        uint32_t shift_value;
        ShiftType shift_type;
    };

    struct MultiplyArguments{
        bool A;   // accumulate bit
        bool S;   // status bit
        uint32_t Rd;
        uint32_t Rn;
        uint32_t Rs;
        uint32_t Rm;
    };

    struct MultiplyLongArguments{
        bool U;   // unsigned bit
        bool A;   // accumulate bit
        bool S;   // status bit
        uint32_t RdHi;
        uint32_t RdLo;
        uint32_t Rm;
        uint32_t Rs;
    };

    struct SingleDataTransferArguments{
        bool I;   // immediate offset bit
        bool P;   // pre/post indexing bit
        bool U;   // up/down bit
        bool B;   // byte/word bit
        bool W;   // write-back bit
        bool L;   // load/store bit
        uint32_t Rn;
        uint32_t Rd;
        uint32_t offset;
    };
    
    struct HalfWordAndSignedDataTransferArguments{
        bool P;   // pre/post indexing bit
        bool U;   // up/down bit
        bool W;   // write-back bit
        bool L;   // load/store bit
        bool S;   // sign bit
        bool H;   // halfword/signed data bit
        uint32_t Rn;
        uint32_t Rd;
        uint32_t offset;
    };

    struct BlockDataTransferArguments{
        bool P;   // pre/post indexing bit
        bool U;   // up/down bit
        bool S;   // PSR and force user bit
        bool W;   // write-back bit
        bool L;   // load/store bit
        uint32_t Rn;
        uint32_t register_list;
    };

    struct SingleDataSwapArguments{
        uint32_t Rn;
        uint32_t Rd;
    };

    struct CoprocessorDataOperationsArguments{
        uint32_t CP_opc;
        uint32_t CRn;
        uint32_t CRd;
        uint32_t CP_num;
        uint32_t CP_inf;
        uint32_t C_Rm;
    };

    struct CoprocessorDataTransfersArguments{
        bool P;   // pre/post indexing bit
        bool U;   // up/down bit
        bool N;   // transfer length bit
        bool W;   // write-back bit
        bool L;   // load/store bit
        uint32_t Rn;
        uint32_t CRd;
        uint32_t CP_num;
        uint32_t Offset;
    };

    struct CoprocessorRegisterTransfersArguments{
        uint32_t CP_opc;
        bool L;   // load/store bit
        uint32_t C_Rn;
        uint32_t Rd;
        uint32_t CP_num;
        uint32_t CP_inf;
        uint32_t C_Rm;
    };
    
}

#endif