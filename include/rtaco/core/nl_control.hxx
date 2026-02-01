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

#include "rtaco/events/nl_address_event.hxx"
#include "rtaco/events/nl_link_event.hxx"
#include "rtaco/events/nl_neighbor_event.hxx"
#include "rtaco/events/nl_route_event.hxx"
#include "rtaco/socket/nl_socket_guard.hxx"

namespace llmx {
namespace rtaco {

/** @brief High-level control interface for kernel netlink operations.
 *
 * The `Control` class provides synchronous and asynchronous methods to query
 * kernel networking state (routes, addresses, links, neighbors) and to perform
 * neighbor-related operations such as probe, flush, and get. It owns a
 * `SocketGuard`, manages sequencing for netlink requests, and exposes both
 * blocking and awaitable APIs to callers.
 */
class Control {
    using route_list_result_t = std::expected<RouteEventList, std::error_code>;
    using address_list_result_t = std::expected<AddressEventList, std::error_code>;
    using link_list_result_t = std::expected<LinkEventList, std::error_code>;
    using neighbor_result_t = std::expected<NeighborEvent, std::error_code>;
    using neighbor_list_result = std::expected<NeighborEventList, std::error_code>;
    using void_result_t = std::expected<void, std::error_code>;

public:
    /** @brief Construct a Control instance attached to an io_context.
     *
     * @param io The Boost.Asio io_context used for async operations.
     */
    Control(boost::asio::io_context& io) noexcept;

    /** @brief Destroy the Control object and release resources. */
    ~Control();

    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;
    Control(Control&&) = delete;
    Control& operator=(Control&&) = delete;

    /** @brief Synchronously dump routes from the kernel.
     *
     * @return Expected RouteEventList or an error_code on failure.
     */
    auto dump_routes() -> route_list_result_t;

    /** @brief Synchronously dump addresses from the kernel. */
    auto dump_addresses() -> address_list_result_t;

    /** @brief Synchronously dump links from the kernel. */
    auto dump_links() -> link_list_result_t;

    /** @brief Synchronously dump neighbor entries from the kernel. */
    auto dump_neighbors() -> neighbor_list_result;

    /** @brief Asynchronously dump routes.
     *
     * @return Awaitable that yields the route list result.
     */
    auto async_dump_routes() -> boost::asio::awaitable<route_list_result_t>;

    /** @brief Asynchronously dump addresses. */
    auto async_dump_addresses() -> boost::asio::awaitable<address_list_result_t>;

    /** @brief Asynchronously dump links. */
    auto async_dump_links() -> boost::asio::awaitable<link_list_result_t>;

    /** @brief Asynchronously dump neighbors. */
    auto async_dump_neighbors() -> boost::asio::awaitable<neighbor_list_result>;

    /** @brief Probe a neighbor entry (synchronous).
     *
     * @param ifindex Interface index to probe on.
     * @param address IPv6/IPv4 address bytes (16-byte span).
     * @return Expected void or error on failure.
     */
    auto probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> void_result_t;

    /** @brief Flush a neighbour entry (synchronous). */
    auto flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> void_result_t;

    /** @brief Get a neighbor entry synchronously. */
    auto get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> neighbor_result_t;

    /** @brief Asynchronously probe a neighbor. */
    auto async_probe_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    /** @brief Asynchronously flush a neighbor. */
    auto async_flush_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<void_result_t>;

    /** @brief Asynchronously get a neighbor. */
    auto async_get_neighbor(uint16_t ifindex, std::span<uint8_t, 16> address)
            -> boost::asio::awaitable<neighbor_result_t>;

    /** @brief Stop ongoing operations and release control resources. */
    void stop();

private:
    auto acquire_socket_token() -> boost::asio::awaitable<void>;

    auto async_dump_routes_impl() -> boost::asio::awaitable<route_list_result_t>;
    auto async_dump_addresses_impl() -> boost::asio::awaitable<address_list_result_t>;
    auto async_dump_links_impl() -> boost::asio::awaitable<link_list_result_t>;
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
