#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <linux/netlink.h>

#include "llmx/core/context.h"
#include "llmx/core/error.h"
#include "llmx/core/expected_ext.h"
#include "llmx/net/ether.h"
#include "llmx/net/ip6.h"
#include "llmx/nl/netlink_socket_guard.h"

namespace llmx {
namespace nl {

class Control {
public:
    Control(Context& ctx) noexcept;
    ~Control();

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;
    Control(Control&&) = delete;
    Control& operator=(Control&&) = delete;

    auto dump_routes() -> expected<RouteEventList, llmx_error_policy>;
    auto dump_addresses() -> expected<AddressEventList, llmx_error_policy>;
    auto dump_neighbors() -> expected<NeighborEventList, llmx_error_policy>;

    auto probe_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> expected<void, llmx_error_policy>;

    auto flush_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> expected<void, llmx_error_policy>;

    auto get_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> expected<EtherAddr, llmx_error_policy>;

    auto async_dump_routes()
            -> boost::asio::awaitable<expected<RouteEventList, llmx_error_policy>>;

    auto async_dump_addresses()
            -> boost::asio::awaitable<expected<AddressEventList, llmx_error_policy>>;

    auto async_dump_neighbors()
            -> boost::asio::awaitable<expected<NeighborEventList, llmx_error_policy>>;

    auto async_probe_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> boost::asio::awaitable<expected<void, llmx_error_policy>>;

    auto async_flush_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> boost::asio::awaitable<expected<void, llmx_error_policy>>;

    auto async_get_neighbor(IfIndex ifindex, const Ip6Address& address)
            -> boost::asio::awaitable<expected<EtherAddr, llmx_error_policy>>;

    void stop();

private:
    Context& ctx_;
    boost::asio::io_context& io_;
    std::unique_ptr<SocketGuard> socket_guard_;
    std::atomic_uint32_t sequence_{1U};
    std::stop_source stop_source_;
};

} // namespace nl
} // namespace llmx
