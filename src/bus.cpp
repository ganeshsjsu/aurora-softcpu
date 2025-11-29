#include "softcpu/bus.hpp"
#include "softcpu/device.hpp"

#include <stdexcept>

namespace softcpu {

Bus::Bus(Memory &memory) : memory_(memory) {}

void Bus::attachDevice(std::shared_ptr<IODevice> device) {
  devices_.push_back(std::move(device));
}

IODevice *Bus::findDevice(std::uint16_t address) const {
  for (const auto &dev : devices_) {
    if (dev->handles(address)) {
      return dev.get();
    }
  }
  return nullptr;
}

std::uint8_t Bus::read8(std::uint16_t address) const {
  // Check if address maps to an I/O device
  if (auto *dev = findDevice(address)) {
    return dev->read(dev->offset(address));
  }
  // Otherwise read from memory
  return memory_.read8(address);
}

std::uint16_t Bus::read16(std::uint16_t address) const {
  // Check if address maps to an I/O device
  if (auto *dev = findDevice(address)) {
    const std::uint8_t low = dev->read(dev->offset(address));
    const std::uint8_t high =
        dev->read(dev->offset(static_cast<std::uint16_t>(address + 1)));
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(high) << 8) |
                                      low);
  }
  // Otherwise read from memory
  return memory_.read16(address);
}

void Bus::write8(std::uint16_t address, std::uint8_t value) {
  // Check if address maps to an I/O device
  if (auto *dev = findDevice(address)) {
    dev->write(dev->offset(address), value);
    return;
  }
  // Otherwise write to memory
  memory_.write8(address, value);
}

void Bus::write16(std::uint16_t address, std::uint16_t value) {
  // Check if address maps to an I/O device
  if (auto *dev = findDevice(address)) {
    dev->write(dev->offset(address), static_cast<std::uint8_t>(value & 0xFF));
    dev->write(dev->offset(static_cast<std::uint16_t>(address + 1)),
               static_cast<std::uint8_t>((value >> 8) & 0xFF));
    return;
  }
  // Otherwise write to memory
  memory_.write16(address, value);
}

void Bus::tickDevices() {
  for (auto &dev : devices_) {
    dev->tick();
  }
}

} // namespace softcpu
