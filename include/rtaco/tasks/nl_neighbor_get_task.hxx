#pragma once

#include <stdint.h>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <system_error>

#include "rtaco/events/nl_neighbor_event.hxx"
#include "rtaco/tasks/nl_neighbor_task.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that retrieves a single neighbor entry.
 *
 * Sends a lookup request for the given neighbor address on the specified
 * interface and returns the corresponding `NeighborEvent` if found.
 */
class NeighborGetTask : public NeighborTask<NeighborGetTask, NeighborEvent> {
    std::array<uint8_t, 16> address_;

public:
    /** @brief Construct a NeighborGetTask to retrieve a single neighbor entry.
     *
     * @param socket_guard Socket guard used for IO.
     * @param uint16_t Interface index.
     * @param sequence Netlink message sequence number.
     * @param address Address bytes identifying the neighbor.
     */
    NeighborGetTask(SocketGuard& socket_guard, uint16_t uint16_t, uint32_t sequence,
            std::span<uint8_t, 16> address);

    /** @brief Prepare the netlink request to get the neighbor. */
    void prepare_request();

    /** @brief Process a response message and extract a NeighborEvent.
     *
     * @param header Netlink message header.
     * @return Optional expected NeighborEvent or error.
     */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;

private:
    auto handle_done() -> std::expected<NeighborEvent, std::error_code>;
    auto handle_error(const nlmsghdr& header)
            -> std::expected<NeighborEvent, std::error_code>;
    auto handle_neighbor(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
