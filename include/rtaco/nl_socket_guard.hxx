#pragma once

#include <expected>
#include <memory>
#include <mutex>
#include <string_view>
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
    using lock_type = std::unique_lock<std::mutex>;

    struct LockedSocket {
        Socket& socket;
        lock_type lock;

        LockedSocket(Socket& socket, lock_type&& lock) noexcept;

        LockedSocket(const LockedSocket&) = delete;
        LockedSocket& operator=(const LockedSocket&) = delete;
        LockedSocket(LockedSocket&&) noexcept = default;
        LockedSocket& operator=(LockedSocket&&) noexcept = default;
    };

    SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept;
    ~SocketGuard() = default;

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&&) = delete;
    SocketGuard& operator=(SocketGuard&&) = delete;

    auto acquire() -> std::expected<LockedSocket, std::error_code>;
    auto socket() -> Socket&;
    auto ensure_open() -> std::expected<void, std::error_code>;
    void stop();

protected:
    Socket socket_;
    std::mutex mutex_;

private:
    auto ensure_open_locked() -> std::expected<void, std::error_code>;
};

} // namespace nl
} // namespace llmx
