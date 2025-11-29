#pragma once

#include "softcpu/common.hpp"
#include "softcpu/instruction.hpp"

#include <array>
#include <cstdint>
#include <memory>

namespace softcpu {

class Bus;
class ALU;
class ControlUnit;

// Structure holding the CPU's register state
struct RegisterFile {
  std::array<std::uint16_t, kRegisterCount> gpr{}; // General Purpose Registers
  std::uint16_t pc{kResetVector};                  // Program Counter
  std::uint16_t sp{kStackReset};                   // Stack Pointer
  FlagRegister flags{};                            // Status Flags

  // Reset registers to their default state
  void reset() {
    gpr.fill(0);
    gpr[kRegisterCount - 1] =
        kStackReset;
    pc = kResetVector;
    sp = kStackReset;
    flags.value = 0;
  }
};

// Structure representing a fully decoded instruction ready for execution
struct DecodedInstruction {
  Opcode opcode{Opcode::NOP};
  Operand operand_a{};
  Operand operand_b{};
  std::uint8_t modifier{0};
  std::uint16_t size_bytes{kInstructionHeaderSize};
  std::uint16_t address{0}; // Address where the instruction is located
};

// The Central Processing Unit
class CPU {
public:
  explicit CPU(Bus &bus);
  ~CPU();

  // Reset the CPU to its initial state
  void reset();

  // Execute a single instruction. Returns false if HALT is encountered or an
  // error occurs.
  bool step(bool trace = false);

  // Access the register file
  RegisterFile &registers() { return registers_; }
  const RegisterFile &registers() const { return registers_; }

private:
  Bus &bus_;
  std::unique_ptr<ALU> alu_;
  std::unique_ptr<ControlUnit> control_;
  RegisterFile registers_;
};

} // namespace softcpu
