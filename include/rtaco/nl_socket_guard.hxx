#pragma once

#include <expected>
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
namespace rtaco {

/** @brief Thread-safe guard owning a labeled `Socket`.
 *
 * `SocketGuard` provides synchronized access to an underlying `Socket` and
 * ensures the socket is opened on demand. It serializes operations with an
 * internal mutex and offers convenience methods to ensure the socket is open
 * or to stop/close it.
 */
class SocketGuard {
public:
    using lock_type = std::unique_lock<std::mutex>;

    /** @brief Construct a SocketGuard holding a labeled Socket. */
    SocketGuard(boost::asio::io_context& io, std::string_view label) noexcept;
    ~SocketGuard() = default;

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&&) = delete;
    SocketGuard& operator=(SocketGuard&&) = delete;

    /** @brief Access the underlying Socket instance. */
    auto socket() -> Socket&;

    /** @brief Ensure the underlying socket is open, opening it if necessary.
     *
     * @return Expected void or an error_code on failure.
     */
    auto ensure_open() -> std::expected<void, std::error_code>;

    /** @brief Stop and close the guarded socket. */
    void stop();

private:
    Socket socket_;
    std::mutex mutex_;

    auto ensure_open_locked() -> std::expected<void, std::error_code>;
};

} // namespace rtaco
} // namespace llmx
