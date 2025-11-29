#pragma once

#include "softcpu/common.hpp"

#include <cstdint>

namespace softcpu {

// Result of an ALU operation, containing the result value and status flags
struct ALUResult {
  std::uint16_t value{0};
  FlagRegister flags;
};

// Arithmetic Logic Unit responsible for mathematical and logical operations
class ALU {
public:
  // Addition with optional carry
  ALUResult add(std::uint16_t lhs, std::uint16_t rhs,
                bool with_carry = false) const;

  // Subtraction
  ALUResult sub(std::uint16_t lhs, std::uint16_t rhs) const;

  // Bitwise AND
  ALUResult bit_and(std::uint16_t lhs, std::uint16_t rhs) const;

  // Bitwise OR
  ALUResult bit_or(std::uint16_t lhs, std::uint16_t rhs) const;

  // Bitwise XOR
  ALUResult bit_xor(std::uint16_t lhs, std::uint16_t rhs) const;

  // Bitwise NOT (One's complement)
  ALUResult bit_not(std::uint16_t value) const;

  // Logical Shift Left
  ALUResult shl(std::uint16_t value, std::uint8_t amount) const;

  // Logical Shift Right
  ALUResult shr(std::uint16_t value, std::uint8_t amount) const;

  // Multiplication
  ALUResult mul(std::uint16_t lhs, std::uint16_t rhs) const;

  // Division
  ALUResult divide(std::uint16_t lhs, std::uint16_t rhs) const;
};

} // namespace softcpu
