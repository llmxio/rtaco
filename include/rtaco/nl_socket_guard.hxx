#pragma once

#include <memory>
#include <string_view>

#include <boost/asio/io_context.hpp>

#include "llmx/core/error.h"
#include "rtaco/nl_socket.hxx"

namespace llmx {
namespace nl {

class SocketGuard {
public:
    SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept;
    virtual ~SocketGuard() = default;

    SocketGuard(const SocketGuard&)            = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&&)                 = delete;
    SocketGuard& operator=(SocketGuard&&)      = delete;

    virtual auto socket() -> Socket&;
    virtual auto ensure_open() -> expected<void, llmx_error_policy>;
    virtual void stop();

protected:
    Socket socket_;
};

} // namespace nl
} // namespace llmx
