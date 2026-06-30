#include "cm5/led_gpio.hpp"

namespace cm5 {

namespace {

gpiod::line::offsets led_offsets()
{
    return gpiod::line::offsets{
        gpiod::line::offset{LedGpio::kBluePin},
        gpiod::line::offset{LedGpio::kGreenPin},
        gpiod::line::offset{LedGpio::kRedPin},
    };
}

gpiod::line::value to_line_value(bool active)
{
    return active ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE;
}

} // namespace

LedGpio::LedGpio()
    : chip_{"/dev/gpiochip0"}
    , request_{chip_.prepare_request()
                    .set_consumer("cm5-provisioner-led")
                    .add_line_settings(led_offsets(), [] {
                        gpiod::line_settings settings;
                        settings.set_direction(gpiod::line::direction::OUTPUT);
                        settings.set_output_value(gpiod::line::value::INACTIVE);
                        return settings;
                    }())
                    .do_request()}
{
    apply_mode(LedMode::Off, false);
}

void LedGpio::apply_mode(const LedMode mode, const bool blink_phase_high)
{
    bool blue = false;
    bool green = false;
    bool red = false;

    switch (mode) {
    case LedMode::Off:
        break;
    case LedMode::Ready:
        green = true;
        break;
    case LedMode::Active:
        blue = blink_phase_high;
        break;
    case LedMode::Failed:
        red = blink_phase_high;
        break;
    }

    const gpiod::line::values values{
        to_line_value(blue),
        to_line_value(green),
        to_line_value(red),
    };
    request_.set_values(led_offsets(), values);
}

} // namespace cm5
