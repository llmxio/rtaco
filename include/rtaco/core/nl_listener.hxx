#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <span>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/system/error_code.hpp>

#include <linux/netlink.h>

#include "rtaco/core/nl_signal.hxx"
#include "rtaco/events/nl_address_event.hxx"
#include "rtaco/events/nl_link_event.hxx"
#include "rtaco/events/nl_neighbor_event.hxx"
#include "rtaco/events/nl_route_event.hxx"
#include "rtaco/socket/nl_socket_guard.hxx"

namespace llmx {
namespace rtaco {

/** @brief Asynchronous netlink message listener and event dispatcher.
 *
 * Listens on netlink multicast groups and dispatches typed events
 * (link/address/route/neighbor) via `Signal` instances. Manages a
 * `SocketGuard`, an internal read buffer and sequence numbering for
 * netlink messages.
 */
class Listener {
    static constexpr auto BUFFER_SIZE = 32U * 1024U;

public:
    using link_signal_t = Signal<void(const LinkEvent&)>;
    using address_signal_t = Signal<void(const AddressEvent&)>;
    using route_signal_t = Signal<void(const RouteEvent&)>;
    using neighbor_signal_t = Signal<void(const NeighborEvent&)>;
    using nlmsgerr_signal_t = Signal<void(const nlmsgerr&, const nlmsghdr&)>;

    /** @brief Construct a Listener bound to an io_context. */
    Listener(boost::asio::io_context& io) noexcept;

    /** @brief Destroy the Listener and stop any background activity. */
    ~Listener();

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener(Listener&&) noexcept = delete;
    Listener& operator=(Listener&&) noexcept = delete;

    /** @brief Start listening for netlink messages and dispatch events. */
    void start();

    /** @brief Stop listening and cancel outstanding operations. */
    void stop();

    /** @brief Check whether listener is currently running. */
    bool running() const noexcept;

    /** @brief Connect a handler to link events.
     *
     * @param slot Handler callable invoked when a link event is emitted.
     * @param policy Execution policy (Sync or Async).
     * @return A connection object that can be used to disconnect.
     */
    auto connect_to_event(link_signal_t::slot_t&& slot,
            ExecPolicy policy = ExecPolicy::Sync) -> boost::signals2::connection {
        return on_link_event_.connect(std::move(slot), policy);
    }

    /** @brief Connect a handler to address events. */
    auto connect_to_event(address_signal_t::slot_t&& slot,
            ExecPolicy policy = ExecPolicy::Sync) -> boost::signals2::connection {
        return on_address_event_.connect(std::move(slot), policy);
    }

    /** @brief Connect a handler to route events. */
    auto connect_to_event(route_signal_t::slot_t&& slot,
            ExecPolicy policy = ExecPolicy::Sync) -> boost::signals2::connection {
        return on_route_event_.connect(std::move(slot), policy);
    }

    /** @brief Connect a handler to neighbor events. */
    auto connect_to_event(neighbor_signal_t::slot_t&& slot,
            ExecPolicy policy = ExecPolicy::Sync) -> boost::signals2::connection {
        return on_neighbor_event_.connect(std::move(slot), policy);
    }

    /** @brief Connect a handler to raw netlink error messages. */
    auto connect_to_error(nlmsgerr_signal_t::slot_t&& slot,
            ExecPolicy policy = ExecPolicy::Sync) -> boost::signals2::connection {
        return on_nlmsgerr_event_.connect(std::move(slot), policy);
    }

private:
    boost::asio::io_context& io_;
    SocketGuard socket_guard_;

    link_signal_t on_link_event_;
    address_signal_t on_address_event_;
    route_signal_t on_route_event_;
    neighbor_signal_t on_neighbor_event_;
    nlmsgerr_signal_t on_nlmsgerr_event_;

    std::array<uint8_t, BUFFER_SIZE> buffer_{};
    std::atomic_uint32_t sequence_{1U};
    std::atomic_bool running_{false};

    auto open_socket() -> std::expected<void, std::error_code>;
    void request_read();
    void handle_read(const boost::system::error_code& ec, size_t bytes);
    void process_messages(std::span<const uint8_t> data);

    void handle_message(const nlmsghdr& header);
    void handle_error_message(const nlmsghdr& header);

    void handle_link_message(const nlmsghdr& header);
    void handle_address_message(const nlmsghdr& header);
    void handle_route_message(const nlmsghdr& header);
    void handle_neighbor_message(const nlmsghdr& header);
};

} // namespace rtaco
} // namespace llmx
