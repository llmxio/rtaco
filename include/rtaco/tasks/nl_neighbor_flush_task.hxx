#pragma once

#include <stdint.h>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <system_error>

#include "rtaco/tasks/nl_neighbor_task.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that flushes (deletes) a specific neighbor entry.
 *
 * Constructs and sends an `RTM_GETNEIGH`/`RTM_NEWNEIGH`-style request to
 * remove the specified neighbor and processes the kernel's response.
 */
class NeighborFlushTask : public NeighborTask<NeighborFlushTask, void> {
    std::array<uint8_t, 16> address_;

public:
    /** @brief Construct a NeighborFlushTask to remove a neighbor entry.
     *
     * @param socket_guard Socket guard used to send the request.
     * @param ifindex Interface index.
     * @param sequence Netlink message sequence number.
     * @param address Address bytes to identify the neighbor.
     */
    NeighborFlushTask(SocketGuard& socket_guard, uint16_t ifindex, uint32_t sequence,
            std::span<uint8_t, 16> address);

    /** @brief Prepare the netlink request to flush the neighbor entry. */
    void prepare_request();

    /** @brief Process a response message for the flush operation.
     *
     * @param header Netlink message header.
     * @return Optional expected void or error on failure.
     */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<void, std::error_code>>;

private:
    auto handle_error(const nlmsghdr& header) -> std::expected<void, std::error_code>;
};

} // namespace rtaco
} // namespace llmx
