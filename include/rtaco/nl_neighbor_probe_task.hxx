#pragma once

#include <stdint.h>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <system_error>

#include "rtaco/nl_neighbor_task.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that triggers a neighbor probe (solicit) for an address.
 *
 * Requests the kernel to probe the neighbor (e.g., send an ARP/ND solicitation)
 * and processes the completion or error responses.
 */
class NeighborProbeTask : public NeighborTask<NeighborProbeTask, void> {
    std::array<uint8_t, 16> address_;

public:
    /** @brief Construct a NeighborProbeTask to probe a neighbor address.
     *
     * @param socket_guard Socket guard used for IO.
     * @param uint16_t Interface index.
     * @param sequence Netlink message sequence number.
     * @param address Address bytes to probe.
     */
    NeighborProbeTask(SocketGuard& socket_guard, uint16_t uint16_t, uint32_t sequence,
            std::span<uint8_t, 16> address);

    /** @brief Prepare the netlink request to probe the neighbor. */
    void prepare_request();

    /** @brief Process a response message for the probe operation. */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<void, std::error_code>>;

private:
    auto handle_error(const nlmsghdr& header) -> std::expected<void, std::error_code>;
};

} // namespace rtaco
} // namespace llmx
