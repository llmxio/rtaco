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
#include "rtaco/nl_address_task.hxx"

namespace llmx {
namespace rtaco {

class SocketGuard;

class AddressDumpTask : public AddressTask<AddressDumpTask, AddressEventList> {
    AddressEventList learned_;

public:
    AddressDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<AddressEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<AddressEventList, std::error_code>;

    auto dispatch_address(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
