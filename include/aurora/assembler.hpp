#pragma once

#include "aurora/instruction.hpp"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace aurora {

struct AssemblyResult {
    bool ok {false};
    std::vector<std::uint8_t> bytes;
    std::vector<std::string> messages;
};

struct AssemblerOptions {
    std::uint16_t origin {kResetVector};
    bool emit_listing {false};
};

class Assembler {
public:
    AssemblyResult assembleFile(const std::string& path, const AssemblerOptions& options = {});
    AssemblyResult assembleString(const std::string& source, const AssemblerOptions& options = {});

private:
    struct LineRecord {
        std::size_t number {0};
        std::string text;
    };

    struct SymbolInfo {
        std::uint16_t value {0};
        bool is_constant {false};
    };

    struct PendingOperand {
        std::size_t location {0};
        std::string symbol;
        OperandType type {OperandType::Immediate};
        bool is_offset {false};
        int multiplier {1};
        std::uint8_t width {2};
    };

    struct OperandSpec {
        OperandType type {OperandType::None};
        std::uint8_t reg {0};
        std::int32_t immediate {0};
        bool has_immediate {false};
        bool refers_symbol {false};
        std::string symbol;
        bool has_offset {false};
        std::int32_t offset {0};
        bool offset_refers_symbol {false};
        std::string offset_symbol;
        int offset_sign {1};
    };

    AssemblyResult assemble(const std::vector<LineRecord>& lines, const AssemblerOptions& options);
    bool parseLine(const LineRecord& line, std::uint16_t& location_counter,
                   std::vector<std::uint8_t>& program,
                   std::vector<PendingOperand>& pending);

    bool encodeDirective(const LineRecord& line, std::string_view directive,
                         std::string_view remainder, std::uint16_t& location_counter,
                         std::vector<std::uint8_t>& program,
                         std::vector<PendingOperand>& pending);

    bool encodeInstruction(const LineRecord& line, std::string_view mnemonic,
                           std::string_view operands, std::uint16_t& location_counter,
                           std::vector<std::uint8_t>& program,
                           std::vector<PendingOperand>& pending);

    OperandSpec parseOperand(std::string_view token);
    std::optional<std::int32_t> parseValue(std::string_view token) const;

    std::unordered_map<std::string, SymbolInfo> symbols_;
    std::vector<std::string> errors_;
    std::uint16_t origin_ {0};
};

}  // namespace aurora
