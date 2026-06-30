#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cm5/led_gpio.hpp"
#include "cm5/led_mode.hpp"
#include "cm5/paths.hpp"
#include "cm5/protocol.hpp"

namespace {

constexpr auto kBlinkInterval = std::chrono::milliseconds{250};

[[nodiscard]] bool write_response(const int client_fd, const std::string_view response)
{
    std::string payload{response};
    payload.push_back('\n');
    const auto* data = payload.data();
    auto remaining = payload.size();
    while (remaining > 0) {
        const auto written = ::write(client_fd, data, remaining);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        data += written;
        remaining -= static_cast<std::size_t>(written);
    }
    return true;
}

[[nodiscard]] std::optional<std::string> read_request(const int client_fd)
{
    std::string line;
    char byte = 0;
    while (true) {
        const auto read_bytes = ::read(client_fd, &byte, 1);
        if (read_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            return std::nullopt;
        }
        if (read_bytes == 0) {
            break;
        }
        if (byte == '\n') {
            break;
        }
        line.push_back(byte);
    }
    return line;
}

[[nodiscard]] bool handle_client(
    const int client_fd,
    cm5::LedMode& mode,
    bool& blink_phase_high,
    cm5::LedGpio& gpio)
{
    const auto request = read_request(client_fd);
    if (!request.has_value()) {
        return false;
    }

    const auto command = cm5::parse_command(*request);
    if (!command.has_value()) {
        (void)write_response(client_fd, "ERR unknown command");
        return false;
    }

    switch (command->type) {
    case cm5::CommandType::Ping:
        return write_response(client_fd, "OK");
    case cm5::CommandType::Get:
        return write_response(client_fd, std::string{"OK "} + std::string{cm5::led_mode_name(mode)});
    case cm5::CommandType::Set:
        if (!command->mode.has_value()) {
            return write_response(client_fd, "ERR missing mode");
        }
        mode = *command->mode;
        blink_phase_high = true;
        gpio.apply_mode(mode, blink_phase_high);
        return write_response(client_fd, "OK");
    }

    return write_response(client_fd, "ERR internal");
}

[[nodiscard]] int create_listen_socket()
{
    const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    const auto path = cm5::socket_path().native();
    if (path.size() >= sizeof(addr.sun_path)) {
        ::close(fd);
        return -1;
    }
    std::memcpy(addr.sun_path, path.c_str(), path.size() + 1);

    ::unlink(path.c_str());
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return -1;
    }
    if (::listen(fd, 8) < 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}

[[nodiscard]] int create_blink_timer()
{
    const int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd < 0) {
        return -1;
    }

    const itimerspec spec{
        .it_interval{
            .tv_sec = static_cast<time_t>(kBlinkInterval.count() / 1000),
            .tv_nsec = static_cast<long>((kBlinkInterval.count() % 1000) * 1'000'000),
        },
        .it_value{
            .tv_sec = static_cast<time_t>(kBlinkInterval.count() / 1000),
            .tv_nsec = static_cast<long>((kBlinkInterval.count() % 1000) * 1'000'000),
        },
    };
    if (::timerfd_settime(fd, 0, &spec, nullptr) < 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}

[[nodiscard]] bool mode_blinks(const cm5::LedMode mode)
{
    return mode == cm5::LedMode::Active || mode == cm5::LedMode::Failed;
}

} // namespace

int main()
{
    std::error_code ec;
    std::filesystem::create_directories(cm5::kRunDir, ec);

    try {
        cm5::LedGpio gpio;
        const int listen_fd = create_listen_socket();
        if (listen_fd < 0) {
            std::cerr << "cm5-provisioner-led: failed to bind socket\n";
            return 1;
        }

        const int timer_fd = create_blink_timer();
        if (timer_fd < 0) {
            std::cerr << "cm5-provisioner-led: failed to create blink timer\n";
            return 1;
        }

        auto mode = cm5::LedMode::Off;
        auto blink_phase_high = true;
        gpio.apply_mode(mode, blink_phase_high);

        pollfd fds[2]{};
        fds[0].fd = listen_fd;
        fds[0].events = POLLIN;
        fds[1].fd = timer_fd;
        fds[1].events = POLLIN;

        while (true) {
            if (::poll(fds, 2, -1) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }

            if ((fds[0].revents & POLLIN) != 0) {
                const int client_fd = ::accept4(listen_fd, nullptr, nullptr, SOCK_CLOEXEC);
                if (client_fd >= 0) {
                    (void)handle_client(client_fd, mode, blink_phase_high, gpio);
                    ::close(client_fd);
                }
            }

            if ((fds[1].revents & POLLIN) != 0) {
                std::uint64_t expirations = 0;
                if (::read(timer_fd, &expirations, sizeof(expirations)) > 0 && mode_blinks(mode)) {
                    blink_phase_high = !blink_phase_high;
                    gpio.apply_mode(mode, blink_phase_high);
                }
            }
        }

        ::close(timer_fd);
        ::close(listen_fd);
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "cm5-provisioner-led: " << ex.what() << '\n';
        return 1;
    }
}
