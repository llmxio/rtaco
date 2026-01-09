#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <linux/rtnetlink.h>

#include "llmx/core/error.h"
#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace nl {

struct RouteRequest {
    nlmsghdr header;
    rtmsg message;
};

template<typename Derived, typename Result>
class RouteTask : public RequestTask<Derived, Result> {
protected:
    RouteRequest request_{};

public:
    using RequestTask<Derived, Result>::RequestTask;

    auto request_payload() const -> std::span<const std::byte> {
        return {reinterpret_cast<const std::byte*>(&request_), request_.header.nlmsg_len};
    }

protected:
    void build_request() {
        request_.header.nlmsg_len = NLMSG_LENGTH(sizeof(rtmsg));
        request_.header.nlmsg_type = RTM_GETROUTE;
        request_.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
        request_.header.nlmsg_seq = this->sequence();
        request_.header.nlmsg_pid = 0;

        request_.message.rtm_family = AF_INET6;
        request_.message.rtm_table = RT_TABLE_MAIN;
        request_.message.rtm_scope = RT_SCOPE_UNIVERSE;
        request_.message.rtm_protocol = RTPROT_UNSPEC;
        request_.message.rtm_type = RTN_UNSPEC;
    }
};

} // namespace nl
} // namespace llmx
