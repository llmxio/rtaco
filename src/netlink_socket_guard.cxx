#include "llmx/nl/netlink_socket_guard.h"

#include <boost/system/error_code.hpp>

#include "llmx/core/logger.h"
#include "llmx/core/utils.h"

namespace llmx {
namespace nl {

SocketGuard::SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept
    : socket_{io, label} {}

auto SocketGuard::socket() -> Socket& {
    return socket_;
}

auto SocketGuard::ensure_open() -> expected<void, llmx_error_policy> {
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
        failwith("nl-control connect(AF_NETLINK) failed: {}", ec.message());
    }

    return {};
}

void SocketGuard::stop() {
    if (!socket_.is_open()) {
        return;
    }

    if (auto result = socket_.cancel(); !result) {
        LOG(WARN) << "Failed to cancel netlink nl-control socket: "
                  << result.error().message();
    }

    if (auto result = socket_.close(); !result) {
        LOG(WARN) << "Failed to close netlink nl-control socket: "
                  << result.error().message();
    }
}

} // namespace nl
} // namespace llmx
