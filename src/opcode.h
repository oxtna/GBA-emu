#ifndef GBA_OPCODE_H
#define GBA_OPCODE_H
namespace GBA {
    enum Opcode
    {
        AND = 0x00,
        XOR = 0x01,
        SUB = 0x02,
        RSB = 0x03,
        ADD = 0x04,
        ADC = 0x05,
        SBC = 0x06,
        RSC = 0x07,
        TST = 0x08,
        TEQ = 0x09,
        CMP = 0x0A,
        CMN = 0x0B,
        ORR = 0x0C,
        MOV = 0x0D,
        BIC = 0x0E,
        MVN = 0x0F
    };
}
#endif