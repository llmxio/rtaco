#include "rtaco/nl_socket.hxx"

#include <stdexcept>
#include <iostream>
#include <unistd.h>

#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/impl/scheduler.ipp>
#include <boost/asio/detail/impl/service_registry.hpp>
#include <boost/asio/execution/context_as.hpp>
#include <boost/asio/execution/prefer_only.hpp>
#include <boost/asio/impl/any_io_executor.ipp>
#include <boost/asio/impl/io_context.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system/detail/error_code.hpp>

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

auto Socket::close() -> std::expected<void, std::error_code> {
    boost::system::error_code ec;

    if (socket_.close(ec); ec) {
        return std::unexpected{ec};
    }

    return {};
}

auto Socket::cancel() -> std::expected<void, std::error_code> {
    boost::system::error_code ec;

    if (socket_.cancel(ec); ec) {
        return std::unexpected{ec};
    }

    return {};
}

auto Socket::open(int proto, uint32_t groups) -> std::expected<void, std::error_code> {
    boost::system::error_code ec;

    if (socket_.open(Protocol{proto}, ec); ec) {
        throw std::runtime_error("failed to open netlink " + std::string{label_} +
                " socket: " + ec.message());
    }

    const auto enable_option =
            [this](const auto& option) -> std::expected<void, std::error_code>
    {
        boost::system::error_code ec;

        if (socket_.set_option(option, ec); ec) {
            return std::unexpected{ec};
        }

        return {};
    };

    if (auto rc = enable_option(recv_buf_option{1 << 12}); !rc) {
        return rc;
    }

    if (auto rc = enable_option(no_enobufs_option{1}); !rc) {
        return rc;
    }

    if (auto rc = enable_option(ext_ack_option{1}); !rc) {
        return rc;
    }

    if (auto rc = enable_option(strict_chk_option{1}); !rc) {
        return rc;
    }

    if (socket_.bind(endpoint_type{groups, 0U}, ec); ec) {
        close();
        throw std::runtime_error("failed to bind netlink " + std::string{label_} +
                " socket: " + ec.message());
    }

    // TODO: remove in production code
    {
        sockaddr_nl sa{};
        socklen_t salen = sizeof(sa);
        if (getsockname(socket_.native_handle(), reinterpret_cast<sockaddr*>(&sa),
                    &salen) == 0) {
            std::cout << label_ << ": bound nl_pid=" << sa.nl_pid << " nl_groups=0x"
                      << std::hex << sa.nl_groups << std::dec << "\n";
        } else {
            std::cout << label_ << ": getsockname failed: " << strerror(errno) << "\n";
        }
    }

    return {};
}

auto Socket::native_handle() -> native_type {
    return socket_.native_handle();
}

} // namespace nl
} // namespace llmx
