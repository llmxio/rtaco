#pragma once

#include <cstdint>
#include <span>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace rtaco {

struct LinkRequest {
    nlmsghdr header;
    ifinfomsg message;
};

/** @brief Base task type for link-related netlink operations.
 *
 * `LinkTask` provides storage for a link-specific request (ifinfomsg) and
 * a helper `request_payload()` for sending. Derived types implement
 * `prepare_request()` and `process_message()` to handle the specific
 * operation semantics (dump/get/etc.).
 */
template<typename Derived, typename Result>
class LinkTask : public RequestTask<Derived, Result> {
protected:
    LinkRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    /** @brief Get the serialized payload for the link request. */
    auto request_payload() const -> std::span<const uint8_t> {
        return {reinterpret_cast<const uint8_t*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request(uint16_t msg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            uint8_t family = AF_UNSPEC, uint16_t type = 0, uint32_t flags = 0,
            uint32_t change = 0) {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg));
        request_.header.nlmsg_type = RTM_GETLINK;
        request_.header.nlmsg_flags = msg_flags;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.ifi_family = family;
        request_.message.ifi_type = type;
        request_.message.ifi_index = this->ifindex();
        request_.message.ifi_flags = flags;
        request_.message.ifi_change = change;
    }
};

} // namespace rtaco
} // namespace llmx
