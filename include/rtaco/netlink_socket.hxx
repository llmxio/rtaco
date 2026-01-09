#pragma once

#include <boost/system/error_code.hpp>

#include <cstddef>
#include <utility>

#include "llmx/core/error.h"
#include "llmx/nl/netlink_protocol.h"

namespace llmx {
namespace nl {

class Socket {
public:
    using socket_type = Protocol::socket;
    using endpoint_type = Protocol::endpoint;
    using native_type = typename Protocol::socket::native_handle_type;

    explicit Socket(boost::asio::io_context& io, std::string_view label) noexcept;
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&&) noexcept = default;
    Socket& operator=(Socket&&) noexcept = default;

    auto is_open() const noexcept -> bool;

    auto close() -> expected<void, llmx_error_policy>;
    auto cancel() -> expected<void, llmx_error_policy>;

    auto open(int proto, uint32_t groups) -> expected<void, llmx_error_policy>;

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
            -> decltype(std::declval<socket_type>().async_receive(buffers,
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
            -> decltype(std::declval<socket_type>()
                            .async_send(buffers, std::forward<CompletionToken>(token))) {
        return socket_.async_send(buffers, std::forward<CompletionToken>(token));
    }

    template<typename ConstBufferSequence>
    auto send(const ConstBufferSequence& buffers, boost::system::error_code& ec)
            -> size_t {
        return socket_.send(buffers, 0, ec);
    }

    auto native_handle() -> native_type;

private:
    using ext_ack_option =
            boost::asio::detail::socket_option::integer<SOL_NETLINK, NETLINK_EXT_ACK>;
    using strict_chk_option = boost::asio::detail::socket_option::integer<SOL_NETLINK,
            NETLINK_GET_STRICT_CHK>;
    using recv_buf_option =
            boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVBUF>;
    using no_enobufs_option =
            boost::asio::detail::socket_option::integer<SOL_NETLINK, NETLINK_NO_ENOBUFS>;

    Protocol::socket socket_;
    std::string label_{};
};

} // namespace nl
} // namespace llmx
