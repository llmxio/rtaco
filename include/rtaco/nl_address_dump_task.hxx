#pragma once

#include <cstdint>
#include <expected>
#include <memory_resource>
#include <optional>
#include <span>
#include <system_error>

#include <linux/if_addr.h>
#include <linux/netlink.h>

#include "rtaco/nl_address_event.hxx"
#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace rtaco {

class SocketGuard;

struct AddressRequest {
    nlmsghdr header;
    ifaddrmsg message;
};

class AddressDumpTask : public RequestTask<AddressDumpTask, AddressEventList> {
    AddressRequest request_;
    AddressEventList learned_;

public:
    AddressDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    auto request_payload() const -> std::span<const uint8_t>;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;

private:
    void build_request();

    auto handle_done() -> std::expected<AddressEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<AddressEventList, std::error_code>;

    auto dispatch_address(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
