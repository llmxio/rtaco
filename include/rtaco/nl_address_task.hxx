#pragma once

#include <cstdint>
#include <span>

#include <linux/if_addr.h>
#include <linux/netlink.h>

#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace rtaco {

struct AddressRequest {
    nlmsghdr header;
    ifaddrmsg message;
};

/** @brief Base task type for address-related netlink operations.
 *
 * `AddressTask` stores an address request (ifaddrmsg) and exposes
 * `request_payload()` for sending the serialized request. Derived classes
 * implement the concrete request preparation and response processing.
 */
template<typename Derived, typename Result>
class AddressTask : public RequestTask<Derived, Result> {
protected:
    AddressRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    /** @brief Get the serialized request payload for sending.
     *
     * @return A span referencing the request payload bytes suitable for send.
     */
    auto request_payload() const -> std::span<const uint8_t> {
        return {reinterpret_cast<const uint8_t*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request(uint16_t msg_flags = NLM_F_REQUEST | NLM_F_DUMP,
            uint8_t address_family = RTN_UNSPEC, uint8_t prefixlen = 0,
            uint8_t ifa_flags = 0, uint8_t scope = RT_SCOPE_UNIVERSE) {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(ifaddrmsg));
        request_.header.nlmsg_type = RTM_GETADDR;
        request_.header.nlmsg_flags = msg_flags;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.ifa_family = address_family;
        request_.message.ifa_prefixlen = prefixlen;
        request_.message.ifa_flags = ifa_flags;
        request_.message.ifa_scope = scope;
        request_.message.ifa_index = this->ifindex();
    }
};

} // namespace rtaco
} // namespace llmx
