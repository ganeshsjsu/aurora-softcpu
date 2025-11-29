#pragma once

#include "softcpu/bus.hpp"
#include "softcpu/cpu.hpp"
#include "softcpu/device.hpp"
#include "softcpu/memory.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace softcpu {

// Options for running the emulator
struct RunOptions {
  std::uint64_t cycle_limit{
      0};            // Maximum number of cycles to run (0 for unlimited)
  bool trace{false}; // Enable instruction tracing
};

// Main Emulator class that integrates CPU, Memory, Bus, and Devices
class Emulator {
public:
  Emulator();

  // Reset the emulator state (CPU, Memory, etc.)
  void reset();

  // Attach default I/O devices to the bus
  void attachDefaultDevices();

  // Load a binary image into memory at a specific origin
  void loadImage(const std::vector<std::uint8_t> &image,
                 std::uint16_t origin = kResetVector);

  // Load a binary file from disk into memory
  bool loadBinaryFile(const std::string &path,
                      std::uint16_t origin = kResetVector);

  // Save the entire memory content to a file
  bool saveMemoryDump(const std::string &path) const;

  // Dump a range of memory to standard output (hexdump format)
  bool dumpToStdout(std::uint16_t start, std::size_t count) const;

  // Run the emulation loop
  bool run(const RunOptions &options);

  // Accessors for registers
  RegisterFile &registers();
  const RegisterFile &registers() const;

  // Accessors for memory
  Memory &memory();
  const Memory &memory() const;

private:
  Memory memory_;
  Bus bus_;
  std::unique_ptr<CPU> cpu_;
  std::vector<std::shared_ptr<IODevice>> devices_;
};

} // namespace softcpu
