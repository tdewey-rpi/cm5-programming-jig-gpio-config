#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "cm5/led_mode.hpp"

namespace cm5 {

enum class CommandType {
    Ping,
    Get,
    Set,
};

struct Command {
    CommandType type{};
    std::optional<LedMode> mode{};
};

[[nodiscard]] std::optional<Command> parse_command(std::string_view line);

[[nodiscard]] bool send_command(std::string_view command_line);

[[nodiscard]] std::optional<LedMode> query_mode();

} // namespace cm5
