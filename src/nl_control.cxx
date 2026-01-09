#include "rtaco/nl_control.hxx"

#include <future>
#include <memory>
#include <utility>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>

#include "rtaco/nl_address_dump_task.hxx"
#include "rtaco/nl_neighbor_dump_task.hxx"
#include "rtaco/nl_neighbor_flush_task.hxx"
#include "rtaco/nl_neighbor_get_task.hxx"
#include "rtaco/nl_neighbor_probe_task.hxx"
#include "rtaco/nl_route_dump_task.hxx"
#include "rtaco/nl_socket_guard.hxx"

namespace llmx {
namespace nl {

Control::Control(boost::asio::io_context& io) noexcept
    : io_{io}
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
    RouteDumpTask task{socket_guard_->socket(), std::pmr::get_default_resource(), 0,
            sequence};

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
    AddressDumpTask task{socket_guard_->socket(), std::pmr::get_default_resource(), 0,
            sequence};

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
    NeighborDumpTask task{socket_guard_->socket(), std::pmr::get_default_resource(), 0,
            sequence};

    auto result = co_await task.async_run();

    if (!result) {
        co_return std::unexpected(result.error());
    }

    co_return result;
}

auto Control::flush_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> expected<void, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_flush_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_flush_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<void, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborFlushTask task{socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::probe_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> expected<void, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_probe_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_probe_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<void, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborProbeTask task{socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::get_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> expected<NeighborEvent, llmx_error_policy> {
    auto future = boost::asio::co_spawn(io_, async_get_neighbor(ifindex, address),
            boost::asio::use_future);

    return future.get();
}

auto Control::async_get_neighbor(uint16_t ifindex, const Ip6Address& address)
        -> boost::asio::awaitable<expected<NeighborEvent, llmx_error_policy>> {
    if (auto result = socket_guard_->ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborGetTask task{socket_guard_->socket(), ifindex, sequence, address};

    co_return co_await task.async_run();
}

void Control::stop() {
    socket_guard_->stop();
}

} // namespace nl
} // namespace llmx
