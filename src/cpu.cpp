#include "softcpu/cpu.hpp"

#include "softcpu/alu.hpp"
#include "softcpu/bus.hpp"
#include "softcpu/control_unit.hpp"

namespace softcpu {

CPU::CPU(Bus &bus) : bus_(bus) {
  alu_ = std::make_unique<ALU>();
  control_ = std::make_unique<ControlUnit>(bus_, registers_, *alu_);
  reset();
}

CPU::~CPU() = default;

void CPU::reset() {
  registers_.reset();
  control_->reset();
}

bool CPU::step(bool trace) {
  // Update I/O devices (e.g., timers)
  bus_.tickDevices();

  // Execute one instruction
  return control_->step(trace);
}

} // namespace softcpu
