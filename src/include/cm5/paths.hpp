#pragma once

#include <filesystem>

namespace cm5 {

inline constexpr std::string_view kRunDir = "/run/cm5-programming-jig-gpio-config";
inline constexpr std::string_view kSocketName = "led.sock";

[[nodiscard]] inline std::filesystem::path socket_path()
{
    return std::filesystem::path{kRunDir} / kSocketName;
}

} // namespace cm5
