#include "rtaco/core/nl_listener.hxx"

#include <array>
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

namespace {
auto has_payload(const nlmsghdr& header, size_t min_len) noexcept -> bool {
    if (header.nlmsg_len < NLMSG_LENGTH(min_len)) {
        return false;
    }

    return NLMSG_PAYLOAD(&header, 0) >= min_len;
}
} // namespace

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

    if (auto rc = open_socket(); !rc) {
        return;
    }

    running_.store(true, std::memory_order_release);

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
    if (auto rc = socket_guard_.ensure_open(); !rc) {
        std::cerr << "Failed to open netlink socket: " << rc.error().message() << "\n";
        return rc;
    }

    return {};
}

void Listener::request_read() {
    if (!running()) {
        return;
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
        std::cerr << "Warning: " << remaining
                  << " bytes of unread data remaining in netlink message buffer\n";
    }

    request_read();
}

void Listener::handle_message(const nlmsghdr& header) {
    using HandlerEntry12 = std::pair<size_t, void (Listener::*)(const nlmsghdr&)>;

    static std::unordered_map<int, HandlerEntry12> handlers;

    handlers[RTM_NEWLINK] = {sizeof(ifinfomsg), &Listener::handle_link_message};
    handlers[RTM_DELLINK] = {sizeof(ifinfomsg), &Listener::handle_link_message};
    handlers[RTM_NEWADDR] = {sizeof(ifaddrmsg), &Listener::handle_address_message};
    handlers[RTM_DELADDR] = {sizeof(ifaddrmsg), &Listener::handle_address_message};
    handlers[RTM_NEWROUTE] = {sizeof(rtmsg), &Listener::handle_route_message};
    handlers[RTM_DELROUTE] = {sizeof(rtmsg), &Listener::handle_route_message};
    handlers[RTM_NEWNEIGH] = {sizeof(ndmsg), &Listener::handle_neighbor_message};
    handlers[RTM_DELNEIGH] = {sizeof(ndmsg), &Listener::handle_neighbor_message};
    handlers[NLMSG_ERROR] = {sizeof(nlmsgerr), &Listener::handle_error_message};

    if (!handlers.contains(header.nlmsg_type)) {
        return;
    }

    auto handler = handlers.at(header.nlmsg_type);

    if (handler.first > 0 && !has_payload(header, handler.first)) {
        return;
    }

    (this->*handler.second)(header);
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
}

void Listener::handle_address_message(const nlmsghdr& header) {
    auto event = AddressEvent::from_nlmsghdr(header);

    if (event.type == AddressEvent::Type::UNKNOWN) {
        return;
    }

    on_address_event_(event);
}

void Listener::handle_route_message(const nlmsghdr& header) {
    const auto event = RouteEvent::from_nlmsghdr(header);

    if (event.type == RouteEvent::Type::UNKNOWN) {
        return;
    }

    on_route_event_(event);
}

void Listener::handle_neighbor_message(const nlmsghdr& header) {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type == NeighborEvent::Type::UNKNOWN) {
        return;
    }

    on_neighbor_event_(event);
}

} // namespace rtaco
} // namespace llmx
