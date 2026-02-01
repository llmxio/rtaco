#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <linux/rtnetlink.h>

#include "rtaco/tasks/nl_request_task.hxx"

namespace llmx {
namespace rtaco {

struct RouteRequest {
    nlmsghdr header;
    rtmsg message;
};

/** @brief Base task type for route-related netlink operations.
 *
 * `RouteTask` holds the route-specific request (rtmsg) and supplies
 * `request_payload()` for sending. Derived classes supply message preparation
 * and message processing for dumps, gets, and related operations.
 */
template<typename Derived, typename Result>
class RouteTask : public RequestTask<Derived, Result> {
protected:
    RouteRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    /** @brief Get the serialized request payload for the route request. */
    auto request_payload() const -> std::span<const uint8_t> {
        return {reinterpret_cast<const uint8_t*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request(uint16_t msg_flags, uint8_t address_family, uint8_t route_table,
            uint8_t scope, uint8_t protocol) {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(rtmsg));
        request_.header.nlmsg_type = RTM_GETROUTE;
        request_.header.nlmsg_flags = msg_flags;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.rtm_family = address_family;
        request_.message.rtm_table = route_table;
        request_.message.rtm_scope = scope;
        request_.message.rtm_protocol = protocol;
        request_.message.rtm_type = RTN_UNSPEC;
    }
};

} // namespace rtaco
} // namespace llmx
