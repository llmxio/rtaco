#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <optional>
#include <span>
#include <system_error>
#include <vector>

#include <linux/netlink.h>

#include <boost/asio/async_result.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>

#include "llmx/core/asio.h"
#include "llmx/core/context.h"
#include "llmx/core/error.h"
#include "llmx/core/utils.h"
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
            } -> std::same_as<std::optional<expected<Result, llmx_error_policy>>>;
        };

template<typename Derived, typename Result>
class RequestTask {
    static constexpr size_t MAX_RESPONSE_BYTES = 64U * 1024U;
    std::array<std::byte, MAX_RESPONSE_BYTES> receive_buffer_{};

    Context& ctx_;
    Socket& socket_;
    IfIndex ifindex_;
    uint32_t sequence_;

public:
    RequestTask(Context& ctx, Socket& socket, IfIndex ifindex, uint32_t sequence) noexcept
        : ctx_{ctx}
        , socket_{socket}
        , ifindex_{ifindex}
        , sequence_{sequence} {}

    virtual ~RequestTask() {}

    auto async_run() -> boost::asio::awaitable<expected<Result, llmx_error_policy>> {
        // static_assert(request_behavior<Derived, Result>,
        //         "Derived must implement the netlink request behavior interface");

        impl().prepare_request();

        if (auto send_result = co_await send_request(); !send_result) {
            co_return std::unexpected(send_result.error());
        }

        // co_return co_await read_loop();
        co_return co_await with_timeout(read_loop(), std::chrono::seconds(3));
    }

protected:
    auto context() noexcept -> Context& {
        return ctx_;
    }

    auto context() const noexcept -> const Context& {
        return ctx_;
    }

    auto socket() noexcept -> Socket& {
        return socket_;
    }

    auto socket() const noexcept -> const Socket& {
        return socket_;
    }

    auto sequence() noexcept -> uint32_t {
        return sequence_;
    }

    auto ifindex() noexcept -> IfIndex {
        return ifindex_;
    }

private:
    auto impl() noexcept -> Derived& {
        return static_cast<Derived&>(*this);
    }

    auto impl() const noexcept -> const Derived& {
        return static_cast<const Derived&>(*this);
    }

    auto send_request() -> boost::asio::awaitable<expected<void, llmx_error_policy>> {
        const auto payload = impl().request_payload();
        size_t offset      = 0;

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

        co_return expected<void>{};
    }

    auto read_loop() -> boost::asio::awaitable<expected<Result, llmx_error_policy>> {
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

            auto remaining     = static_cast<int>(bytes);
            auto header_size   = static_cast<int>(sizeof(nlmsghdr));
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
