#include "softcpu/utils.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <sstream>
#include <system_error>

namespace softcpu::util {

std::string trim(std::string_view text) {
  auto begin = text.begin();
  auto end = text.end();
  while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) {
    ++begin;
  }
  while (end != begin) {
    auto prev = end;
    --prev;
    if (!std::isspace(static_cast<unsigned char>(*prev))) {
      break;
    }
    end = prev;
  }
  return std::string(begin, end);
}

std::vector<std::string> splitOperands(std::string_view text) {
  std::vector<std::string> parts;
  std::string current;
  bool in_string = false;
  for (char ch : text) {
    if (ch == '"') {
      in_string = !in_string;
      current.push_back(ch);
      continue;
    }
    if (ch == ',' && !in_string) {
      parts.push_back(trim(current));
      current.clear();
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty()) {
    parts.push_back(trim(current));
  }
  return parts;
}

std::optional<std::int32_t> parseNumber(std::string_view token) {
  if (token.empty()) {
    return std::nullopt;
  }
  std::string text = trim(token);
  int base = 10;
  if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
    base = 16;
    text = text.substr(2);
  } else if (text.size() > 2 && text[0] == '0' &&
             (text[1] == 'b' || text[1] == 'B')) {
    base = 2;
    text = text.substr(2);
  } else if (text.size() > 1 && text[0] == '$') {
    base = 16;
    text = text.substr(1);
  }

  if (text.size() >= 3 && text.front() == '\'' && text.back() == '\'' &&
      text.size() == 3) {
    return static_cast<std::int32_t>(text[1]);
  }

  std::int32_t value = 0;
  const auto result =
      std::from_chars(text.data(), text.data() + text.size(), value, base);
  if (result.ec != std::errc()) {
    return std::nullopt;
  }
  return value;
}

std::vector<std::uint8_t> readBinaryFile(const std::string &path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return {};
  }
  std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(input)),
                                 std::istreambuf_iterator<char>());
  return data;
}

bool writeBinaryFile(const std::string &path,
                     const std::vector<std::uint8_t> &data) {
  std::ofstream output(path, std::ios::binary);
  if (!output) {
    return false;
  }
  output.write(reinterpret_cast<const char *>(data.data()),
               static_cast<std::streamsize>(data.size()));
  return output.good();
}

} // namespace softcpu::util
