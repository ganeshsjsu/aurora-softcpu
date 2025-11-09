#include "aurora/control_unit.hpp"

#include "aurora/alu.hpp"
#include "aurora/bus.hpp"

#include <cstdio>
#include <iostream>

namespace aurora {
namespace {
constexpr std::uint8_t kStackRegisterIndex = static_cast<std::uint8_t>(kRegisterCount - 1);
constexpr std::uint16_t kPortConsoleData = 0;
constexpr std::uint16_t kPortConsoleStatus = 1;
constexpr std::uint16_t kPortTimerControl = 2;
constexpr std::uint16_t kPortTimerCounter = 3;
constexpr std::uint16_t kPortLedValue = 4;

std::uint16_t readRegister(const RegisterFile& regs, std::uint8_t index) {
    if (index >= kRegisterCount) {
        return 0;
    }
    if (index == kStackRegisterIndex) {
        return regs.sp;
    }
    return regs.gpr[index];
}

void writeRegister(RegisterFile& regs, std::uint8_t index, std::uint16_t value) {
    if (index >= kRegisterCount) {
        return;
    }
    if (index == kStackRegisterIndex) {
        regs.sp = value;
        regs.gpr[index] = value;
        return;
    }
    regs.gpr[index] = value;
}

std::uint16_t portToAddress(std::uint16_t port_id) {
    switch (port_id) {
        case kPortConsoleData:
            return 0xFF00;
        case kPortConsoleStatus:
            return 0xFF01;
        case kPortTimerControl:
            return 0xFF12;
        case kPortTimerCounter:
            return 0xFF10;
        case kPortLedValue:
            return 0xFF20;
        default:
            return static_cast<std::uint16_t>(0xFF00 + port_id);
    }
}

void updateZN(FlagRegister& flags, std::uint16_t value) {
    flags.set(StatusFlag::kZero, value == 0);
    flags.set(StatusFlag::kNegative, (value & 0x8000) != 0);
    flags.set(StatusFlag::kCarry, false);
    flags.set(StatusFlag::kOverflow, false);
}

}  // namespace

ControlUnit::ControlUnit(Bus& bus, RegisterFile& registers, ALU& alu)
    : bus_(bus), registers_(registers), alu_(alu) {}

void ControlUnit::reset() {
    registers_.reset();
}

bool ControlUnit::step(bool trace) {
    const auto instruction = fetchInstruction();
    if (trace) {
        std::printf("%04X %-5s\n", instruction.address, opcodeName(instruction.opcode));
    }
    return execute(instruction, trace);
}

DecodedInstruction ControlUnit::fetchInstruction() {
    DecodedInstruction decoded;
    decoded.address = registers_.pc;
    std::uint16_t pc = registers_.pc;
    InstructionWord word{};
    word.opcode = bus_.read8(pc++);
    word.operand_a = bus_.read8(pc++);
    word.operand_b = bus_.read8(pc++);
    word.modifier = bus_.read8(pc++);
    decoded.opcode = static_cast<Opcode>(word.opcode);
    decoded.modifier = word.modifier;

    const auto descriptor_a = decodeOperand(word.operand_a);
    const auto descriptor_b = decodeOperand(word.operand_b);
    if (descriptor_a.type != OperandType::None) {
        decoded.operand_a = resolveOperand(descriptor_a, pc);
    }
    if (descriptor_b.type != OperandType::None) {
        decoded.operand_b = resolveOperand(descriptor_b, pc);
    }

    decoded.size_bytes = static_cast<std::uint16_t>(pc - decoded.address);
    registers_.pc = pc;
    return decoded;
}

Operand ControlUnit::resolveOperand(const OperandDescriptor& descriptor, std::uint16_t& pc) {
    Operand operand;
    operand.type = descriptor.type;
    operand.reg = descriptor.payload;
    switch (descriptor.type) {
        case OperandType::Register:
        case OperandType::RegisterIndirect:
            operand.reg &= 0x07;
            break;
        case OperandType::RegisterIndexed:
            operand.reg &= 0x07;
            operand.offset = static_cast<std::int16_t>(bus_.read16(pc));
            operand.has_offset = true;
            pc = static_cast<std::uint16_t>(pc + 2);
            break;
        case OperandType::Immediate:
        case OperandType::Absolute:
            operand.value = bus_.read16(pc);
            pc = static_cast<std::uint16_t>(pc + 2);
            break;
        case OperandType::Port:
            operand.value = descriptor.payload;
            break;
        case OperandType::None:
        default:
            break;
    }
    return operand;
}

std::uint16_t readOperandValue(Bus& bus, const RegisterFile& regs, const Operand& operand) {
    switch (operand.type) {
        case OperandType::Register:
            return readRegister(regs, operand.reg);
        case OperandType::Immediate:
            return operand.value;
        case OperandType::Absolute:
            return bus.read16(operand.value);
        case OperandType::RegisterIndirect: {
            const auto address = readRegister(regs, operand.reg);
            return bus.read16(address);
        }
        case OperandType::RegisterIndexed: {
            const auto base = readRegister(regs, operand.reg);
            const auto address = static_cast<std::uint16_t>(base + operand.offset);
            return bus.read16(address);
        }
        default:
            return operand.value;
    }
}

void writeOperandValue(Bus& bus, RegisterFile& regs, const Operand& operand, std::uint16_t value) {
    switch (operand.type) {
        case OperandType::Register:
            writeRegister(regs, operand.reg, value);
            break;
        case OperandType::Absolute:
            bus.write16(operand.value, value);
            break;
        case OperandType::RegisterIndirect: {
            const auto address = readRegister(regs, operand.reg);
            bus.write16(address, value);
            break;
        }
        case OperandType::RegisterIndexed: {
            const auto base = readRegister(regs, operand.reg);
            const auto address = static_cast<std::uint16_t>(base + operand.offset);
            bus.write16(address, value);
            break;
        }
        default:
            break;
    }
}

void push(Bus& bus, RegisterFile& regs, std::uint16_t value) {
    const auto new_sp = static_cast<std::uint16_t>(regs.sp - 2);
    bus.write16(new_sp, value);
    writeRegister(regs, kStackRegisterIndex, new_sp);
}

std::uint16_t pop(Bus& bus, RegisterFile& regs) {
    const auto value = bus.read16(regs.sp);
    const auto new_sp = static_cast<std::uint16_t>(regs.sp + 2);
    writeRegister(regs, kStackRegisterIndex, new_sp);
    return value;
}

bool ControlUnit::execute(const DecodedInstruction& inst, bool) {
    switch (inst.opcode) {
        case Opcode::NOP:
            return true;
        case Opcode::HALT:
            return false;
        case Opcode::LDI: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_b);
            writeOperandValue(bus_, registers_, inst.operand_a, value);
            updateZN(registers_.flags, value);
            return true;
        }
        case Opcode::MOV: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_b);
            writeOperandValue(bus_, registers_, inst.operand_a, value);
            return true;
        }
        case Opcode::LOAD: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_b);
            writeOperandValue(bus_, registers_, inst.operand_a, value);
            return true;
        }
        case Opcode::STORE: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_a);
            writeOperandValue(bus_, registers_, inst.operand_b, value);
            return true;
        }
        case Opcode::ADD:
        case Opcode::ADDI: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.add(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::SUB:
        case Opcode::SUBI: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.sub(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::MUL: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.mul(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::DIV: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.divide(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::AND: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.bit_and(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::OR: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.bit_or(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::XOR: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            const auto result = alu_.bit_xor(lhs, rhs);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::NOT: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_a);
            const auto result = alu_.bit_not(value);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::SHL: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_a);
            const auto shift = static_cast<std::uint8_t>(readOperandValue(bus_, registers_, inst.operand_b) & 0xFF);
            const auto result = alu_.shl(value, shift);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::SHR: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_a);
            const auto shift = static_cast<std::uint8_t>(readOperandValue(bus_, registers_, inst.operand_b) & 0xFF);
            const auto result = alu_.shr(value, shift);
            writeOperandValue(bus_, registers_, inst.operand_a, result.value);
            registers_.flags = result.flags;
            return true;
        }
        case Opcode::CMP: {
            const auto lhs = readOperandValue(bus_, registers_, inst.operand_a);
            const auto rhs = readOperandValue(bus_, registers_, inst.operand_b);
            registers_.flags = alu_.sub(lhs, rhs).flags;
            return true;
        }
        case Opcode::JMP: {
            const auto target = readOperandValue(bus_, registers_, inst.operand_a);
            registers_.pc = target;
            return true;
        }
        case Opcode::JZ: {
            if (registers_.flags.test(StatusFlag::kZero)) {
                registers_.pc = readOperandValue(bus_, registers_, inst.operand_a);
            }
            return true;
        }
        case Opcode::JNZ: {
            if (!registers_.flags.test(StatusFlag::kZero)) {
                registers_.pc = readOperandValue(bus_, registers_, inst.operand_a);
            }
            return true;
        }
        case Opcode::JN: {
            if (registers_.flags.test(StatusFlag::kNegative)) {
                registers_.pc = readOperandValue(bus_, registers_, inst.operand_a);
            }
            return true;
        }
        case Opcode::JC: {
            if (registers_.flags.test(StatusFlag::kCarry)) {
                registers_.pc = readOperandValue(bus_, registers_, inst.operand_a);
            }
            return true;
        }
        case Opcode::CALL: {
            const auto target = readOperandValue(bus_, registers_, inst.operand_a);
            push(bus_, registers_, registers_.pc);
            registers_.pc = target;
            return true;
        }
        case Opcode::RET: {
            registers_.pc = pop(bus_, registers_);
            return true;
        }
        case Opcode::PUSH: {
            const auto value = readOperandValue(bus_, registers_, inst.operand_a);
            push(bus_, registers_, value);
            return true;
        }
        case Opcode::POP: {
            const auto value = pop(bus_, registers_);
            writeOperandValue(bus_, registers_, inst.operand_a, value);
            return true;
        }
        case Opcode::OUT: {
            const auto port = portToAddress(inst.operand_a.value);
            const auto value = static_cast<std::uint8_t>(readOperandValue(bus_, registers_, inst.operand_b) & 0xFF);
            bus_.write8(port, value);
            return true;
        }
        case Opcode::IN: {
            const auto port = portToAddress(inst.operand_b.value);
            const auto value = bus_.read8(port);
            writeOperandValue(bus_, registers_, inst.operand_a, value);
            return true;
        }
        case Opcode::ADJSP: {
            const auto delta = static_cast<std::int16_t>(readOperandValue(bus_, registers_, inst.operand_a));
            const auto new_sp = static_cast<std::uint16_t>(registers_.sp + delta);
            writeRegister(registers_, kStackRegisterIndex, new_sp);
            return true;
        }
        case Opcode::SYS: {
            const auto code = readOperandValue(bus_, registers_, inst.operand_a);
            switch (code) {
                case 0:
                    break;
                case 1:
                    std::cout << "\n";
                    break;
                case 2:
                    std::cout << "[R0=" << readRegister(registers_, 0) << "]" << std::endl;
                    break;
                default:
                    break;
            }
            return true;
        }
        default:
            std::fprintf(stderr, "Unknown opcode %02X at %04X\n", static_cast<int>(inst.opcode), inst.address);
            return false;
    }
}

}  // namespace aurora
