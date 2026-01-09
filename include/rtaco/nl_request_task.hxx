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

#include "rtaco/nl_socket.hxx"

namespace llmx {
namespace nl {

template<typename Derived, typename Result>
concept request_behavior =
        requires(Derived& derived, const Derived& const_derived, const nlmsghdr& header) {
            { derived.prepare_request() } -> std::same_as<void>;
            {
                const_derived.request_payload()
            } -> std::same_as<std::span<const std::byte>>;
            {
                derived.process_message(header)
            } -> std::same_as<std::optional<std::expected<Result, std::error_code>>>;
        };

template<typename Derived, typename Result>
class RequestTask {
    static constexpr size_t MAX_RESPONSE_BYTES = 64U * 1024U;
    std::array<std::byte, MAX_RESPONSE_BYTES> receive_buffer_{};

    Socket& socket_;
    uint16_t ifindex_;
    uint32_t sequence_;

public:
    RequestTask(Socket& socket, uint16_t ifindex, uint32_t sequence) noexcept
        : socket_{socket}
        , ifindex_{ifindex}
        , sequence_{sequence} {}

    virtual ~RequestTask() {}

    auto async_run() -> boost::asio::awaitable<std::expected<Result, std::error_code>> {
        // static_assert(request_behavior<Derived, Result>,
        //         "Derived must implement the netlink request behavior interface");

        impl().prepare_request();

        if (auto send_result = co_await send_request(); !send_result) {
            co_return std::unexpected(send_result.error());
        }

        // TODO: timeout value configurable?
        co_return co_await read_loop();
    }

protected:
    auto socket() noexcept -> Socket& {
        return socket_;
    }

    auto socket() const noexcept -> const Socket& {
        return socket_;
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
            const auto sent = co_await socket_.async_send(
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
            const auto bytes = co_await socket_.async_receive(
                    boost::asio::buffer(receive_buffer_),
                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec) {
                co_return std::unexpected(
                        std::error_code{ec.value(), std::generic_category()});
            }

            if (bytes >= receive_buffer_.size()) {
            }

            auto remaining = static_cast<int>(bytes);
            auto header_size = static_cast<int>(sizeof(nlmsghdr));
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

} // namespace nl
} // namespace llmx
