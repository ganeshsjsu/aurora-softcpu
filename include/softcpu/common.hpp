#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace softcpu {

// Constants defining the architecture of the SoftCPU
constexpr std::size_t kMemorySize =
    64 * 1024; // 64 KiB addressable space (16-bit address bus)
constexpr std::size_t kRegisterCount = 8; // 8 General purpose registers: R0-R7
constexpr std::uint16_t kResetVector =
    0x0000; // Default entry point (Program Counter start address)
constexpr std::uint16_t kStackReset =
    0xFF00; // Default stack pointer address (grows downwards)
constexpr std::uint8_t kInstructionHeaderSize =
    4; // Size of instruction header: opcode + two operands + modifier byte

// Status flags for the CPU status register
enum class StatusFlag : std::uint16_t {
  kCarry = 1 << 0,    // Carry flag: set if an arithmetic operation generates a
                      // carry or borrow
  kZero = 1 << 1,     // Zero flag: set if the result of an operation is zero
  kNegative = 1 << 2, // Negative flag: set if the most significant bit of the
                      // result is set
  kOverflow = 1 << 3  // Overflow flag: set if an arithmetic overflow occurs
};

// Bitwise OR operator for StatusFlag
inline constexpr StatusFlag operator|(StatusFlag lhs, StatusFlag rhs) {
  return static_cast<StatusFlag>(static_cast<std::uint16_t>(lhs) |
                                 static_cast<std::uint16_t>(rhs));
}

// Bitwise AND operator for StatusFlag
inline constexpr StatusFlag operator&(StatusFlag lhs, StatusFlag rhs) {
  return static_cast<StatusFlag>(static_cast<std::uint16_t>(lhs) &
                                 static_cast<std::uint16_t>(rhs));
}

// Bitwise OR assignment operator for StatusFlag
inline constexpr StatusFlag &operator|=(StatusFlag &lhs, StatusFlag rhs) {
  lhs = lhs | rhs;
  return lhs;
}

// Check if any flag is set
inline constexpr bool any(StatusFlag flag) {
  return static_cast<std::uint16_t>(flag) != 0;
}

// Wrapper for the status register to easily set and test flags
struct FlagRegister {
  std::uint16_t value{0};

  // Set or clear a specific flag
  void set(StatusFlag flag, bool on) {
    if (on) {
      value |= static_cast<std::uint16_t>(flag);
    } else {
      value &= ~static_cast<std::uint16_t>(flag);
    }
  }

  // Test if a specific flag is set
  bool test(StatusFlag flag) const {
    return (value & static_cast<std::uint16_t>(flag)) != 0;
  }
};

// Structure representing a decoded instruction
struct InstructionWord {
  std::uint8_t opcode{0};    // Operation code
  std::uint8_t operand_a{0}; // First operand (usually register index)
  std::uint8_t operand_b{
      0}; // Second operand (register index or immediate value part)
  std::uint8_t modifier{
      0}; // Modifier byte (e.g., for addressing modes or extended opcodes)
};

} // namespace softcpu
