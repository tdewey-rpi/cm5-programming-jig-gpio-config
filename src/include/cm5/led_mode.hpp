#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace cm5 {

enum class LedMode : std::uint8_t {
    Off,
    Ready,
    Active,
    Failed,
};

[[nodiscard]] constexpr std::string_view led_mode_name(LedMode mode) noexcept
{
    switch (mode) {
    case LedMode::Off:
        return "off";
    case LedMode::Ready:
        return "ready";
    case LedMode::Active:
        return "active";
    case LedMode::Failed:
        return "failed";
    }
    return "off";
}

[[nodiscard]] constexpr std::optional<LedMode> led_mode_from_name(std::string_view name) noexcept
{
    if (name == "off") {
        return LedMode::Off;
    }
    if (name == "ready") {
        return LedMode::Ready;
    }
    if (name == "active") {
        return LedMode::Active;
    }
    if (name == "failed") {
        return LedMode::Failed;
    }
    return std::nullopt;
}

} // namespace cm5
