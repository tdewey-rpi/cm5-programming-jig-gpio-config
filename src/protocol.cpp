#include "cm5/protocol.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cm5/paths.hpp"

namespace cm5 {

namespace {

[[nodiscard]] bool write_all(const int fd, std::string_view data)
{
    while (!data.empty()) {
        const auto written = ::write(fd, data.data(), data.size());
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        data.remove_prefix(static_cast<std::size_t>(written));
    }
    return true;
}

[[nodiscard]] std::optional<std::string> read_line(const int fd)
{
    std::string line;
    std::array<char, 1> byte{};

    while (true) {
        const auto read_bytes = ::read(fd, byte.data(), byte.size());
        if (read_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            return std::nullopt;
        }
        if (read_bytes == 0) {
            break;
        }
        if (byte[0] == '\n') {
            break;
        }
        line.push_back(byte[0]);
    }

    return line;
}

[[nodiscard]] int connect_socket()
{
    const int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    const auto path = socket_path().native();
    if (path.size() >= sizeof(addr.sun_path)) {
        ::close(fd);
        return -1;
    }
    std::memcpy(addr.sun_path, path.c_str(), path.size() + 1);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}

[[nodiscard]] std::optional<std::string> transact(std::string_view request)
{
    const int fd = connect_socket();
    if (fd < 0) {
        return std::nullopt;
    }

    std::string payload{request};
    if (payload.empty() || payload.back() != '\n') {
        payload.push_back('\n');
    }

    if (!write_all(fd, payload)) {
        ::close(fd);
        return std::nullopt;
    }

    const auto response = read_line(fd);
    ::close(fd);
    return response;
}

[[nodiscard]] std::optional<std::string_view> strip_ok_prefix(std::string_view response)
{
    constexpr std::string_view kPrefix = "OK";
    if (!response.starts_with(kPrefix)) {
        return std::nullopt;
    }
    response.remove_prefix(kPrefix.size());
    if (!response.empty() && response.front() == ' ') {
        response.remove_prefix(1);
    }
    return response;
}

} // namespace

std::optional<Command> parse_command(const std::string_view line)
{
    if (line == "PING") {
        return Command{CommandType::Ping, std::nullopt};
    }
    if (line == "GET") {
        return Command{CommandType::Get, std::nullopt};
    }
    constexpr std::string_view kSetPrefix = "SET ";
    if (line.starts_with(kSetPrefix)) {
        const auto mode = led_mode_from_name(line.substr(kSetPrefix.size()));
        if (!mode.has_value()) {
            return std::nullopt;
        }
        return Command{CommandType::Set, mode};
    }
    return std::nullopt;
}

bool send_command(const std::string_view command_line)
{
    const auto response = transact(command_line);
    return response.has_value() && response->starts_with("OK");
}

std::optional<LedMode> query_mode()
{
    const auto response = transact("GET");
    if (!response.has_value()) {
        return std::nullopt;
    }
    const auto mode_name = strip_ok_prefix(*response);
    if (!mode_name.has_value()) {
        return std::nullopt;
    }
    return led_mode_from_name(*mode_name);
}

} // namespace cm5
