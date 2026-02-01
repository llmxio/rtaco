#pragma once

#include <cstdint>
#include <expected>
#include <memory_resource>
#include <optional>
#include <system_error>

#include "rtaco/events/nl_link_event.hxx"
#include "rtaco/tasks/nl_link_task.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that performs a full link dump from the kernel.
 *
 * Issues an `RTM_GETLINK` dump and collects `LinkEvent` entries as messages
 * are received. Returns the accumulated `LinkEventList` on completion.
 */
class LinkDumpTask : public LinkTask<LinkDumpTask, LinkEventList> {
    LinkEventList learned_;

public:
    /** @brief Construct a LinkDumpTask.
     *
     * @param socket_guard Reference to the socket guard.
     * @param pmr Memory resource used for event list allocations.
     * @param ifindex Interface index to target (0 = all).
     * @param sequence Netlink message sequence number.
     */
    LinkDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    /** @brief Prepare the netlink request to dump links. */
    void prepare_request();

    /** @brief Process a received netlink message for link dump responses.
     *
     * @param header Netlink message header.
     * @return Optional expected with LinkEventList on completion or an error.
     */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<LinkEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<LinkEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<LinkEventList, std::error_code>;

    auto dispatch_link(const nlmsghdr& header)
            -> std::optional<std::expected<LinkEventList, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
