#pragma once

#include <cstdint>
#include <string>

namespace aurora {

class IODevice {
public:
    IODevice(std::string name, std::uint16_t base, std::uint16_t size)
        : name_(std::move(name)), base_(base), size_(size) {}
    virtual ~IODevice() = default;

    friend class Bus;

    bool handles(std::uint16_t address) const {
        return address >= base_ && address < static_cast<std::uint32_t>(base_) + size_;
    }

    std::uint16_t base() const { return base_; }
    std::uint16_t size() const { return size_; }
    const std::string& name() const { return name_; }

    virtual std::uint8_t read(std::uint16_t offset) = 0;
    virtual void write(std::uint16_t offset, std::uint8_t value) = 0;
    virtual void tick() {}

protected:
    std::uint16_t offset(std::uint16_t address) const { return static_cast<std::uint16_t>(address - base_); }

private:
    std::string name_;
    std::uint16_t base_;
    std::uint16_t size_;
};

class ConsoleDevice final : public IODevice {
public:
    ConsoleDevice();
    std::uint8_t read(std::uint16_t offset) override;
    void write(std::uint16_t offset, std::uint8_t value) override;
    std::string buffer() const { return buffer_; }

private:
    std::string buffer_;
    bool ready_ {true};
};

class TimerDevice final : public IODevice {
public:
    TimerDevice();
    std::uint8_t read(std::uint16_t offset) override;
    void write(std::uint16_t offset, std::uint8_t value) override;
    void tick() override;

private:
    std::uint32_t divider_ {0};
    std::uint16_t period_ {1000};
    std::uint16_t counter_ {0};
    bool enabled_ {false};
    bool auto_reload_ {true};
};

class LedPanel final : public IODevice {
public:
    LedPanel();
    std::uint8_t read(std::uint16_t offset) override;
    void write(std::uint16_t offset, std::uint8_t value) override;
    std::uint8_t state() const { return state_; }

private:
    std::uint8_t state_ {0};
};

}  // namespace aurora
