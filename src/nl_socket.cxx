#include "rtaco/nl_socket.hxx"

#include <boost/asio/io_context.hpp>

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
        throw std::runtime_error("failed to open netlink " + std::string{label_} +
                " socket: " + ec.message());
    }

    const auto enable_option = [&](const auto& option, const char* description)
    {
        boost::system::error_code option_ec;

        if (socket_.set_option(option, option_ec); option_ec) {
            (void)option_ec;
        }
    };

    enable_option(recv_buf_option{2048}, "SO_RCVBUF not available");
    enable_option(no_enobufs_option{1}, "NETLINK_NO_ENOBUFS not available");
    enable_option(ext_ack_option{1}, "NETLINK_EXT_ACK not available");
    enable_option(strict_chk_option{1}, "NETLINK_GET_STRICT_CHK not available");

    if (socket_.bind(endpoint_type{groups, 0U}, ec); ec) {
        close();
        throw std::runtime_error("failed to bind netlink " + std::string{label_} +
                " socket: " + ec.message());
    }

    return {};
}

auto Socket::native_handle() -> native_type {
    return socket_.native_handle();
}

} // namespace nl
} // namespace llmx
