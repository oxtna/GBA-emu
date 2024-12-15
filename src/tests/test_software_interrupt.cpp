#include "../cpu.h"
#include <array>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

using namespace GBA;

int main() {
    bool failed = false;
    CPU cpu;
    cpu.setMode(CPU::Mode::User);
    cpu.PC() = 0;
    auto cpsr = cpu.getCPSR();

    cpu.callSoftwareInterruptInstruction(0xEF000000, cpu.PC());
    if (cpu.getMode() != CPU::Mode::Supervisor) {
        failed = true;
        std::cerr << "Software Interrupt instruction did not cause CPU to enter Supervisor mode\n";
    }
    if (cpu.PC() != 0x8) {
        failed = true;
        std::cerr << "Software Interrupt instruction did not set PC to 0x08\n";
    }
    if (cpu.LR(CPU::Mode::Supervisor) != 0x4) {
        failed = true;
        std::cerr << "Software Interrupt instruction did not set LR_SVC to the next instruction address\n";
    }

    cpu.callDataProcessingInstruction(0xE1B0F00E, cpu.PC());
    if (cpu.PC() != 0x4) {
        failed = true;
        std::cerr << "Returning from Supervisor mode did not set PC to the next instruction's saved address\n";
    }
    if (cpu.getMode() != CPU::Mode::User) {
        failed = true;
        std::cerr << "Returning from Supervisor mode did not restore previous CPU mode\n";
    }
    if (cpu.getCPSR() != cpsr) {
        failed = true;
        std::cerr << "Returning from Supervisor mode did not restore the CPSR\n";
    }

    return failed ? 1 : 0;
}
