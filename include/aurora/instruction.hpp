#pragma once

#include "aurora/common.hpp"

#include <cstddef>
#include <cstdint>

namespace aurora {

enum class Opcode : std::uint8_t {
    NOP = 0x00,
    HALT = 0x01,
    LDI = 0x02,
    MOV = 0x03,
    LOAD = 0x04,
    STORE = 0x05,
    ADD = 0x06,
    ADDI = 0x07,
    SUB = 0x08,
    SUBI = 0x09,
    MUL = 0x0A,
    DIV = 0x0B,
    AND = 0x0C,
    OR = 0x0D,
    XOR = 0x0E,
    NOT = 0x0F,
    SHL = 0x10,
    SHR = 0x11,
    CMP = 0x12,
    JMP = 0x13,
    JZ = 0x14,
    JNZ = 0x15,
    JN = 0x16,
    JC = 0x17,
    CALL = 0x18,
    RET = 0x19,
    PUSH = 0x1A,
    POP = 0x1B,
    OUT = 0x1C,
    IN = 0x1D,
    ADJSP = 0x1E,
    SYS = 0x1F
};

enum class OperandType : std::uint8_t {
    None = 0,
    Register = 1,
    RegisterIndirect = 2,
    RegisterIndexed = 3,
    Immediate = 4,
    Absolute = 5,
    Port = 6
};

struct OperandDescriptor {
    OperandType type {OperandType::None};
    std::uint8_t payload {0};
};

struct Operand {
    OperandType type {OperandType::None};
    std::uint8_t reg {0};
    std::uint16_t value {0};
    std::int16_t offset {0};
    bool has_offset {false};
};

constexpr std::uint8_t encodeOperand(OperandType type, std::uint8_t payload = 0) {
    return (static_cast<std::uint8_t>(type) << 5) | (payload & 0x1F);
}

constexpr OperandDescriptor decodeOperand(std::uint8_t raw) {
    return OperandDescriptor{
        static_cast<OperandType>((raw >> 5) & 0x07),
        static_cast<std::uint8_t>(raw & 0x1F)
    };
}

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

inline const char* opcodeName(Opcode opcode) {
    switch (opcode) {
        case Opcode::NOP: return "NOP";
        case Opcode::HALT: return "HALT";
        case Opcode::LDI: return "LDI";
        case Opcode::MOV: return "MOV";
        case Opcode::LOAD: return "LOAD";
        case Opcode::STORE: return "STORE";
        case Opcode::ADD: return "ADD";
        case Opcode::ADDI: return "ADDI";
        case Opcode::SUB: return "SUB";
        case Opcode::SUBI: return "SUBI";
        case Opcode::MUL: return "MUL";
        case Opcode::DIV: return "DIV";
        case Opcode::AND: return "AND";
        case Opcode::OR: return "OR";
        case Opcode::XOR: return "XOR";
        case Opcode::NOT: return "NOT";
        case Opcode::SHL: return "SHL";
        case Opcode::SHR: return "SHR";
        case Opcode::CMP: return "CMP";
        case Opcode::JMP: return "JMP";
        case Opcode::JZ: return "JZ";
        case Opcode::JNZ: return "JNZ";
        case Opcode::JN: return "JN";
        case Opcode::JC: return "JC";
        case Opcode::CALL: return "CALL";
        case Opcode::RET: return "RET";
        case Opcode::PUSH: return "PUSH";
        case Opcode::POP: return "POP";
        case Opcode::OUT: return "OUT";
        case Opcode::IN: return "IN";
        case Opcode::ADJSP: return "ADJSP";
        case Opcode::SYS: return "SYS";
    }
    return "?";
}

}  // namespace aurora
