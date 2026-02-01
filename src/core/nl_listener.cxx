#include "rtaco/core/nl_listener.hxx"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <system_error>

#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>

#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/events/nl_address_event.hxx"
#include "rtaco/events/nl_link_event.hxx"
#include "rtaco/events/nl_route_event.hxx"
#include "rtaco/events/nl_neighbor_event.hxx"

namespace llmx {
namespace rtaco {

namespace asio = boost::asio;

Listener::Listener(asio::io_context& io) noexcept
    : io_{io}
    , socket_guard_{io_, "nl-listener"}
    , on_link_event_{io_.get_executor()}
    , on_address_event_{io_.get_executor()}
    , on_route_event_{io_.get_executor()}
    , on_neighbor_event_{io_.get_executor()}
    , on_nlmsgerr_event_{io_.get_executor()} {}

Listener::~Listener() {
    stop();
}

auto Listener::running() const noexcept -> bool {
    return running_.load(std::memory_order_acquire);
}

void Listener::start() {
    if (running()) {
        return;
    }

    open_socket();
    request_read();
}

void Listener::stop() {
    if (!running()) {
        return;
    }

    running_.store(false, std::memory_order_release);
    socket_guard_.stop();
}

auto Listener::open_socket() -> std::expected<void, std::error_code> {
    if (auto result = socket_guard_.ensure_open(); !result) {
        std::cerr << "Failed to open netlink socket: " << result.error().message()
                  << "\n";
        return result;
    }

    return {};
}

void Listener::request_read() {
    if (!running()) {
        running_.store(true, std::memory_order_release);
    }

    socket_guard_.socket().async_receive(asio::buffer(buffer_),
            [this](const auto& ec, size_t bytes) { handle_read(ec, bytes); });
}

void Listener::handle_read(const boost::system::error_code& ec, size_t bytes) {
    if (!running()) {
        return;
    }

    if (ec == asio::error::operation_aborted) {
        return;
    }

    if (ec) {
        request_read();
        return;
    }

    process_messages(std::span<const uint8_t>(buffer_.data(), bytes));
    request_read();
}

void Listener::process_messages(std::span<const uint8_t> data) {
    auto remaining = static_cast<unsigned int>(data.size());
    const auto header_size = static_cast<unsigned int>(sizeof(nlmsghdr));
    const auto* header = reinterpret_cast<const nlmsghdr*>(data.data());

    while (remaining >= header_size && NLMSG_OK(header, remaining)) {
        handle_message(*header);
        header = NLMSG_NEXT(header, remaining);
    }

    if (remaining > 0) {
    }
}

void Listener::handle_message(const nlmsghdr& header) {
    switch (header.nlmsg_type) {
    case RTM_NEWLINK:
    case RTM_DELLINK: handle_link_message(header); break;
    case RTM_NEWADDR:
    case RTM_DELADDR: handle_address_message(header); break;
    case RTM_NEWROUTE:
    case RTM_DELROUTE: handle_route_message(header); break;
    case RTM_NEWNEIGH:
    case RTM_DELNEIGH: handle_neighbor_message(header); break;
    case NLMSG_DONE: break;
    case NLMSG_ERROR: {
        handle_error_message(header);
        break;
    }
    default: break;
    }
}

void Listener::handle_error_message(const nlmsghdr& header) {
    if (const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
            err != nullptr) {
        on_nlmsgerr_event_(*err, header);
    }
}

void Listener::handle_link_message(const nlmsghdr& header) {
    const auto event = LinkEvent::from_nlmsghdr(header);
    if (event.type == LinkEvent::Type::UNKNOWN) {
        return;
    }

    on_link_event_(event);

    // auto link_state_string = [](uint32_t flags) -> std::string {
    //     return (flags & IFF_RUNNING) != 0U ? "running" : "down";
    // };

    // const auto type_name =
    // type_to_string(static_cast<uint16_t>(header.nlmsg_type)); LOG(DEBUG) <<  "Link
    // event type=" << type_name << " index=" << event.index
    //            << " state=" << link_state_string(event.flags)
    //            << " name=" << (event.name.empty() ? std::string{"unknown"} :
    //            event.name);
}

void Listener::handle_address_message(const nlmsghdr& header) {
    auto event = AddressEvent::from_nlmsghdr(header);

    if (event.type == AddressEvent::Type::UNKNOWN) {
        return;
    }

    on_address_event_(event);

    // const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    // const auto address = event.address.empty() ? std::string{"unknown"} :
    // event.address;
}

void Listener::handle_route_message(const nlmsghdr& header) {
    const auto event = RouteEvent::from_nlmsghdr(header);

    if (event.type == RouteEvent::Type::UNKNOWN) {
        return;
    }

    on_route_event_(event);

    // const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    // const auto dst = event.dst.empty() ? std::string{"default"} : event.dst;
    // const auto gateway = event.gateway.empty() ? std::string{"direct"} : event.gateway;

    // std::string oif = event.oif;
    // if (oif.empty()) {
    //     if (event.oif_index != 0U) {
    //         oif = std::to_string(event.oif_index);
    //     } else {
    //         oif = "unknown";
    //     }
    // }
}

void Listener::handle_neighbor_message(const nlmsghdr& header) {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type == NeighborEvent::Type::UNKNOWN) {
        return;
    }

    on_neighbor_event_(event);

    // const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    // const auto address = event.address.empty() ? std::string{"unknown"} :
    // event.address; const auto lladdr = event.lladdr.empty() ? std::string{"unknown"} :
    // event.lladdr;

    // (void)type_name;
    // (void)address;
    // (void)lladdr;
    // (void)event;
}

} // namespace rtaco
} // namespace llmx
