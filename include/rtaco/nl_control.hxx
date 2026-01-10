#pragma once

#include <atomic>
#include <cstdint>
#include <expected>
#include <span>
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
namespace rtaco {

class Control {
    using route_list_result_t = std::expected<RouteEventList, std::error_code>;
    using address_list_result_t = std::expected<AddressEventList, std::error_code>;
    using neighbor_result_t = std::expected<NeighborEvent, std::error_code>;
    using neighbor_list_result = std::expected<NeighborEventList, std::error_code>;
    using void_result_t = std::expected<void, std::error_code>;

public:
    Control(boost::asio::io_context& io) noexcept;
    ~Control();

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;
    Control(Control&&) = delete;
    Control& operator=(Control&&) = delete;

    auto dump_routes() -> route_list_result_t;
    auto dump_addresses() -> address_list_result_t;
    auto dump_neighbors() -> neighbor_list_result;

    auto async_dump_routes() -> boost::asio::awaitable<route_list_result_t>;
    auto async_dump_addresses() -> boost::asio::awaitable<address_list_result_t>;
    auto async_dump_neighbors() -> boost::asio::awaitable<neighbor_list_result>;

    auto probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> void_result_t;

    auto flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> void_result_t;

    auto get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> neighbor_result_t;

    auto async_probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    auto async_flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    auto async_get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<neighbor_result_t>;

    void stop();

private:
    auto acquire_socket_token() -> boost::asio::awaitable<void>;

    auto async_dump_routes_impl() -> boost::asio::awaitable<route_list_result_t>;
    auto async_dump_addresses_impl() -> boost::asio::awaitable<address_list_result_t>;
    auto async_dump_neighbors_impl() -> boost::asio::awaitable<neighbor_list_result>;

    auto async_probe_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    auto async_flush_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    auto async_get_neighbor_impl(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<neighbor_result_t>;

    boost::asio::io_context& io_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer gate_;
    SocketGuard socket_guard_;
    std::atomic_uint32_t sequence_{1U};
};

} // namespace rtaco
} // namespace llmx
