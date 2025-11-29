#pragma once

#include "softcpu/common.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace softcpu {

// Class representing the system memory (RAM)
class Memory {
public:
  Memory();

  // Read a single byte from the specified address
  std::uint8_t read8(std::uint16_t address) const;

  // Read a 16-bit word from the specified address (little-endian)
  std::uint16_t read16(std::uint16_t address) const;

  // Write a single byte to the specified address
  void write8(std::uint16_t address, std::uint8_t value);

  // Write a 16-bit word to the specified address (little-endian)
  void write16(std::uint16_t address, std::uint16_t value);

  // Load a block of data into memory starting at the origin address
  void loadBlock(const std::vector<std::uint8_t> &data, std::uint16_t origin);

  // Access the raw memory array
  const std::array<std::uint8_t, kMemorySize> &bytes() const { return bytes_; }

private:
  std::array<std::uint8_t, kMemorySize> bytes_{};
};

} // namespace softcpu
