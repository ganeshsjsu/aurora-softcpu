#pragma once

#include "aurora/memory.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace aurora {

class IODevice;

class Bus {
public:
    explicit Bus(Memory& memory);

    std::uint8_t read8(std::uint16_t address) const;
    std::uint16_t read16(std::uint16_t address) const;
    void write8(std::uint16_t address, std::uint8_t value);
    void write16(std::uint16_t address, std::uint16_t value);

    void attachDevice(std::shared_ptr<IODevice> device);
    void tickDevices();

private:
    IODevice* findDevice(std::uint16_t address) const;

    Memory& memory_;
    std::vector<std::shared_ptr<IODevice>> devices_;
};

}  // namespace aurora
