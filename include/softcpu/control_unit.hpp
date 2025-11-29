#pragma once

#include "softcpu/cpu.hpp"
#include "softcpu/instruction.hpp"

#include <optional>

namespace softcpu {

// The Control Unit orchestrates the fetch-decode-execute cycle
class ControlUnit {
public:
  ControlUnit(Bus &bus, RegisterFile &registers, ALU &alu);

  // Reset the control unit state
  void reset();

  // Perform one instruction cycle (fetch, decode, execute)
  bool step(bool trace = false);

private:
  // Fetch the next instruction from memory pointed to by PC
  DecodedInstruction fetchInstruction();

  // Decode and resolve an operand (e.g., read immediate value or calculate
  // address)
  Operand resolveOperand(const OperandDescriptor &descriptor,
                         std::uint16_t &pc);

  // Execute the decoded instruction
  bool execute(const DecodedInstruction &instruction, bool trace);

  Bus &bus_;
  RegisterFile &registers_;
  ALU &alu_;
};

} // namespace softcpu
