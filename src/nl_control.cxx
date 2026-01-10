#include "rtaco/nl_control.hxx"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <expected>
#include <future>
#include <memory_resource>
#include <span>
#include <stdexcept>
#include <system_error>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/system/error_code.hpp>

#include "rtaco/nl_address_dump_task.hxx"
#include "rtaco/nl_address_event.hxx"
#include "rtaco/nl_neighbor_dump_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"
#include "rtaco/nl_neighbor_flush_task.hxx"
#include "rtaco/nl_neighbor_get_task.hxx"
#include "rtaco/nl_neighbor_probe_task.hxx"
#include "rtaco/nl_route_dump_task.hxx"
#include "rtaco/nl_route_event.hxx"

namespace llmx {
namespace nl {

namespace asio = boost::asio;

Control::Control(asio::io_context& io) noexcept
    : io_{io}
    , strand_{asio::make_strand(io_)}
    , gate_{io_}
    , socket_guard_{io_, "nl-control"} {
    gate_.expires_at(asio::steady_timer::time_point::min());
}

Control::~Control() = default;

auto Control::dump_routes() -> std::expected<RouteEventList, std::error_code> {
    auto future = asio::co_spawn(strand_, async_dump_routes_impl(), asio::use_future);
    return future.get();
}

auto Control::dump_addresses() -> std::expected<AddressEventList, std::error_code> {
    auto future = asio::co_spawn(strand_, async_dump_addresses_impl(), asio::use_future);
    return future.get();
}

auto Control::dump_neighbors() -> std::expected<NeighborEventList, std::error_code> {
    auto future = asio::co_spawn(strand_, async_dump_neighbors_impl(), asio::use_future);
    return future.get();
}

auto Control::async_dump_routes()
        -> asio::awaitable<std::expected<RouteEventList, std::error_code>> {
    co_return co_await asio::co_spawn(strand_, async_dump_routes_impl(),
            asio::use_awaitable);
}

auto Control::async_dump_addresses()
        -> asio::awaitable<std::expected<AddressEventList, std::error_code>> {
    co_return co_await asio::co_spawn(strand_, async_dump_addresses_impl(),
            asio::use_awaitable);
}

auto Control::async_dump_neighbors()
        -> asio::awaitable<std::expected<NeighborEventList, std::error_code>> {
    co_return co_await asio::co_spawn(strand_, async_dump_neighbors_impl(),
            asio::use_awaitable);
}

auto Control::flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> std::expected<void, std::error_code> {
    auto future = asio::co_spawn(strand_, async_flush_neighbor_impl(ifindex, address),
            asio::use_future);

    return future.get();
}

auto Control::async_flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<std::expected<void, std::error_code>> {
    co_return co_await asio::co_spawn(strand_,
            async_flush_neighbor_impl(ifindex, address), asio::use_awaitable);
}

auto Control::probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> std::expected<void, std::error_code> {
    auto future = asio::co_spawn(strand_, async_probe_neighbor_impl(ifindex, address),
            asio::use_future);

    return future.get();
}

auto Control::async_probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<std::expected<void, std::error_code>> {
    co_return co_await asio::co_spawn(strand_,
            async_probe_neighbor_impl(ifindex, address), asio::use_awaitable);
}

auto Control::get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> std::expected<NeighborEvent, std::error_code> {
    auto future = asio::co_spawn(strand_, async_get_neighbor_impl(ifindex, address),
            asio::use_future);

    return future.get();
}

auto Control::async_get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<std::expected<NeighborEvent, std::error_code>> {
    co_return co_await asio::co_spawn(strand_, async_get_neighbor_impl(ifindex, address),
            asio::use_awaitable);
}

void Control::stop() {
    socket_guard_.stop();
}

auto Control::async_dump_routes_impl() -> asio::awaitable<route_result> {
    co_await acquire_socket_token();
    SocketGuard guard{io_, "nl-control-route"};

    if (auto result = guard.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    RouteDumpTask task{guard, std::pmr::get_default_resource(), 0, sequence};

    co_return co_await task.async_run();
}

auto Control::async_dump_addresses_impl() -> asio::awaitable<address_result> {
    co_await acquire_socket_token();

    SocketGuard guard{io_, "nl-control-address"};

    if (auto result = guard.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    AddressDumpTask task{guard, std::pmr::get_default_resource(), 0, sequence};

    co_return co_await task.async_run();
}

auto Control::async_dump_neighbors_impl() -> asio::awaitable<neighbor_list_result> {
    co_await acquire_socket_token();

    SocketGuard guard{io_, "nl-control-neighbor"};

    if (auto result = guard.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborDumpTask task{guard, std::pmr::get_default_resource(), 0, sequence};

    co_return co_await task.async_run();
}

auto Control::async_probe_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<void_result> {
    co_await acquire_socket_token();

    if (auto result = socket_guard_.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborProbeTask task{socket_guard_, ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::async_flush_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<void_result> {
    co_await acquire_socket_token();

    if (auto result = socket_guard_.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborFlushTask task{socket_guard_, ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::async_get_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
        -> asio::awaitable<neighbor_result> {
    co_await acquire_socket_token();

    if (auto result = socket_guard_.ensure_open(); !result) {
        co_return std::unexpected(result.error());
    }

    auto sequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    NeighborGetTask task{socket_guard_, ifindex, sequence, address};

    co_return co_await task.async_run();
}

auto Control::acquire_socket_token() -> asio::awaitable<void> {
    auto next = gate_.expiry() + std::chrono::nanoseconds{1};
    auto now = asio::steady_timer::clock_type::now();

    if (next <= now) {
        next = now + std::chrono::nanoseconds{1};
    }

    gate_.expires_at(next);

    boost::system::error_code ec;
    co_await gate_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

    if (ec && ec != asio::error::operation_aborted) {
        throw std::runtime_error("gate wait failed: " + ec.message());
    }
}

} // namespace nl
} // namespace llmx
