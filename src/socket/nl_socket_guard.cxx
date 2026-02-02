#include "rtaco/socket/nl_socket_guard.hxx"

#include <expected>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include <boost/system/error_code.hpp>
#include <linux/netlink.h>

#include <linux/rtnetlink.h>
#include "rtaco/socket/nl_protocol.hxx"

namespace llmx {
namespace rtaco {

constexpr uint32_t DEFAULT_GROUP_MASK = RTMGRP_LINK | RTMGRP_NEIGH | RTMGRP_IPV4_IFADDR |
        RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

SocketGuard::SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept
    : SocketGuard{io, label, DEFAULT_GROUP_MASK} {}

SocketGuard::SocketGuard(boost::asio::io_context& io, std::string_view label,
        uint32_t group_mask) noexcept
    : socket_{io, label}
    , group_mask_{group_mask} {}

auto SocketGuard::socket() -> Socket& {
    return socket_;
}

auto SocketGuard::ensure_open() -> std::expected<void, std::error_code> {
    if (socket_.is_open()) {
        return {};
    }

    if (auto result = socket_.open(NETLINK_ROUTE, group_mask_); !result) {
        return std::unexpected{result.error()};
    }

    boost::system::error_code ec;
    socket_.connect(Protocol::endpoint{group_mask_, 0}, ec);

    if (ec) {
        socket_.close();
        return std::unexpected{ec};
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

} // namespace rtaco
} // namespace llmx
