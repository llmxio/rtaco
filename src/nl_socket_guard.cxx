#include "rtaco/nl_socket_guard.hxx"

#include <linux/netlink.h>
#include <boost/system/detail/error_code.hpp>
#include <stdexcept>
#include <string>

namespace llmx {
namespace nl {

SocketGuard::SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept
    : socket_{io, label} {}

auto SocketGuard::socket() -> Socket& {
    return socket_;
}

auto SocketGuard::ensure_open() -> std::expected<void, std::error_code> {
    if (socket_.is_open()) {
        return {};
    }

    if (auto result = socket_.open(NETLINK_ROUTE, 0U); !result) {
        return std::unexpected{result.error()};
    }

    boost::system::error_code ec;
    socket_.connect(nl::Protocol::endpoint{0U, 0U}, ec);
    if (ec) {
        socket_.close();
        throw std::runtime_error(
                "nl-control connect(AF_NETLINK) failed: " + ec.message());
    }

    return {};
}

void SocketGuard::stop() {
    if (!socket_.is_open()) {
        return;
    }

    if (auto result = socket_.cancel(); !result) {
        (void)result;
    }

    if (auto result = socket_.close(); !result) {
        (void)result;
    }
}

} // namespace nl
} // namespace llmx
