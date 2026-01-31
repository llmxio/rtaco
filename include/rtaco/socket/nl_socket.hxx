#pragma once

/**
 * @file nl_socket.hxx
 * @brief Netlink socket helper declarations.
 *
 * Provides a RAII wrapper around a Boost.Asio netlink socket and
 * utility functions for setup and error handling used by rtaco.
 */

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <linux/netlink.h>
#include <stdint.h>
#include <sys/socket.h>

#include <boost/asio/detail/socket_option.hpp>

#include "rtaco/nl_protocol.hxx"

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost
namespace boost {
namespace system {
class error_code;
}
} // namespace boost

namespace llmx {
namespace rtaco {

/**
 * @brief RAII wrapper for a Boost.Asio netlink socket.
 *
 * Encapsulates socket creation, option configuration, bind, and teardown
 * for Netlink protocol sockets used by rtaco.
 */
class Socket {
public:
    using socket_t = Protocol::socket;
    using endpoint_t = Protocol::endpoint;
    using native_t = typename socket_t::native_handle_type;

    /** @brief Construct a netlink Socket with an io_context and label. */
    explicit Socket(boost::asio::io_context& io, std::string_view label) noexcept;
    /** @brief Destroy the Socket object and ensure underlying socket is closed. */
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&&) noexcept = default;
    Socket& operator=(Socket&&) noexcept = default;

    /** @brief Check whether the socket is currently open. @return true if the underlying
     * Boost.Asio socket is open. */
    auto is_open() const noexcept -> bool;
    /**
     * @brief Close the underlying socket.
     *
     * @return std::expected<void, std::error_code> Empty on success or contains
     *         the encountered error on failure.
     */
    auto close() -> std::expected<void, std::error_code>;
    /**
     * @brief Cancel any asynchronous operations on the socket.
     *
     * @return std::expected<void, std::error_code> Empty on success or contains
     *         the encountered error on failure.
     */
    auto cancel() -> std::expected<void, std::error_code>;

    /**
     * @brief Open and configure the netlink socket.
     *
     * Opens the socket for the given netlink protocol and subscribes to the
     * provided multicast groups. Configures recommended socket options and binds
     * the socket.
     *
     * @throws std::runtime_error on fatal failures when opening or binding fails.
     * @return std::expected<void, std::error_code> Empty on success or contains
     *         the encountered error from socket option configuration.
     */
    auto open(int proto, uint32_t groups) -> std::expected<void, std::error_code>;

    template<typename Option>
    void set_option(const Option& option, boost::system::error_code& ec) {
        socket_.set_option(option, ec);
    }

    template<typename Endpoint>
    void bind(const Endpoint& endpoint, boost::system::error_code& ec) {
        socket_.bind(endpoint, ec);
    }

    template<typename Endpoint>
    void connect(const Endpoint& endpoint, boost::system::error_code& ec) {
        socket_.connect(endpoint, ec);
    }

    template<typename MutableBufferSequence, typename CompletionToken>
    auto async_receive(const MutableBufferSequence& buffers, CompletionToken&& token)
            -> decltype(std::declval<socket_t>().async_receive(buffers,
                    std::forward<CompletionToken>(token))) {
        return socket_.async_receive(buffers, std::forward<CompletionToken>(token));
    }

    template<typename MutableBufferSequence>
    auto receive(const MutableBufferSequence& buffers, boost::system::error_code& ec)
            -> size_t {
        return socket_.receive(buffers, 0, ec);
    }

    template<typename ConstBufferSequence, typename CompletionToken>
    auto async_send(const ConstBufferSequence& buffers, CompletionToken&& token)
            -> decltype(std::declval<socket_t>()
                            .async_send(buffers, std::forward<CompletionToken>(token))) {
        return socket_.async_send(buffers, std::forward<CompletionToken>(token));
    }

    template<typename ConstBufferSequence>
    auto send(const ConstBufferSequence& buffers, boost::system::error_code& ec)
            -> size_t {
        return socket_.send(buffers, 0, ec);
    }

    /**
     * @brief Get the native socket handle.
     *
     * @return native_t The underlying native socket handle (file descriptor).
     */
    auto native_handle() -> native_t;

private:
    using ext_ack_option =
            boost::asio::detail::socket_option::integer<SOL_NETLINK, NETLINK_EXT_ACK>;

    using strict_chk_option = boost::asio::detail::socket_option::integer<SOL_NETLINK,
            NETLINK_GET_STRICT_CHK>;

    using recv_buf_option =
            boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVBUF>;

    using no_enobufs_option =
            boost::asio::detail::socket_option::integer<SOL_NETLINK, NETLINK_NO_ENOBUFS>;

    socket_t socket_;
    std::string label_{};
};

} // namespace rtaco
} // namespace llmx
