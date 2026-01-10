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
    boost::asio::io_context& io_;
    SocketGuard socket_guard_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    std::atomic_uint32_t sequence_{1U};
    std::stop_source stop_source_;
    std::mutex mutex_;
};

} // namespace nl
} // namespace llmx
