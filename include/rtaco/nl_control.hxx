#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <expected>
#include <span>
#include <stop_token>
#include <system_error>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "rtaco/nl_address_event.hxx"
#include "rtaco/nl_neighbor_event.hxx"
#include "rtaco/nl_route_event.hxx"
#include "rtaco/nl_socket_guard.hxx"

namespace llmx {
namespace nl {

class Control {
public:
    Control(boost::asio::io_context& io) noexcept;
    ~Control();

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;
    Control(Control&&) = delete;
    Control& operator=(Control&&) = delete;

    auto dump_routes() -> std::expected<RouteEventList, std::error_code>;
    auto dump_addresses() -> std::expected<AddressEventList, std::error_code>;
    auto dump_neighbors() -> std::expected<NeighborEventList, std::error_code>;

    auto probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> std::expected<void, std::error_code>;

    auto flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> std::expected<void, std::error_code>;

    auto get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> std::expected<NeighborEvent, std::error_code>;

    auto async_dump_routes()
            -> boost::asio::awaitable<std::expected<RouteEventList, std::error_code>>;

    auto async_dump_addresses()
            -> boost::asio::awaitable<std::expected<AddressEventList, std::error_code>>;

    auto async_dump_neighbors()
            -> boost::asio::awaitable<std::expected<NeighborEventList, std::error_code>>;

    auto async_probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<std::expected<void, std::error_code>>;

    auto async_flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<std::expected<void, std::error_code>>;

    auto async_get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<std::expected<NeighborEvent, std::error_code>>;

    void stop();

private:
    using route_result = std::expected<RouteEventList, std::error_code>;
    using address_result = std::expected<AddressEventList, std::error_code>;
    using neighbor_result = std::expected<NeighborEvent, std::error_code>;
    using neighbor_list_result = std::expected<NeighborEventList, std::error_code>;
    using void_result = std::expected<void, std::error_code>;

    auto acquire_socket_token() -> boost::asio::awaitable<void>;

    auto async_dump_routes_impl() -> boost::asio::awaitable<route_result>;
    auto async_dump_addresses_impl() -> boost::asio::awaitable<address_result>;
    auto async_dump_neighbors_impl() -> boost::asio::awaitable<neighbor_list_result>;

    auto async_probe_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result>;

    auto async_flush_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result>;

    auto async_get_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<neighbor_result>;

    boost::asio::io_context& io_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer gate_;
    SocketGuard socket_guard_;
    std::atomic_uint32_t sequence_{1U};
};

} // namespace nl
} // namespace llmx
