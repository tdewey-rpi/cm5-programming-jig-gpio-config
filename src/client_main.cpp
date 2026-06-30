#include <iostream>
#include <optional>
#include <string_view>

#include "cm5/led_mode.hpp"
#include "cm5/protocol.hpp"

namespace {

[[nodiscard]] bool set_mode(const cm5::LedMode mode)
{
    const std::string command = std::string{"SET "} + std::string{cm5::led_mode_name(mode)};
    return cm5::send_command(command);
}

void usage()
{
    std::cerr << "Usage: cm5_led_blink stop-all\n"
              << "       cm5_led_blink hold <line=value> ...\n"
              << "       cm5_led_blink <blue|red> <start|stop>\n";
}

[[nodiscard]] bool legacy_hold(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return set_mode(cm5::LedMode::Ready);
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2) {
        usage();
        return 1;
    }

    const std::string_view command = argv[1];

    if (command == "stop-all") {
        return set_mode(cm5::LedMode::Off) ? 0 : 1;
    }

    if (command == "hold") {
        return legacy_hold(argc, argv) ? 0 : 1;
    }

    if (argc < 3) {
        usage();
        return 1;
    }

    const std::string_view colour = argv[1];
    const std::string_view action = argv[2];

    if (action == "stop") {
        return set_mode(cm5::LedMode::Off) ? 0 : 1;
    }

    if (action != "start") {
        usage();
        return 1;
    }

    if (colour == "blue") {
        return set_mode(cm5::LedMode::Active) ? 0 : 1;
    }
    if (colour == "red") {
        return set_mode(cm5::LedMode::Failed) ? 0 : 1;
    }

    usage();
    return 1;
}
