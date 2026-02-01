#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <optional>
#include <span>
#include <system_error>
#include <expected>
#include <vector>

#include <linux/netlink.h>

#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>

#include "rtaco/socket/nl_socket_guard.hxx"

namespace llmx {
namespace rtaco {

template<typename Derived, typename Result>
concept request_behavior =
        requires(Derived& derived, const Derived& const_derived, const nlmsghdr& header) {
            { derived.prepare_request() } -> std::same_as<void>;
            { const_derived.request_payload() } -> std::same_as<std::span<const uint8_t>>;
            {
                derived.process_message(header)
            } -> std::same_as<std::optional<std::expected<Result, std::error_code>>>;
        };

/** @brief Base template for single-request netlink tasks.
 *
 * Implements the common flow: ask the Derived to prepare a request, send
 * the request via the guarded socket, and read replies until the Derived
 * signals completion via `process_message`.
 *
 * Derived classes must implement `prepare_request()`, `request_payload()`
 * and `process_message(const nlmsghdr&)`.
 */
template<typename Derived, typename Result>
class RequestTask {
    static constexpr size_t MAX_RESPONSE_BYTES = 64U * 1024U;
    std::array<uint8_t, MAX_RESPONSE_BYTES> receive_buffer_;

    SocketGuard& socket_guard_;
    uint16_t ifindex_;
    uint32_t sequence_;

public:
    /** @brief Construct a RequestTask.
     *
     * @param socket_guard SocketGuard used for I/O.
     * @param ifindex Interface index associated with the request.
     * @param sequence Netlink sequence number for messages.
     */
    RequestTask(SocketGuard& socket_guard, uint16_t ifindex, uint32_t sequence) noexcept
        : socket_guard_{socket_guard}
        , ifindex_{ifindex}
        , sequence_{sequence} {}

    /** @brief Virtual destructor. */
    virtual ~RequestTask() {}

    /** @brief Run the request asynchronously and return the result.
     *
     * This co-routine will send the prepared request, read replies and return
     * the resulting expected<Result, std::error_code> when complete.
     */
    auto async_run() -> boost::asio::awaitable<std::expected<Result, std::error_code>> {
        impl().prepare_request();

        if (auto send_result = co_await send_request(); !send_result) {
            co_return std::unexpected(send_result.error());
        }

        co_return co_await read_loop();
    }

protected:
    auto socket() noexcept -> Socket& {
        return socket_guard_.socket();
    }

    auto sequence() const noexcept -> uint32_t {
        return sequence_;
    }

    auto ifindex() const noexcept -> uint16_t {
        return ifindex_;
    }

private:
    auto impl() noexcept -> Derived& {
        return static_cast<Derived&>(*this);
    }

    auto impl() const noexcept -> const Derived& {
        return static_cast<const Derived&>(*this);
    }

    auto send_request() -> boost::asio::awaitable<std::expected<void, std::error_code>> {
        const auto payload = impl().request_payload();
        size_t offset = 0;

        while (offset < payload.size()) {
            boost::system::error_code ec{};
            const auto sent = co_await socket().async_send(
                    boost::asio::buffer(payload.data() + offset, payload.size() - offset),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec) {
                co_return std::unexpected(
                        std::error_code{ec.value(), std::generic_category()});
            }

            offset += sent;
        }

        co_return std::expected<void, std::error_code>{};
    }

    auto read_loop() -> boost::asio::awaitable<std::expected<Result, std::error_code>> {
        while (true) {
            boost::system::error_code ec{};
            const auto bytes = co_await socket().async_receive(
                    boost::asio::buffer(receive_buffer_),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec) {
                co_return std::unexpected(
                        std::error_code{ec.value(), std::generic_category()});
            }

            if (bytes >= receive_buffer_.size()) {
            }

            auto remaining = static_cast<unsigned int>(bytes);
            const auto header_size = static_cast<unsigned int>(sizeof(nlmsghdr));
            const auto* header = reinterpret_cast<const nlmsghdr*>(receive_buffer_
                            .data());

            while (remaining >= header_size && NLMSG_OK(header, remaining)) {
                if (auto result = impl().process_message(*header)) {
                    co_return std::move(*result);
                }

                header = NLMSG_NEXT(header, remaining);
            }
        }
    }
};

} // namespace rtaco
} // namespace llmx
