#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace softcpu::util {

// Remove leading and trailing whitespace from a string
std::string trim(std::string_view text);

// Split a comma-separated string of operands into a vector of strings
std::vector<std::string> splitOperands(std::string_view text);

// Parse a number from a string, supporting decimal and hexadecimal (0x prefix)
// formats
std::optional<std::int32_t> parseNumber(std::string_view text);

// Read the entire contents of a binary file into a vector of bytes
std::vector<std::uint8_t> readBinaryFile(const std::string &path);

// Write a vector of bytes to a binary file
bool writeBinaryFile(const std::string &path,
                     const std::vector<std::uint8_t> &data);

} // namespace softcpu::util
