#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace aurora::util {

std::string trim(std::string_view text);
std::vector<std::string> splitOperands(std::string_view text);
std::optional<std::int32_t> parseNumber(std::string_view text);
std::vector<std::uint8_t> readBinaryFile(const std::string& path);
bool writeBinaryFile(const std::string& path, const std::vector<std::uint8_t>& data);

}  // namespace aurora::util
