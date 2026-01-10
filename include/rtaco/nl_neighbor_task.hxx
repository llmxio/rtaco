#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include <linux/rtnetlink.h>

#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace nl {

struct NeighborRequest {
    nlmsghdr header;
    ndmsg message;
    rtattr dst_attr;
    std::array<uint8_t, 16> dst;
};

template<typename Derived, typename Result>
class NeighborTask : public RequestTask<Derived, Result> {
protected:
    NeighborRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    auto request_payload() const -> std::span<const uint8_t> {
        return {reinterpret_cast<const uint8_t*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request(uint16_t msg_type, uint16_t msg_flags, uint16_t ndm_state,
            uint8_t ndm_flags, std::span<uint8_t, 16> address) {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(ndmsg));
        request_.header.nlmsg_type = msg_type;
        request_.header.nlmsg_flags = msg_flags;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.ndm_family = RTN_UNSPEC;
        request_.message.ndm_ifindex = this->ifindex();
        request_.message.ndm_state = ndm_state;
        request_.message.ndm_flags = ndm_flags;
        request_.message.ndm_type = RTN_UNSPEC;

        if (!this->ifindex()) { // TODO: make prettier
            return;
        }

        request_.dst_attr.rta_type = NDA_DST;
        request_.dst_attr.rta_len = RTA_LENGTH(request_.dst.size());

        std::memcpy(request_.dst.data(), address.data(), address.size());

        const auto payload = NLMSG_ALIGN(request_.header.nlmsg_len);
        request_.header.nlmsg_len = payload + RTA_ALIGN(request_.dst_attr.rta_len);
    }
};

} // namespace nl
} // namespace llmx
