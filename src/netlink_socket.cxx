#include "llmx/nl/netlink_socket.h"

#include <boost/asio/io_context.hpp>

#include "llmx/core/utils.h"

namespace llmx {
namespace nl {

Socket::Socket(boost::asio::io_context& io, std::string_view label) noexcept
    : socket_{io}
    , label_{label} {}

Socket::~Socket() noexcept {
    close();
}

auto Socket::is_open() const noexcept -> bool {
    return socket_.is_open();
}

auto Socket::close() -> expected<void, llmx_error_policy> {
    boost::system::error_code ec;

    if (socket_.close(ec); ec) {
        return std::unexpected{ec};
    }

    return {};
}

auto Socket::cancel() -> expected<void, llmx_error_policy> {
    boost::system::error_code ec;

    if (socket_.cancel(ec); ec) {
        return std::unexpected{ec};
    }

    return {};
}

auto Socket::open(int proto, uint32_t groups) -> expected<void, llmx_error_policy> {
    boost::system::error_code ec;

    if (socket_.open(Protocol{proto}, ec); ec) {
        failwith("failed to open netlink {} socket: {}", label_, ec.message());
    }

    const auto enable_option = [&](const auto& option, const char* description)
    {
        boost::system::error_code option_ec;

        if (socket_.set_option(option, option_ec); option_ec) {
            LOG(WARN) << label_ << ": " << description << ": " << option_ec.message();
        }
    };

    enable_option(recv_buf_option{2048}, "SO_RCVBUF not available");
    enable_option(no_enobufs_option{1}, "NETLINK_NO_ENOBUFS not available");
    enable_option(ext_ack_option{1}, "NETLINK_EXT_ACK not available");
    enable_option(strict_chk_option{1}, "NETLINK_GET_STRICT_CHK not available");

    if (socket_.bind(endpoint_type{groups, 0U}, ec); ec) {
        close();
        failwith("{} bind(AF_NETLINK) failed: {}", label_, ec.message());
    }

    return {};
}

auto Socket::native_handle() -> native_type {
    return socket_.native_handle();
}

} // namespace nl
} // namespace llmx
