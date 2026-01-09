#pragma once

#include <memory>
#include <string_view>
#include <expected>
#include <system_error>

#include "rtaco/nl_socket.hxx"

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

namespace llmx {
namespace nl {

class SocketGuard {
public:
    SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept;
    virtual ~SocketGuard() = default;

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&&) = delete;
    SocketGuard& operator=(SocketGuard&&) = delete;

    virtual auto socket() -> Socket&;
    virtual auto ensure_open() -> std::expected<void, std::error_code>;
    virtual void stop();

protected:
    Socket socket_;
};

} // namespace nl
} // namespace llmx
