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

/** @brief Task that performs a full address dump from the kernel.
 *
 * Sends an `RTM_GETADDR` dump request and accumulates `AddressEvent`s as
 * responses are received. The task completes when an NLMSG_DONE message is
 * encountered and returns the collected `AddressEventList` or an error.
 */
class AddressDumpTask : public AddressTask<AddressDumpTask, AddressEventList> {
    AddressEventList learned_;

public:
    /** @brief Construct an AddressDumpTask.
     *
     * @param socket_guard Reference to the socket guard used for sending and
     *        receiving netlink requests.
     * @param pmr Memory resource used for allocations of the returned event list.
     * @param ifindex Interface index to target (0 = all interfaces).
     * @param sequence Netlink message sequence number for requests.
     */
    AddressDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    /** @brief Prepare the netlink request to perform an address dump. */
    void prepare_request();

    /** @brief Process a received netlink message for address dump responses.
     *
     * @param header Reference to the received netlink message header.
     * @return Optional expected containing the accumulated AddressEventList on
     *         completion, or an error_code on failure.
     */
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
