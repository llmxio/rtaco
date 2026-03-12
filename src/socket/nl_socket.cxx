#include "rtaco/socket/nl_socket.hxx"

#include <cerrno>
#include <cstdint>
#include <expected>
#include <string_view>
#include <system_error>

#include <linux/netlink.h>
#include <sys/socket.h>

#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>

#if defined(RTACO_ENABLE_TEST_HOOKS)
#include "tests/support/nl_test_hooks.hxx"
#endif

namespace llmx {
namespace rtaco {

Socket::Socket(boost::asio::io_context& io, std::string_view label) noexcept
    : socket_{io}
    , label_{label} {}

Socket::~Socket() noexcept {
    if (is_open()) {
        close();
    }
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

#if defined(RTACO_ENABLE_TEST_HOOKS)
    if (auto injected = test_hooks::consume_socket_open_error(); injected) {
        return std::unexpected{injected};
    }
#endif

    if (socket_.open(Protocol{proto}, ec); ec) {
        return std::unexpected{ec};
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

    if (auto rc = enable_option(recv_buf_option{(1 << 16)}); !rc) {
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

#if defined(RTACO_ENABLE_TEST_HOOKS)
    if (auto injected = test_hooks::consume_socket_bind_error(); injected) {
        ec = injected;
    } else {
        socket_.bind(endpoint_t{groups, 0}, ec);
    }
#else
    socket_.bind(endpoint_t{groups, 0}, ec);
#endif

    if (ec) {
        (void)close();
        return std::unexpected{ec};
    }

    return {};
}

auto Socket::native_handle() -> native_t {
    return socket_.native_handle();
}

} // namespace rtaco
} // namespace llmx
