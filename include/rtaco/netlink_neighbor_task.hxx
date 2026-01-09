#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include <linux/rtnetlink.h>

#include "llmx/net/ip6.h"
#include "llmx/nl/netlink_request_task.h"
#include "llmx/core/types.h"

namespace llmx {
namespace nl {

struct NeighborRequest {
    nlmsghdr header;
    ndmsg message;
    rtattr dst_attr;
    std::array<uint8_t, Ip6Addr::BYTES_SIZE> dst;
};

template<typename Derived, typename Result>
class NeighborTask : public RequestTask<Derived, Result> {
protected:
    NeighborRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    auto request_payload() const -> std::span<const std::byte> {
        return {reinterpret_cast<const std::byte*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request(uint16_t msg_type, uint16_t msg_flags, uint16_t ndm_state,
            uint8_t ndm_flags, const Ip6Address& address) {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(ndmsg));
        request_.header.nlmsg_type = msg_type;
        request_.header.nlmsg_flags = msg_flags;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.ndm_family = AF_INET6;
        request_.message.ndm_ifindex = static_cast<int>(this->ifindex());
        request_.message.ndm_state = ndm_state;
        request_.message.ndm_flags = ndm_flags;
        request_.message.ndm_type = RTN_UNSPEC;

        if (!this->ifindex()) { // TODO: make prettier
            return;
        }

        request_.dst_attr.rta_type = NDA_DST;
        request_.dst_attr
                .rta_len = static_cast<uint16_t>(RTA_LENGTH(request_.dst.size()));

        const auto address_bytes = address.to_bytes();
        std::memcpy(request_.dst.data(), address_bytes.data(), address_bytes.size());

        const auto aligned_payload = NLMSG_ALIGN(request_.header.nlmsg_len);
        request_.header
                .nlmsg_len = aligned_payload + RTA_ALIGN(request_.dst_attr.rta_len);
    }
};

} // namespace nl
} // namespace llmx
