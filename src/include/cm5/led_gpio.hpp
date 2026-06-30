#pragma once

#include <gpiod.hpp>

#include "cm5/led_mode.hpp"

namespace cm5 {

// GPIO 9 = blue, GPIO 10 = green, GPIO 11 = red (active-high).
class LedGpio {
public:
    static constexpr unsigned int kBluePin = 9;
    static constexpr unsigned int kGreenPin = 10;
    static constexpr unsigned int kRedPin = 11;

    LedGpio();
    ~LedGpio() = default;

    LedGpio(const LedGpio&) = delete;
    LedGpio& operator=(const LedGpio&) = delete;

    void apply_mode(LedMode mode, bool blink_phase_high);

private:
    gpiod::chip chip_;
    gpiod::line_request request_;
};

} // namespace cm5
