#pragma once

#include "softcpu/instruction.hpp"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace softcpu {

// Result of an assembly operation
struct AssemblyResult {
  bool ok{false};                    // True if assembly was successful
  std::vector<std::uint8_t> bytes;   // The resulting machine code
  std::vector<std::string> messages; // Error messages or warnings
};

// Options for the assembler
struct AssemblerOptions {
  std::uint16_t origin{kResetVector}; // Starting address for the program
  bool emit_listing{
      false}; // Whether to generate a listing (not implemented yet)
};

// The Assembler class converts assembly source code into machine code
class Assembler {
public:
  // Assemble a source file from disk
  AssemblyResult assembleFile(const std::string &path,
                              const AssemblerOptions &options = {});

  // Assemble source code from a string
  AssemblyResult assembleString(const std::string &source,
                                const AssemblerOptions &options = {});

private:
  // Internal representation of a source line
  struct LineRecord {
    std::size_t number{0};
    std::string text;
  };

  // Information about a defined symbol (label or constant)
  struct SymbolInfo {
    std::uint16_t value{0};
    bool is_constant{false};
  };

  // Information about an operand that needs to be resolved later (e.g., forward
  // reference)
  struct PendingOperand {
    std::size_t location{0};
    std::string symbol;
    OperandType type{OperandType::Immediate};
    bool is_offset{false};
    int multiplier{1};
    std::uint8_t width{2};
  };

  // Parsed specification of an operand
  struct OperandSpec {
    OperandType type{OperandType::None};
    std::uint8_t reg{0};
    std::int32_t immediate{0};
    bool has_immediate{false};
    bool refers_symbol{false};
    std::string symbol;
    bool has_offset{false};
    std::int32_t offset{0};
    bool offset_refers_symbol{false};
    std::string offset_symbol;
    int offset_sign{1};
  };

  // Main assembly pass
  AssemblyResult assemble(const std::vector<LineRecord> &lines,
                          const AssemblerOptions &options);

  // Parse and process a single line of assembly
  bool parseLine(const LineRecord &line, std::uint16_t &location_counter,
                 std::vector<std::uint8_t> &program,
                 std::vector<PendingOperand> &pending);

  // Handle assembler directives (e.g., .org, .byte)
  bool encodeDirective(const LineRecord &line, std::string_view directive,
                       std::string_view remainder,
                       std::uint16_t &location_counter,
                       std::vector<std::uint8_t> &program,
                       std::vector<PendingOperand> &pending);

  // Encode a CPU instruction
  bool encodeInstruction(const LineRecord &line, std::string_view mnemonic,
                         std::string_view operands,
                         std::uint16_t &location_counter,
                         std::vector<std::uint8_t> &program,
                         std::vector<PendingOperand> &pending);

  // Parse a single operand string
  OperandSpec parseOperand(std::string_view token);

  // Parse a numeric value or symbol reference
  std::optional<std::int32_t> parseValue(std::string_view token) const;

  std::unordered_map<std::string, SymbolInfo> symbols_;
  std::vector<std::string> errors_;
  std::uint16_t origin_{0};
};

} // namespace softcpu
