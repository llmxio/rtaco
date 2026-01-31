#pragma once

#include <stdint.h>
#include <expected>
#include <memory_resource>
#include <optional>
#include <system_error>

#include "rtaco/nl_route_task.hxx"
#include "rtaco/nl_route_event.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

/** @brief Task that performs a full route table dump from the kernel.
 *
 * Sends an `RTM_GETROUTE` dump request and collects `RouteEvent` messages
 * emitted by the kernel until completion, returning the collected list.
 */
class RouteDumpTask : public RouteTask<RouteDumpTask, RouteEventList> {
    RouteEventList learned_;

public:
    /** @brief Construct a RouteDumpTask.
     *
     * @param socket_guard Socket guard used for netlink I/O.
     * @param pmr Memory resource for event list allocations.
     * @param ifindex Target interface index (0 = all).
     * @param sequence Netlink message sequence number.
     */
    RouteDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

    /** @brief Prepare the netlink request to dump routes. */
    void prepare_request();

    /** @brief Process a received netlink message for route dump responses.
     *
     * @param header Netlink message header.
     * @return Optional expected with RouteEventList on completion or an error.
     */
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<RouteEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<RouteEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<RouteEventList, std::error_code>;

    auto dispatch_route(const nlmsghdr& header)
            -> std::optional<std::expected<RouteEventList, std::error_code>>;
};

} // namespace rtaco
} // namespace llmx
