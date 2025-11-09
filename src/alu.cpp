#include "aurora/alu.hpp"

namespace aurora {

namespace {
FlagRegister updateCommonFlags(std::uint32_t result, bool carry, bool overflow) {
    FlagRegister flags;
    const std::uint16_t value16 = static_cast<std::uint16_t>(result & 0xFFFF);
    flags.set(StatusFlag::kZero, value16 == 0);
    flags.set(StatusFlag::kNegative, (value16 & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, carry);
    flags.set(StatusFlag::kOverflow, overflow);
    return flags;
}
}

ALUResult ALU::add(std::uint16_t lhs, std::uint16_t rhs, bool with_carry) const {
    const std::uint32_t carry_in = with_carry ? 1 : 0;
    const std::uint32_t wide = static_cast<std::uint32_t>(lhs) + static_cast<std::uint32_t>(rhs) + carry_in;
    const bool carry = wide > 0xFFFF;
    const bool overflow = (~(lhs ^ rhs) & (lhs ^ static_cast<std::uint16_t>(wide)) & 0x8000) != 0;
    return {static_cast<std::uint16_t>(wide & 0xFFFF), updateCommonFlags(wide, carry, overflow)};
}

ALUResult ALU::sub(std::uint16_t lhs, std::uint16_t rhs) const {
    const std::uint32_t wide = static_cast<std::uint32_t>(lhs) - static_cast<std::uint32_t>(rhs);
    const bool carry = lhs >= rhs;
    const bool overflow = ((lhs ^ rhs) & (lhs ^ static_cast<std::uint16_t>(wide)) & 0x8000) != 0;
    return {static_cast<std::uint16_t>(wide & 0xFFFF), updateCommonFlags(wide, carry, overflow)};
}

ALUResult ALU::bit_and(std::uint16_t lhs, std::uint16_t rhs) const {
    const std::uint16_t value = static_cast<std::uint16_t>(lhs & rhs);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, value == 0);
    flags.set(StatusFlag::kNegative, (value & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
    return {value, flags};
}

ALUResult ALU::bit_or(std::uint16_t lhs, std::uint16_t rhs) const {
    const std::uint16_t value = static_cast<std::uint16_t>(lhs | rhs);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, value == 0);
    flags.set(StatusFlag::kNegative, (value & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
    return {value, flags};
}

ALUResult ALU::bit_xor(std::uint16_t lhs, std::uint16_t rhs) const {
    const std::uint16_t value = static_cast<std::uint16_t>(lhs ^ rhs);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, value == 0);
    flags.set(StatusFlag::kNegative, (value & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
    return {value, flags};
}

ALUResult ALU::bit_not(std::uint16_t value) const {
    const std::uint16_t result = static_cast<std::uint16_t>(~value);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, result == 0);
    flags.set(StatusFlag::kNegative, (result & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
    return {result, flags};
}

ALUResult ALU::shl(std::uint16_t value, std::uint8_t amount) const {
    amount %= 16;
    const std::uint32_t wide = static_cast<std::uint32_t>(value) << amount;
    const bool carry = (wide >> 16) != 0;
    FlagRegister flags = updateCommonFlags(wide, carry, false);
    return {static_cast<std::uint16_t>(wide & 0xFFFF), flags};
}

ALUResult ALU::shr(std::uint16_t value, std::uint8_t amount) const {
    amount %= 16;
    if (amount == 0) {
        FlagRegister flags;
        flags.set(StatusFlag::kZero, value == 0);
        flags.set(StatusFlag::kNegative, false);
        flags.set(StatusFlag::kCarry, false);
        flags.set(StatusFlag::kOverflow, false);
        return {value, flags};
    }
    const std::uint16_t result = static_cast<std::uint16_t>(value >> amount);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, result == 0);
    flags.set(StatusFlag::kNegative, false);
    flags.set(StatusFlag::kCarry, (value >> (amount - 1)) & 0x1);
    flags.set(StatusFlag::kOverflow, false);
    return {result, flags};
}

ALUResult ALU::mul(std::uint16_t lhs, std::uint16_t rhs) const {
    const std::uint32_t wide = static_cast<std::uint32_t>(lhs) * static_cast<std::uint32_t>(rhs);
    const bool carry = (wide >> 16) != 0;
    FlagRegister flags = updateCommonFlags(wide, carry, false);
    return {static_cast<std::uint16_t>(wide & 0xFFFF), flags};
}

ALUResult ALU::divide(std::uint16_t lhs, std::uint16_t rhs) const {
    if (rhs == 0) {
        FlagRegister flags;
        flags.set(StatusFlag::kZero, false);
        flags.set(StatusFlag::kNegative, false);
        flags.set(StatusFlag::kCarry, true);
        flags.set(StatusFlag::kOverflow, true);
        return {0, flags};
    }
    const std::uint16_t result = static_cast<std::uint16_t>(lhs / rhs);
    FlagRegister flags;
    flags.set(StatusFlag::kZero, result == 0);
    flags.set(StatusFlag::kNegative, (result & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
    return {result, flags};
}

}  // namespace aurora
