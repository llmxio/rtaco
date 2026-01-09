#include "llmx/nl/netlink_control.h"

#include <future>
#include <memory>
#include <utility>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>

#include "llmx/core/io_pool.h"
#include "llmx/core/logger.h"
#include "llmx/nl/netlink_address_dump_task.h"
#include "llmx/nl/netlink_neighbor_dump_task.h"
#include "llmx/nl/netlink_neighbor_flush_task.h"
#include "llmx/nl/netlink_neighbor_get_task.h"
#include "llmx/nl/netlink_neighbor_probe_task.h"
#include "llmx/nl/netlink_route_dump_task.h"
#include "llmx/nl/netlink_socket_guard.h"

namespace llmx {
namespace nl {

Control::Control(Context& ctx) noexcept
    : ctx_{ctx}
    , io_{IoPool::query()}
    , socket_guard_{std::make_unique<SocketGuard>(io_, "nl-control")} {}

Control::~Control() = default;

auto Control::dump_routes() -> expected<RouteEventList, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_dump_routes(),
            boost::asio::use_future);
    return future.get();
}

auto Control::dump_addresses() -> expected<AddressEventList, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_dump_addresses(),
            boost::asio::use_future);
    return future.get();
}

auto Control::dump_neighbors() -> expected<NeighborEventList, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_dump_neighbors(),
            boost::asio::use_future);
    return future.get();
}

auto Control::async_dump_routes()
        -> boost::asio::awaitable<expected<RouteEventList, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    RouteDumpTask task{ctx_, socket_guard_->socket(), 0, sequence};

    auto result = co_await task.async_run();
    if (!result) {
        co_return std::unexpected(result.error());
    }

    co_return result;
}

auto Control::async_dump_addresses()
        -> boost::asio::awaitable<expected<AddressEventList, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    AddressDumpTask task{ctx_, socket_guard_->socket(), 0, sequence};

    auto result = co_await task.async_run();
    if (!result) {
        co_return std::unexpected(result.error());
    }

    co_return result;
}

auto Control::async_dump_neighbors()
        -> boost::asio::awaitable<expected<NeighborEventList, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborDumpTask task{ctx_, socket_guard_->socket(), 0, sequence};

    auto result = co_await task.async_run();

    if (!result) {
        co_return std::unexpected(result.error());
    }

    co_return result;
}

auto Control::flush_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> expected<void, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_flush_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_flush_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<void, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborFlushTask task{ctx_, socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::probe_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> expected<void, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_probe_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_probe_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<void, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborProbeTask task{ctx_, socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::get_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> expected<EtherAddr, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_get_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_get_neighbor(IfIndex ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<EtherAddr, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborGetTask task{ctx_, socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

void Control::stop() {
    socket_guard_->stop();
}

} // namespace nl
} // namespace llmx
