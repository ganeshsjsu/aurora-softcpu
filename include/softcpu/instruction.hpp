#pragma once

#include "softcpu/common.hpp"

#include <cstddef>
#include <cstdint>

namespace softcpu {

// Enumeration of all supported CPU opcodes
enum class Opcode : std::uint8_t {
  NOP = 0x00,   // No Operation
  HALT = 0x01,  // Halt execution
  LDI = 0x02,   // Load Immediate
  MOV = 0x03,   // Move Register
  LOAD = 0x04,  // Load from Memory
  STORE = 0x05, // Store to Memory
  ADD = 0x06,   // Add Registers
  ADDI = 0x07,  // Add Immediate
  SUB = 0x08,   // Subtract Registers
  SUBI = 0x09,  // Subtract Immediate
  MUL = 0x0A,   // Multiply
  DIV = 0x0B,   // Divide
  AND = 0x0C,   // Bitwise AND
  OR = 0x0D,    // Bitwise OR
  XOR = 0x0E,   // Bitwise XOR
  NOT = 0x0F,   // Bitwise NOT
  SHL = 0x10,   // Shift Left
  SHR = 0x11,   // Shift Right
  CMP = 0x12,   // Compare
  JMP = 0x13,   // Jump Unconditional
  JZ = 0x14,    // Jump if Zero
  JNZ = 0x15,   // Jump if Not Zero
  JN = 0x16,    // Jump if Negative
  JC = 0x17,    // Jump if Carry
  CALL = 0x18,  // Call Subroutine
  RET = 0x19,   // Return from Subroutine
  PUSH = 0x1A,  // Push to Stack
  POP = 0x1B,   // Pop from Stack
  OUT = 0x1C,   // Output to Port
  IN = 0x1D,    // Input from Port
  ADJSP = 0x1E, // Adjust Stack Pointer
  SYS = 0x1F    // System Call
};

// Types of operands supported by the instruction set
enum class OperandType : std::uint8_t {
  None = 0,             // No operand
  Register = 1,         // Register direct (e.g., R0)
  RegisterIndirect = 2, // Register indirect (e.g., [R0])
  RegisterIndexed = 3,  // Register indexed (e.g., [R0 + offset])
  Immediate = 4,        // Immediate value (e.g., #1234)
  Absolute = 5,         // Absolute memory address (e.g., [0x1234])
  Port = 6              // I/O Port (e.g., %10)
};

// Descriptor for a decoded operand from an instruction byte
struct OperandDescriptor {
  OperandType type{OperandType::None};
  std::uint8_t payload{0}; // Register index or other small data
};

// Full operand information including resolved values
struct Operand {
  OperandType type{OperandType::None};
  std::uint8_t reg{0};
  std::uint16_t value{0};
  std::int16_t offset{0};
  bool has_offset{false};
};

// Encode an operand type and payload into a single byte
constexpr std::uint8_t encodeOperand(OperandType type,
                                     std::uint8_t payload = 0) {
  return (static_cast<std::uint8_t>(type) << 5) | (payload & 0x1F);
}

// Decode a single byte into an operand descriptor
constexpr OperandDescriptor decodeOperand(std::uint8_t raw) {
  return OperandDescriptor{static_cast<OperandType>((raw >> 5) & 0x07),
                           static_cast<std::uint8_t>(raw & 0x1F)};
}

// Check if an operand type requires an additional 16-bit word
inline bool operandNeedsWord(OperandType type) {
  switch (type) {
  case OperandType::Immediate:
  case OperandType::Absolute:
  case OperandType::RegisterIndexed:
    return true;
  default:
    return false;
  }
}

// Get the string representation of an opcode
inline const char *opcodeName(Opcode opcode) {
  switch (opcode) {
  case Opcode::NOP:
    return "NOP";
  case Opcode::HALT:
    return "HALT";
  case Opcode::LDI:
    return "LDI";
  case Opcode::MOV:
    return "MOV";
  case Opcode::LOAD:
    return "LOAD";
  case Opcode::STORE:
    return "STORE";
  case Opcode::ADD:
    return "ADD";
  case Opcode::ADDI:
    return "ADDI";
  case Opcode::SUB:
    return "SUB";
  case Opcode::SUBI:
    return "SUBI";
  case Opcode::MUL:
    return "MUL";
  case Opcode::DIV:
    return "DIV";
  case Opcode::AND:
    return "AND";
  case Opcode::OR:
    return "OR";
  case Opcode::XOR:
    return "XOR";
  case Opcode::NOT:
    return "NOT";
  case Opcode::SHL:
    return "SHL";
  case Opcode::SHR:
    return "SHR";
  case Opcode::CMP:
    return "CMP";
  case Opcode::JMP:
    return "JMP";
  case Opcode::JZ:
    return "JZ";
  case Opcode::JNZ:
    return "JNZ";
  case Opcode::JN:
    return "JN";
  case Opcode::JC:
    return "JC";
  case Opcode::CALL:
    return "CALL";
  case Opcode::RET:
    return "RET";
  case Opcode::PUSH:
    return "PUSH";
  case Opcode::POP:
    return "POP";
  case Opcode::OUT:
    return "OUT";
  case Opcode::IN:
    return "IN";
  case Opcode::ADJSP:
    return "ADJSP";
  case Opcode::SYS:
    return "SYS";
  }
  return "?";
}

} // namespace softcpu
