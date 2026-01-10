#include "rtaco/nl_socket_guard.hxx"

#include <stdexcept>
#include <string>
#include <utility>
#include <iostream>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <boost/system/detail/error_code.hpp>

namespace llmx {
namespace nl {

constexpr unsigned NETLINK_GROUPS = RTMGRP_LINK | RTMGRP_NEIGH | RTMGRP_IPV4_IFADDR |
        RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

SocketGuard::SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept
    : socket_{io, label} {}

auto SocketGuard::socket() -> Socket& {
    return socket_;
}

auto SocketGuard::ensure_open() -> std::expected<void, std::error_code> {
    auto lock = lock_type{mutex_};

    return ensure_open_locked();
}

auto SocketGuard::ensure_open_locked() -> std::expected<void, std::error_code> {
    if (socket_.is_open()) {
        return {};
    }

    std::cout << "Opening netlink socket\n";

    if (auto result = socket_.open(NETLINK_ROUTE, NETLINK_GROUPS); !result) {
        return std::unexpected{result.error()};
    }

    std::cout << "Netlink socket opened\n";

    boost::system::error_code ec;
    socket_.connect(Protocol::endpoint{NETLINK_GROUPS, 0U}, ec);
    if (ec) {
        socket_.close();
        throw std::runtime_error(" connect failed: " + ec.message());
    }

    std::cout << "Netlink socket connected\n";

    return {};
}

void SocketGuard::stop() {
    auto lock = lock_type{mutex_};

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
