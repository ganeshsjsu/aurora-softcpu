#pragma once

#include "aurora/cpu.hpp"
#include "aurora/instruction.hpp"

#include <optional>

namespace aurora {

class ControlUnit {
public:
    ControlUnit(Bus& bus, RegisterFile& registers, ALU& alu);
    void reset();
    bool step(bool trace = false);

private:
    DecodedInstruction fetchInstruction();
    Operand resolveOperand(const OperandDescriptor& descriptor, std::uint16_t& pc);
    bool execute(const DecodedInstruction& instruction, bool trace);

    Bus& bus_;
    RegisterFile& registers_;
    ALU& alu_;
};

}  // namespace aurora
