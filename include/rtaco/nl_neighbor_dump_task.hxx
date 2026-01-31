#pragma once

#include <cstdint>
#include <expected>
#include <memory_resource>
#include <optional>
#include <system_error>

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that performs a complete neighbor table dump.
 *
 * Sends an `RTM_GETNEIGH` dump request and gathers `NeighborEvent` entries
 * until the dump completes, returning the accumulated list or an error.
 */
class NeighborDumpTask : public NeighborTask<NeighborDumpTask, NeighborEventList> {
    NeighborEventList learned_;

public:
    /** @brief Construct a NeighborDumpTask.
     *
     * @param socket_guard Reference to the socket guard used for netlink I/O.
     * @param pmr Memory resource for event list allocations.
     * @param ifindex Target interface index (0 = all).
     * @param sequence Netlink message sequence number.
     */
    NeighborDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    /** @brief Prepare the netlink request to dump neighbor entries. */
    void prepare_request();

    /** @brief Process a received neighbor-related netlink message.
     *
     * @param header Netlink message header.
     * @return Optional expected with NeighborEventList on completion or an error.
     */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<NeighborEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<NeighborEventList, std::error_code>;

    auto dispatch_neighbor(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEventList, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
