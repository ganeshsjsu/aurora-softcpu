#pragma once

#include "aurora/common.hpp"
#include "aurora/instruction.hpp"

#include <array>
#include <cstdint>
#include <memory>

namespace aurora {

class Bus;
class ALU;
class ControlUnit;

struct RegisterFile {
    std::array<std::uint16_t, kRegisterCount> gpr{};
    std::uint16_t pc {kResetVector};
    std::uint16_t sp {kStackReset};
    FlagRegister flags{};

    void reset() {
        gpr.fill(0);
        gpr[kRegisterCount - 1] = kStackReset;
        pc = kResetVector;
        sp = kStackReset;
        flags.value = 0;
    }
};

struct DecodedInstruction {
    Opcode opcode {Opcode::NOP};
    Operand operand_a{};
    Operand operand_b{};
    std::uint8_t modifier {0};
    std::uint16_t size_bytes {kInstructionHeaderSize};
    std::uint16_t address {0};
};

class CPU {
public:
    explicit CPU(Bus& bus);
    ~CPU();

    void reset();
    bool step(bool trace = false);
    RegisterFile& registers() { return registers_; }
    const RegisterFile& registers() const { return registers_; }

private:
    Bus& bus_;
    std::unique_ptr<ALU> alu_;
    std::unique_ptr<ControlUnit> control_;
    RegisterFile registers_;
};

}  // namespace aurora
