#include "llmx/nl/netlink_listener.h"

#include <cstring>
#include <memory>
#include <system_error>
#include <utility>

#include <linux/neighbour.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/socket_option.hpp>
#include <boost/asio/error.hpp>

#include "llmx/core/io_pool.h"
#include "llmx/core/logger.h"
#include "llmx/core/utils.h"

namespace llmx {
namespace {

constexpr unsigned NETLINK_GROUPS = RTMGRP_LINK | RTMGRP_NEIGH | RTMGRP_IPV4_IFADDR |
        RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

constexpr size_t NEIGHBOR_MESSAGE_SPACE = NLMSG_SPACE(sizeof(ndmsg)) +
        RTA_SPACE(Ip6Addr::BYTES_SIZE);

} // namespace

namespace nl {

Listener::Listener(Context& ctx) noexcept
    : ctx_{ctx}
    , io_{IoPool::query()}
    , socket_{io_, "nl-listener"}
    , on_link_event_{IoPool::executor()}
    , on_address_event_{IoPool::executor()}
    , on_route_event_{IoPool::executor()}
    , on_neighbor_event_{IoPool::executor()} {}

Listener::~Listener() {
    stop();
}

auto Listener::running() const noexcept -> bool {
    return running_.load(std::memory_order_relaxed);
}

void Listener::start() {
    if (running_.load(std::memory_order_acquire)) {
        return;
    }

    open_socket();
    request_read();

    LOG(INFO) << "Netlink router started";
}

void Listener::stop() {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    if (auto result = socket_.cancel(); !result) {
        LOG(WARN) << "Failed to cancel netlink router socket: "
                  << result.error().message();
    }

    if (auto result = socket_.close(); !result) {
        LOG(WARN) << "Failed to close netlink router socket: "
                  << result.error().message();
    }

    running_.store(false, std::memory_order_release);

    LOG(INFO) << "Netlink listener shutdown complete.";
}

void Listener::open_socket() {
    socket_.open(NETLINK_ROUTE, NETLINK_GROUPS);
}

void Listener::request_read() {
    if (!running_.load(std::memory_order_acquire)) {
        running_.store(true, std::memory_order_release);
    }

    socket_.async_receive(boost::asio::buffer(buffer_),
            [this](const boost::system::error_code& ec, size_t bytes)
    { handle_read(ec, bytes); });
}

void Listener::handle_read(const boost::system::error_code& ec, size_t bytes) {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        LOG(ERROR) << "Netlink router read error: " << ec.message();
        request_read();
        return;
    }

    process_messages(std::span<const std::byte>(buffer_.data(), bytes));
    request_read();
}

void Listener::process_messages(std::span<const std::byte> data) {
    int remaining = static_cast<int>(data.size());
    const auto* header = reinterpret_cast<const nlmsghdr*>(data.data());

    while (remaining >= static_cast<int>(sizeof(nlmsghdr)) &&
            NLMSG_OK(header, remaining)) {
        handle_message(*header);
        header = NLMSG_NEXT(header, remaining);
    }

    if (remaining > 0) {
        LOG(WARN) << "Netlink router parsing left trailing bytes=" << remaining;
    }
}

void Listener::handle_message(const nlmsghdr& header) {
    switch (header.nlmsg_type) {
    case RTM_NEWLINK:
    case RTM_DELLINK:
        handle_link_message(header);
        break;
    case RTM_NEWADDR:
    case RTM_DELADDR:
        handle_address_message(header);
        break;
    case RTM_NEWROUTE:
    case RTM_DELROUTE:
        handle_route_message(header);
        break;
    case RTM_NEWNEIGH:
    case RTM_DELNEIGH:
        handle_neighbor_message(header);
        break;
    case NLMSG_DONE:
        LOG(DEBUG) << "Netlink router dump complete";
        break;
    case NLMSG_ERROR: {
        handle_error_message(header);
        break;
    }
    default:
        LOG(DEBUG) << "Netlink router message type=" << header.nlmsg_type << " ("
                   << type_to_string(header.nlmsg_type) << ")";
        break;
    }
}

void Listener::handle_error_message(const nlmsghdr& header) {
    if (const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
            err != nullptr) {
        if (err->error == 0) {
            LOG(DEBUG) << "Netlink router ack received";
        } else {
            LOG(ERROR) << "Netlink router error: code=" << err->error;
        }
    }
}

void Listener::handle_link_message(const nlmsghdr& header) {
    const auto event = LinkEvent::from_nlmsghdr(header);
    if (event.type == LinkEvent::Type::UNKNOWN) {
        LOG(WARN) << "Netlink router received unknown link event";
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
    const auto event = AddressEvent::from_nlmsghdr(header);
    if (event.type == AddressEvent::Type::UNKNOWN) {
        LOG(WARN) << "Netlink router received unknown address event";
        return;
    }

    on_address_event_(event);

    const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    const auto address = event.address.empty() ? std::string{"unknown"} : event.address;

    if (event.label.empty()) {
        LOG(DEBUG) << "Address event type=" << type_name << " index=" << event.index
                   << " family=" << static_cast<int>(event.family)
                   << " prefix_len=" << static_cast<int>(event.prefix_len)
                   << " address=" << address;
    } else {
        LOG(DEBUG) << "Address event type=" << type_name << " index=" << event.index
                   << " family=" << static_cast<int>(event.family)
                   << " prefix_len=" << static_cast<int>(event.prefix_len)
                   << " address=" << address << " label=" << event.label;
    }
}

void Listener::handle_route_message(const nlmsghdr& header) {
    const auto event = RouteEvent::from_nlmsghdr(header);

    if (event.type == RouteEvent::Type::UNKNOWN) {
        LOG(WARN) << "Netlink router received unknown route event";
        return;
    }

    on_route_event_(event);

    const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    const auto dst = event.dst.empty() ? std::string{"default"} : event.dst;
    const auto gateway = event.gateway.empty() ? std::string{"direct"} : event.gateway;

    std::string oif = event.oif;
    if (oif.empty()) {
        if (event.oif_index != 0U) {
            oif = std::to_string(event.oif_index);
        } else {
            oif = "unknown";
        }
    }

    LOG(DEBUG) << "Route event type=" << type_name << " table=" << event.table
               << " family=" << static_cast<int>(event.family) << " dst=" << dst << "/"
               << static_cast<int>(event.dst_prefix_len) << " gateway=" << gateway
               << " oif=" << oif << " priority=" << event.priority;
}

void Listener::handle_neighbor_message(const nlmsghdr& header) {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type == NeighborEvent::Type::UNKNOWN) {
        LOG(WARN) << "Netlink router received unknown neighbor event";
        return;
    }

    on_neighbor_event_(event);

    const auto type_name = type_to_string(static_cast<uint16_t>(header.nlmsg_type));
    const auto address = event.address.empty() ? std::string{"unknown"} : event.address;
    const auto lladdr = event.lladdr.empty() ? std::string{"unknown"} : event.lladdr;

    LOG(DEBUG) << "Neighbor event type=" << type_name << " index=" << event.index
               << " family=" << static_cast<int>(event.family)
               << " state=" << event.state_to_string()
               << " flags=" << static_cast<unsigned int>(event.flags)
               << " type=" << static_cast<unsigned int>(event.neighbor_type)
               << " address=" << address << " lladdr=" << lladdr;
}

} // namespace nl
} // namespace llmx
