#pragma once

#include "aurora/bus.hpp"
#include "aurora/cpu.hpp"
#include "aurora/device.hpp"
#include "aurora/memory.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace aurora {

struct RunOptions {
    std::uint64_t cycle_limit {0};
    bool trace {false};
};

class Emulator {
public:
    Emulator();

    void reset();
    void attachDefaultDevices();
    void loadImage(const std::vector<std::uint8_t>& image, std::uint16_t origin = kResetVector);
    bool loadBinaryFile(const std::string& path, std::uint16_t origin = kResetVector);
    bool saveMemoryDump(const std::string& path) const;
    bool dumpToStdout(std::uint16_t start, std::size_t count) const;
    bool run(const RunOptions& options);

    RegisterFile& registers();
    const RegisterFile& registers() const;
    Memory& memory();
    const Memory& memory() const;

private:
    Memory memory_;
    Bus bus_;
    std::unique_ptr<CPU> cpu_;
    std::vector<std::shared_ptr<IODevice>> devices_;
};

}  // namespace aurora
