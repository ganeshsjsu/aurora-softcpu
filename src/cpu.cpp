#include "aurora/cpu.hpp"

#include "aurora/alu.hpp"
#include "aurora/bus.hpp"
#include "aurora/control_unit.hpp"

namespace aurora {

CPU::CPU(Bus& bus) : bus_(bus) {
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
    bus_.tickDevices();
    return control_->step(trace);
}

}  // namespace aurora
