#pragma once

#include "softcpu/memory.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace softcpu {

class IODevice;

// The Bus class handles communication between the CPU, Memory, and I/O Devices
class Bus {
public:
  explicit Bus(Memory &memory);

  // Read a byte from memory or an I/O device
  std::uint8_t read8(std::uint16_t address) const;

  // Read a 16-bit word from memory or an I/O device
  std::uint16_t read16(std::uint16_t address) const;

  // Write a byte to memory or an I/O device
  void write8(std::uint16_t address, std::uint8_t value);

  // Write a 16-bit word to memory or an I/O device
  void write16(std::uint16_t address, std::uint16_t value);

  // Attach an I/O device to the bus
  void attachDevice(std::shared_ptr<IODevice> device);

  // Update the state of all attached devices (e.g., for timers or interrupts)
  void tickDevices();

private:
  // Find the device mapped to a specific address
  IODevice *findDevice(std::uint16_t address) const;

  Memory &memory_;
  std::vector<std::shared_ptr<IODevice>> devices_;
};

} // namespace softcpu
