#pragma once

#include "aurora/common.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace aurora {

class Memory {
public:
    Memory();

    std::uint8_t read8(std::uint16_t address) const;
    std::uint16_t read16(std::uint16_t address) const;
    void write8(std::uint16_t address, std::uint8_t value);
    void write16(std::uint16_t address, std::uint16_t value);

    void loadBlock(const std::vector<std::uint8_t>& data, std::uint16_t origin);
    const std::array<std::uint8_t, kMemorySize>& bytes() const { return bytes_; }

private:
    std::array<std::uint8_t, kMemorySize> bytes_{};
};

}  // namespace aurora
