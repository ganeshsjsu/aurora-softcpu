#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace aurora {

constexpr std::size_t kMemorySize = 64 * 1024;          // 64 KiB addressable space
constexpr std::size_t kRegisterCount = 8;               // General purpose registers R0-R7
constexpr std::uint16_t kResetVector = 0x0000;          // Default entry point
constexpr std::uint16_t kStackReset = 0xFF00;           // Default stack pointer address
constexpr std::uint8_t kInstructionHeaderSize = 4;      // opcode + two operands + modifier byte

enum class StatusFlag : std::uint16_t {
    kCarry = 1 << 0,
    kZero = 1 << 1,
    kNegative = 1 << 2,
    kOverflow = 1 << 3
};

inline constexpr StatusFlag operator|(StatusFlag lhs, StatusFlag rhs) {
    return static_cast<StatusFlag>(static_cast<std::uint16_t>(lhs) | static_cast<std::uint16_t>(rhs));
}

inline constexpr StatusFlag operator&(StatusFlag lhs, StatusFlag rhs) {
    return static_cast<StatusFlag>(static_cast<std::uint16_t>(lhs) & static_cast<std::uint16_t>(rhs));
}

inline constexpr StatusFlag& operator|=(StatusFlag& lhs, StatusFlag rhs) {
    lhs = lhs | rhs;
    return lhs;
}

inline constexpr bool any(StatusFlag flag) {
    return static_cast<std::uint16_t>(flag) != 0;
}

struct FlagRegister {
    std::uint16_t value {0};

    void set(StatusFlag flag, bool on) {
        if (on) {
            value |= static_cast<std::uint16_t>(flag);
        } else {
            value &= ~static_cast<std::uint16_t>(flag);
        }
    }

    bool test(StatusFlag flag) const {
        return (value & static_cast<std::uint16_t>(flag)) != 0;
    }
};

struct InstructionWord {
    std::uint8_t opcode {0};
    std::uint8_t operand_a {0};
    std::uint8_t operand_b {0};
    std::uint8_t modifier {0};
};

}  // namespace aurora
