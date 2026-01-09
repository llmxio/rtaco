#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>

#include "llmx/core/context.h"
#include "llmx/core/error.h"
#include "llmx/core/signal.h"
#include "llmx/core/utils.h"
#include "llmx/nl/netlink_event.h"
#include "llmx/nl/netlink_protocol.h"
#include "llmx/nl/netlink_socket.h"
#include "llmx/net/ip6.h"

namespace llmx {
namespace nl {

class Listener {
    static constexpr auto BUFFER_SIZE = 32U * 1024U;

public:
    using LinkSignal = Signal<void(const LinkEvent&)>;
    using AddressSignal = Signal<void(const AddressEvent&)>;
    using RouteSignal = Signal<void(const RouteEvent&)>;
    using NeighborSignal = Signal<void(const NeighborEvent&)>;

    Listener(Context& ctx) noexcept;
    ~Listener();

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;
    Listener(Listener&&) noexcept = default;
    Listener& operator=(Listener&&) noexcept = default;

    void start();
    void stop();

    bool running() const noexcept;

    auto connect_to_event(auto&& slot, ExecPolicy policy = ExecPolicy::Sync)
            -> boost::signals2::connection {
        Visitor action{
                [&](LinkSignal::slot_type&& s)
        { return on_link_event_.connect(std::move(s), policy); },
                [&](AddressSignal::slot_type&& s)
        { return on_address_event_.connect(std::move(s), policy); },
                [&](RouteSignal::slot_type&& s)
        { return on_route_event_.connect(std::move(s), policy); },
                [&](NeighborSignal::slot_type&& s)
        { return on_neighbor_event_.connect(std::move(s), policy); },
        };

        return action(std::forward<decltype(slot)>(slot));
    }

private:
    Context& ctx_;

    boost::asio::io_context& io_;
    nl::Socket socket_;
    std::atomic_uint32_t sequence_{1U};

    LinkSignal on_link_event_;
    AddressSignal on_address_event_;
    RouteSignal on_route_event_;
    NeighborSignal on_neighbor_event_;

    std::array<std::byte, BUFFER_SIZE> buffer_{};
    std::atomic_bool running_{false};

    void open_socket();
    void request_read();
    void handle_read(const boost::system::error_code& ec, size_t bytes);
    void process_messages(std::span<const std::byte> data);

    void handle_message(const nlmsghdr& header);
    void handle_error_message(const nlmsghdr& header);
    void handle_link_message(const nlmsghdr& header);
    void handle_address_message(const nlmsghdr& header);
    void handle_route_message(const nlmsghdr& header);
    void handle_neighbor_message(const nlmsghdr& header);

    auto send_neighbor_request(uint16_t type, IfIndex ifindex, uint16_t state,
            uint8_t flags, std::span<const uint8_t> dst) -> expected<void>;

    void process_route_dump(const nlmsghdr& header, size_t& learned);
};

} // namespace nl
} // namespace llmx
