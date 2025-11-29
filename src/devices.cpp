#include "softcpu/device.hpp"

#include <iostream>

namespace softcpu {

namespace {
// Console device offsets
constexpr std::uint8_t kConsoleData = 0x00;
constexpr std::uint8_t kConsoleStatus = 0x01;

// Timer device offsets
constexpr std::uint8_t kTimerCounterLo = 0x00;
constexpr std::uint8_t kTimerCounterHi = 0x01;
constexpr std::uint8_t kTimerControl = 0x02;
constexpr std::uint8_t kTimerPeriodLo = 0x03;
constexpr std::uint8_t kTimerPeriodHi = 0x04;

// LED device offsets
constexpr std::uint8_t kLedValue = 0x00;
} // namespace

// ConsoleDevice implementation
ConsoleDevice::ConsoleDevice() : IODevice("console", 0xFF00, 0x0010) {}

std::uint8_t ConsoleDevice::read(std::uint16_t offset) {
  switch (offset) {
  case kConsoleStatus:
    return ready_ ? 0x01 : 0x00;
  default:
    return 0;
  }
}

void ConsoleDevice::write(std::uint16_t offset, std::uint8_t value) {
  if (offset == kConsoleData) {
    buffer_.push_back(static_cast<char>(value));
    std::cout << static_cast<char>(value) << std::flush;
  }
}

// TimerDevice implementation
TimerDevice::TimerDevice() : IODevice("timer", 0xFF10, 0x0010) {}

std::uint8_t TimerDevice::read(std::uint16_t offset) {
  switch (offset) {
  case kTimerCounterLo:
    return static_cast<std::uint8_t>(counter_ & 0xFF);
  case kTimerCounterHi:
    return static_cast<std::uint8_t>((counter_ >> 8) & 0xFF);
  case kTimerControl: {
    std::uint8_t control = 0;
    control |= enabled_ ? 0x01 : 0x00;
    control |= auto_reload_ ? 0x02 : 0x00;
    control |= (divider_ >= period_) ? 0x80 : 0x00;
    return control;
  }
  case kTimerPeriodLo:
    return static_cast<std::uint8_t>(period_ & 0xFF);
  case kTimerPeriodHi:
    return static_cast<std::uint8_t>((period_ >> 8) & 0xFF);
  default:
    return 0;
  }
}

void TimerDevice::write(std::uint16_t offset, std::uint8_t value) {
  switch (offset) {
  case kTimerControl:
    enabled_ = (value & 0x01) != 0;
    auto_reload_ = (value & 0x02) != 0;
    if ((value & 0x80) != 0) {
      divider_ = 0;
      counter_ = 0;
    }
    break;
  case kTimerPeriodLo:
    period_ = static_cast<std::uint16_t>((period_ & 0xFF00) | value);
    break;
  case kTimerPeriodHi:
    period_ = static_cast<std::uint16_t>(
        (period_ & 0x00FF) | (static_cast<std::uint16_t>(value) << 8));
    break;
  default:
    break;
  }
}

void TimerDevice::tick() {
  if (!enabled_) {
    return;
  }

  ++divider_;
  ++counter_;
  if (divider_ >= period_) {
    if (auto_reload_) {
      divider_ = 0;
      counter_ = 0;
    } else {
      enabled_ = false;
    }
  }
}

// LedPanel implementation
LedPanel::LedPanel() : IODevice("leds", 0xFF20, 0x0010) {}

std::uint8_t LedPanel::read(std::uint16_t offset) {
  if (offset == kLedValue) {
    return state_;
  }
  return 0;
}

void LedPanel::write(std::uint16_t offset, std::uint8_t value) {
  if (offset == kLedValue) {
    state_ = value;
  }
}

} // namespace softcpu
