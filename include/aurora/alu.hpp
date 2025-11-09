#pragma once

#include "aurora/common.hpp"

#include <cstdint>

namespace aurora {

struct ALUResult {
    std::uint16_t value {0};
    FlagRegister flags;
};

class ALU {
public:
    ALUResult add(std::uint16_t lhs, std::uint16_t rhs, bool with_carry = false) const;
    ALUResult sub(std::uint16_t lhs, std::uint16_t rhs) const;
    ALUResult bit_and(std::uint16_t lhs, std::uint16_t rhs) const;
    ALUResult bit_or(std::uint16_t lhs, std::uint16_t rhs) const;
    ALUResult bit_xor(std::uint16_t lhs, std::uint16_t rhs) const;
    ALUResult bit_not(std::uint16_t value) const;
    ALUResult shl(std::uint16_t value, std::uint8_t amount) const;
    ALUResult shr(std::uint16_t value, std::uint8_t amount) const;
    ALUResult mul(std::uint16_t lhs, std::uint16_t rhs) const;
    ALUResult divide(std::uint16_t lhs, std::uint16_t rhs) const;
};

}  // namespace aurora
