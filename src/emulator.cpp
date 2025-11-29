#include "softcpu/emulator.hpp"

#include "softcpu/device.hpp"
#include "softcpu/utils.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>

namespace softcpu {

Emulator::Emulator() : memory_(), bus_(memory_) {
  cpu_ = std::make_unique<CPU>(bus_);
  attachDefaultDevices();
}

void Emulator::reset() {
  memory_ = Memory();
  cpu_->reset();
}

void Emulator::attachDefaultDevices() {
  if (!devices_.empty()) {
    return;
  }
  // Attach standard I/O devices
  devices_.push_back(std::make_shared<ConsoleDevice>());
  devices_.push_back(std::make_shared<TimerDevice>());
  devices_.push_back(std::make_shared<LedPanel>());
  for (auto &dev : devices_) {
    bus_.attachDevice(dev);
  }
}

void Emulator::loadImage(const std::vector<std::uint8_t> &image,
                         std::uint16_t origin) {
  memory_.loadBlock(image, origin);
}

bool Emulator::loadBinaryFile(const std::string &path, std::uint16_t origin) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    return false;
  }
  std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(input)),
                                 std::istreambuf_iterator<char>());
  loadImage(data, origin);
  return true;
}

bool Emulator::saveMemoryDump(const std::string &path) const {
  const auto &bytes = memory_.bytes();
  std::vector<std::uint8_t> dump(bytes.begin(), bytes.end());
  return util::writeBinaryFile(path, dump);
}

bool Emulator::dumpToStdout(std::uint16_t start, std::size_t count) const {
  if (static_cast<std::size_t>(start) + count > memory_.bytes().size()) {
    return false;
  }
  std::cout << std::hex << std::setfill('0');
  for (std::size_t i = 0; i < count; i += 16) {
    std::cout << std::setw(4) << static_cast<int>(start + i) << ": ";
    for (std::size_t b = 0; b < 16 && i + b < count; ++b) {
      const auto value = memory_.bytes()[start + i + b];
      std::cout << std::setw(2) << static_cast<int>(value) << ' ';
    }
    std::cout << '\n';
  }
  std::cout << std::dec;
  return true;
}

bool Emulator::run(const RunOptions &options) {
  std::uint64_t cycles = 0;
  while (options.cycle_limit == 0 || cycles < options.cycle_limit) {
    if (!cpu_->step(options.trace)) {
      return true;
    }
    ++cycles;
  }
  return true;
}

RegisterFile &Emulator::registers() { return cpu_->registers(); }

const RegisterFile &Emulator::registers() const { return cpu_->registers(); }

Memory &Emulator::memory() { return memory_; }

const Memory &Emulator::memory() const { return memory_; }

} // namespace softcpu
