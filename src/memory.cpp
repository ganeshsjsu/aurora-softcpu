#include "softcpu/memory.hpp"

#include <algorithm>
#include <stdexcept>

namespace softcpu {

namespace {
// Helper functions for byte manipulation
constexpr std::uint8_t hi(std::uint16_t value) {
  return static_cast<std::uint8_t>((value >> 8) & 0xFF);
}
constexpr std::uint8_t lo(std::uint16_t value) {
  return static_cast<std::uint8_t>(value & 0xFF);
}
} // namespace

Memory::Memory() { bytes_.fill(0); }

std::uint8_t Memory::read8(std::uint16_t address) const {
  return bytes_[address];
}

std::uint16_t Memory::read16(std::uint16_t address) const {
  // Little-endian read
  const std::uint8_t low = read8(address);
  const std::uint8_t high = read8(static_cast<std::uint16_t>(address + 1));
  return static_cast<std::uint16_t>((static_cast<std::uint16_t>(high) << 8) |
                                    low);
}

void Memory::write8(std::uint16_t address, std::uint8_t value) {
  bytes_[address] = value;
}

void Memory::write16(std::uint16_t address, std::uint16_t value) {
  // Little-endian write
  write8(address, lo(value));
  write8(static_cast<std::uint16_t>(address + 1), hi(value));
}

void Memory::loadBlock(const std::vector<std::uint8_t> &data,
                       std::uint16_t origin) {
  if (static_cast<std::size_t>(origin) + data.size() > bytes_.size()) {
    throw std::out_of_range("image does not fit in memory");
  }

  std::copy(data.begin(), data.end(), bytes_.begin() + origin);
}

} // namespace softcpu
