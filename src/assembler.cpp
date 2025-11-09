#include "aurora/assembler.hpp"

#include "aurora/utils.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace aurora {
namespace {

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string toUpper(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return text;
}

std::optional<std::uint8_t> parseRegister(std::string_view token) {
    const auto lower = toLower(std::string(token));
    if (lower == "sp" || lower == "r7") {
        return 7;
    }
    if (lower == "pc") {
        return std::nullopt;
    }
    if (lower.size() >= 2 && lower[0] == 'r') {
        const auto digits = lower.substr(1);
        if (std::all_of(digits.begin(), digits.end(), ::isdigit)) {
            const int index = std::stoi(digits);
            if (index >= 0 && index < static_cast<int>(kRegisterCount)) {
                return static_cast<std::uint8_t>(index);
            }
        }
    }
    return std::nullopt;
}

std::optional<std::uint8_t> parsePort(std::string_view token) {
    const auto lower = toLower(std::string(token));
    static const std::unordered_map<std::string, std::uint8_t> kPorts {
        {"console", 0},
        {"console_status", 1},
        {"timer_control", 2},
        {"timer_counter", 3},
        {"leds", 4}
    };

    if (lower.rfind("port", 0) == 0) {
        std::string_view remainder(lower);
        remainder.remove_prefix(4);
        if (!remainder.empty() && (remainder.front() == ':' || remainder.front() == '.')) {
            remainder.remove_prefix(1);
        }
        if (auto it = kPorts.find(std::string(remainder)); it != kPorts.end()) {
            return it->second;
        }
        if (!remainder.empty() && std::all_of(remainder.begin(), remainder.end(), ::isdigit)) {
            int value = std::stoi(std::string(remainder));
            if (value >= 0 && value <= 255) {
                return static_cast<std::uint8_t>(value);
            }
        }
    }
    return std::nullopt;
}

bool isIdentifier(std::string_view token) {
    if (token.empty()) {
        return false;
    }
    if (!std::isalpha(static_cast<unsigned char>(token.front())) && token.front() != '_') {
        return false;
    }
    return std::all_of(token.begin(), token.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_';
    });
}

struct OpcodeInfo {
    Opcode opcode;
    std::size_t operands;
};

const std::unordered_map<std::string, OpcodeInfo> kOpcodeTable {
    {"NOP", {Opcode::NOP, 0}},
    {"HALT", {Opcode::HALT, 0}},
    {"LDI", {Opcode::LDI, 2}},
    {"MOV", {Opcode::MOV, 2}},
    {"LOAD", {Opcode::LOAD, 2}},
    {"STORE", {Opcode::STORE, 2}},
    {"ADD", {Opcode::ADD, 2}},
    {"ADDI", {Opcode::ADDI, 2}},
    {"SUB", {Opcode::SUB, 2}},
    {"SUBI", {Opcode::SUBI, 2}},
    {"MUL", {Opcode::MUL, 2}},
    {"DIV", {Opcode::DIV, 2}},
    {"AND", {Opcode::AND, 2}},
    {"OR", {Opcode::OR, 2}},
    {"XOR", {Opcode::XOR, 2}},
    {"NOT", {Opcode::NOT, 1}},
    {"SHL", {Opcode::SHL, 2}},
    {"SHR", {Opcode::SHR, 2}},
    {"CMP", {Opcode::CMP, 2}},
    {"JMP", {Opcode::JMP, 1}},
    {"JZ", {Opcode::JZ, 1}},
    {"JNZ", {Opcode::JNZ, 1}},
    {"JN", {Opcode::JN, 1}},
    {"JC", {Opcode::JC, 1}},
    {"CALL", {Opcode::CALL, 1}},
    {"RET", {Opcode::RET, 0}},
    {"PUSH", {Opcode::PUSH, 1}},
    {"POP", {Opcode::POP, 1}},
    {"OUT", {Opcode::OUT, 2}},
    {"IN", {Opcode::IN, 2}},
    {"ADJSP", {Opcode::ADJSP, 1}},
    {"SYS", {Opcode::SYS, 1}}
};

}  // namespace

AssemblyResult Assembler::assembleFile(const std::string& path, const AssemblerOptions& options) {
    std::ifstream input(path);
    if (!input) {
        return {false, {}, {"unable to open " + path}};
    }

    std::vector<LineRecord> lines;
    std::string line;
    std::size_t number = 1;
    while (std::getline(input, line)) {
        lines.push_back(LineRecord{number++, line});
    }
    return assemble(lines, options);
}

AssemblyResult Assembler::assembleString(const std::string& source, const AssemblerOptions& options) {
    std::istringstream stream(source);
    std::vector<LineRecord> lines;
    std::string line;
    std::size_t number = 1;
    while (std::getline(stream, line)) {
        lines.push_back(LineRecord{number++, line});
    }
    return assemble(lines, options);
}

AssemblyResult Assembler::assemble(const std::vector<LineRecord>& lines, const AssemblerOptions& options) {
    symbols_.clear();
    errors_.clear();
    origin_ = options.origin;
    std::uint16_t location_counter = origin_;
    std::vector<std::uint8_t> program;
    std::vector<PendingOperand> pending;

    symbols_["IO_CONSOLE_DATA"] = {0xFF00, true};
    symbols_["IO_CONSOLE_STATUS"] = {0xFF01, true};
    symbols_["IO_TIMER_COUNTER"] = {0xFF10, true};
    symbols_["IO_TIMER_CONTROL"] = {0xFF12, true};
    symbols_["IO_LED"] = {0xFF20, true};

    for (const auto& line : lines) {
        parseLine(line, location_counter, program, pending);
    }

    // Resolve pending operands
    for (const auto& entry : pending) {
        auto it = symbols_.find(entry.symbol);
        if (it == symbols_.end()) {
            errors_.push_back("unresolved symbol: " + entry.symbol);
            continue;
        }
        auto value = it->second.value;
        if (entry.is_offset) {
            std::int32_t signed_value = static_cast<std::int32_t>(value) * entry.multiplier;
            value = static_cast<std::uint16_t>(signed_value & 0xFFFF);
        }
        if (entry.location + entry.width - 1 >= program.size()) {
            errors_.push_back("invalid patch location for symbol: " + entry.symbol);
            continue;
        }
        if (entry.width == 1) {
            program[entry.location] = static_cast<std::uint8_t>(value & 0xFF);
        } else {
            program[entry.location] = static_cast<std::uint8_t>(value & 0xFF);
            program[entry.location + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        }
    }

    AssemblyResult result;
    result.ok = errors_.empty();
    result.bytes = program;
    result.messages = errors_;
    return result;
}

bool Assembler::parseLine(const LineRecord& line, std::uint16_t& location_counter,
                          std::vector<std::uint8_t>& program,
                          std::vector<PendingOperand>& pending) {
    std::string text = util::trim(line.text);

    auto semicolon_pos = text.find(';');
    auto slash_pos = text.find("//");
    std::size_t comment_pos = std::string::npos;
    if (semicolon_pos != std::string::npos) {
        comment_pos = semicolon_pos;
    }
    if (slash_pos != std::string::npos) {
        comment_pos = std::min(comment_pos, slash_pos);
    }
    if (comment_pos != std::string::npos) {
        text = text.substr(0, comment_pos);
    }
    text = util::trim(text);
    if (text.empty()) {
        return true;
    }

    auto colon_pos = text.find(':');
    if (colon_pos != std::string::npos) {
        auto label = util::trim(text.substr(0, colon_pos));
        if (!label.empty()) {
            symbols_[label] = {location_counter, false};
        }
        text = util::trim(text.substr(colon_pos + 1));
        if (text.empty()) {
            return true;
        }
    }

    if (text[0] == '.') {
        const auto space = text.find_first_of(" \t");
        const auto directive = text.substr(0, space);
        const auto remainder = space == std::string::npos ? std::string_view{} : std::string_view(text).substr(space + 1);
        return encodeDirective(line, directive, remainder, location_counter, program, pending);
    }

    std::istringstream stream(text);
    std::string mnemonic;
    stream >> mnemonic;
    auto operands = std::string();
    std::getline(stream, operands);
    operands = util::trim(operands);
    return encodeInstruction(line, mnemonic, operands, location_counter, program, pending);
}

namespace {
void writeWord(std::vector<std::uint8_t>& program, std::uint16_t& location,
               std::uint16_t origin, std::uint16_t value) {
    const auto index = static_cast<std::size_t>(location - origin);
    if (program.size() <= index + 1) {
        program.resize(index + 2, 0);
    }
    program[index] = static_cast<std::uint8_t>(value & 0xFF);
    program[index + 1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    location = static_cast<std::uint16_t>(location + 2);
}

void writeByte(std::vector<std::uint8_t>& program, std::uint16_t& location,
               std::uint16_t origin, std::uint8_t value) {
    const auto index = static_cast<std::size_t>(location - origin);
    if (program.size() <= index) {
        program.resize(index + 1, 0);
    }
    program[index] = value;
    location = static_cast<std::uint16_t>(location + 1);
}

std::string parseStringLiteral(std::string_view text) {
    std::string result;
    bool escape = false;
    for (char ch : text) {
        if (!escape && ch == '\\') {
            escape = true;
            continue;
        }
        if (escape) {
            switch (ch) {
                case 'n': result.push_back('\n'); break;
                case 't': result.push_back('\t'); break;
                case 'r': result.push_back('\r'); break;
                case '\\': result.push_back('\\'); break;
                case '"': result.push_back('"'); break;
                default: result.push_back(ch); break;
            }
            escape = false;
            continue;
        }
        if (ch != '"') {
            result.push_back(ch);
        }
    }
    return result;
}
}  // namespace

bool Assembler::encodeDirective(const LineRecord& line, std::string_view directive,
                                std::string_view remainder, std::uint16_t& location_counter,
                                std::vector<std::uint8_t>& program,
                                std::vector<PendingOperand>& pending) {
    const auto name = toLower(std::string(directive));
    if (name == ".org") {
        if (auto value = parseValue(remainder)) {
            if (*value < origin_) {
                errors_.push_back("line " + std::to_string(line.number) + ": .org before origin not supported");
                return false;
            }
            location_counter = static_cast<std::uint16_t>(*value & 0xFFFF);
            return true;
        }
        errors_.push_back("line " + std::to_string(line.number) + ": invalid .org argument");
        return false;
    } else if (name == ".word") {
        const auto values = util::splitOperands(remainder);
        for (const auto& val_token : values) {
            std::string cleaned = util::trim(val_token);
            if (!cleaned.empty() && cleaned.front() == '#') {
                cleaned.erase(cleaned.begin());
            }
            auto value = parseValue(cleaned);
            const auto index = static_cast<std::size_t>(location_counter - origin_);
            if (!value) {
                pending.push_back({index, cleaned, OperandType::Immediate, false, 1, 2});
                writeWord(program, location_counter, origin_, 0);
            } else {
                writeWord(program, location_counter, origin_, static_cast<std::uint16_t>(*value & 0xFFFF));
            }
        }
        return true;
    } else if (name == ".byte") {
        const auto values = util::splitOperands(remainder);
        for (const auto& val_token : values) {
            std::string cleaned = util::trim(val_token);
            if (!cleaned.empty() && cleaned.front() == '#') {
                cleaned.erase(cleaned.begin());
            }
            auto value = parseValue(cleaned);
            const auto index = static_cast<std::size_t>(location_counter - origin_);
            if (!value) {
                pending.push_back({index, cleaned, OperandType::Immediate, false, 1, 1});
                writeByte(program, location_counter, origin_, 0);
            } else {
                writeByte(program, location_counter, origin_, static_cast<std::uint8_t>(*value & 0xFF));
            }
        }
        return true;
    } else if (name == ".ascii" || name == ".asciiz") {
        const auto trimmed = util::trim(remainder);
        if (trimmed.size() < 2 || trimmed.front() != '"' || trimmed.back() != '"') {
            errors_.push_back("line " + std::to_string(line.number) + ": invalid string literal");
            return false;
        }
        const auto text = parseStringLiteral(trimmed);
        for (char ch : text) {
            writeByte(program, location_counter, origin_, static_cast<std::uint8_t>(ch));
        }
        if (name == ".asciiz") {
            writeByte(program, location_counter, origin_, 0);
        }
        return true;
    } else if (name == ".fill") {
        const auto parts = util::splitOperands(remainder);
        if (parts.size() != 2) {
            errors_.push_back("line " + std::to_string(line.number) + ": .fill expects count,value");
            return false;
        }
        auto count = parseValue(parts[0]);
        auto pattern = parseValue(parts[1]);
        if (!count || !pattern) {
            errors_.push_back("line " + std::to_string(line.number) + ": invalid .fill argument");
            return false;
        }
        for (int i = 0; i < *count; ++i) {
            writeByte(program, location_counter, origin_, static_cast<std::uint8_t>(*pattern & 0xFF));
        }
        return true;
    } else if (name == ".const" || name == ".equ") {
        auto parts = util::splitOperands(remainder);
        if (parts.size() == 1) {
            std::istringstream stream(parts.front());
            std::string left;
            std::string right;
            stream >> left >> right;
            if (!left.empty() && !right.empty()) {
                parts = {left, right};
            }
        }
        if (parts.size() != 2) {
            errors_.push_back("line " + std::to_string(line.number) + ": .const name, value");
            return false;
        }
        const auto symbol = util::trim(parts[0]);
        const auto value = parseValue(parts[1]);
        if (!value) {
            errors_.push_back("line " + std::to_string(line.number) + ": invalid constant value");
            return false;
        }
        symbols_[symbol] = {static_cast<std::uint16_t>(*value & 0xFFFF), true};
        return true;
    }

    errors_.push_back("line " + std::to_string(line.number) + ": unknown directive " + std::string(directive));
    return false;
}

bool Assembler::encodeInstruction(const LineRecord& line, std::string_view mnemonic,
                                  std::string_view operands, std::uint16_t& location_counter,
                                  std::vector<std::uint8_t>& program,
                                  std::vector<PendingOperand>& pending) {
    const auto lookup = kOpcodeTable.find(toUpper(std::string(mnemonic)));
    if (lookup == kOpcodeTable.end()) {
        errors_.push_back("line " + std::to_string(line.number) + ": unknown mnemonic " + std::string(mnemonic));
        return false;
    }
    const auto opcode_info = lookup->second;
    std::vector<std::string> operand_tokens = util::splitOperands(operands);
    operand_tokens.erase(std::remove_if(operand_tokens.begin(), operand_tokens.end(), [](const std::string& s) { return s.empty(); }), operand_tokens.end());
    if (operand_tokens.size() != opcode_info.operands) {
        errors_.push_back("line " + std::to_string(line.number) + ": expected " + std::to_string(opcode_info.operands) + " operands");
        return false;
    }

    OperandSpec spec_a;
    OperandSpec spec_b;
    if (!operand_tokens.empty()) {
        spec_a = parseOperand(operand_tokens[0]);
    }
    if (operand_tokens.size() > 1) {
        spec_b = parseOperand(operand_tokens[1]);
    }

    InstructionWord word{};
    word.opcode = static_cast<std::uint8_t>(opcode_info.opcode);
    word.operand_a = encodeOperand(spec_a.type, spec_a.reg);
    word.operand_b = encodeOperand(spec_b.type, spec_b.reg);

    writeByte(program, location_counter, origin_, word.opcode);
    writeByte(program, location_counter, origin_, word.operand_a);
    writeByte(program, location_counter, origin_, word.operand_b);
    writeByte(program, location_counter, origin_, word.modifier);

    auto emitExtended = [&](const OperandSpec& spec, bool is_offset) {
        if (spec.type == OperandType::Immediate || spec.type == OperandType::Absolute ||
            (spec.type == OperandType::RegisterIndexed && is_offset)) {
            std::uint16_t value = 0;
            if (is_offset && spec.has_offset && !spec.offset_refers_symbol) {
                value = static_cast<std::uint16_t>(spec.offset & 0xFFFF);
            } else if (!is_offset && spec.has_immediate && !spec.refers_symbol) {
                value = static_cast<std::uint16_t>(spec.immediate & 0xFFFF);
            }
            const auto index = static_cast<std::size_t>(location_counter - origin_);
            writeWord(program, location_counter, origin_, value);
            if ((is_offset && spec.offset_refers_symbol) || (!is_offset && spec.refers_symbol)) {
                PendingOperand entry;
                entry.location = index;
                entry.symbol = is_offset ? spec.offset_symbol : spec.symbol;
                entry.type = spec.type;
                entry.is_offset = is_offset;
                entry.multiplier = is_offset ? spec.offset_sign : 1;
                entry.width = 2;
                pending.push_back(entry);
            }
        }
    };

    emitExtended(spec_a, false);
    emitExtended(spec_b, false);
    if (spec_a.type == OperandType::RegisterIndexed && spec_a.has_offset) {
        emitExtended(spec_a, true);
    }
    if (spec_b.type == OperandType::RegisterIndexed && spec_b.has_offset) {
        emitExtended(spec_b, true);
    }

    return true;
}

Assembler::OperandSpec Assembler::parseOperand(std::string_view token) {
    OperandSpec spec;
    auto text = util::trim(token);
    if (text.empty()) {
        return spec;
    }

    if (auto port = parsePort(text)) {
        spec.type = OperandType::Port;
        spec.has_immediate = true;
        spec.immediate = *port;
        spec.reg = *port;
        return spec;
    }

    if (text.front() == '[' && text.back() == ']') {
        auto inner = util::trim(text.substr(1, text.size() - 2));
        auto plus_pos = inner.find_first_of("+-");
        auto base_token = util::trim(inner.substr(0, plus_pos));
        if (auto reg = parseRegister(base_token)) {
            if (plus_pos == std::string::npos) {
                spec.type = OperandType::RegisterIndirect;
                spec.reg = *reg;
                return spec;
            }
            spec.type = OperandType::RegisterIndexed;
            spec.reg = *reg;
            spec.has_offset = true;
            const auto offset_token = util::trim(inner.substr(plus_pos));
            spec.offset_sign = offset_token.front() == '-' ? -1 : 1;
            const auto value_token = util::trim(offset_token.substr(1));
            const auto value = parseValue(value_token);
            if (value) {
                spec.offset = static_cast<std::int32_t>(*value) * spec.offset_sign;
            } else {
                spec.offset_refers_symbol = true;
                spec.offset_symbol = std::string(value_token);
            }
            return spec;
        }
        spec.type = OperandType::Absolute;
        const auto value = parseValue(inner);
        if (value) {
            spec.immediate = *value;
            spec.has_immediate = true;
        } else {
            spec.refers_symbol = true;
            spec.symbol = std::string(inner);
        }
        return spec;
    }

    if (text.front() == '#') {
        spec.type = OperandType::Immediate;
        const auto value = parseValue(text.substr(1));
        if (value) {
            spec.immediate = *value;
            spec.has_immediate = true;
        } else {
            spec.refers_symbol = true;
            spec.symbol = std::string(text.substr(1));
        }
        return spec;
    }

    if (auto reg = parseRegister(text)) {
        spec.type = OperandType::Register;
        spec.reg = *reg;
        return spec;
    }

    spec.type = OperandType::Immediate;
    const auto value = parseValue(text);
    if (value) {
        spec.immediate = *value;
        spec.has_immediate = true;
    } else if (isIdentifier(text)) {
        spec.refers_symbol = true;
        spec.symbol = std::string(text);
    }
    return spec;
}

std::optional<std::int32_t> Assembler::parseValue(std::string_view token) const {
    const auto trimmed = util::trim(token);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    if (auto number = util::parseNumber(trimmed)) {
        return number;
    }
    if (auto it = symbols_.find(std::string(trimmed)); it != symbols_.end()) {
        return it->second.value;
    }
    return std::nullopt;
}

}  // namespace aurora
