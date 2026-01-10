#include "rtaco/nl_route_event.hxx"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <string>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto RouteEvent::from_nlmsghdr(const nlmsghdr& header) -> RouteEvent {
    using enum RouteEvent::Type;

    RouteEvent event{};
    switch (header.nlmsg_type) {
    case RTM_NEWROUTE: event.type = NEW_ROUTE; break;
    case RTM_DELROUTE: event.type = DELETE_ROUTE; break;
    default: event.type = UNKNOWN; break;
    }

    if (event.type == UNKNOWN) {
        return event;
    }

    const auto* info = get_msg_payload<rtmsg>(header);
    if (info == nullptr) {
        event.type = UNKNOWN;
        return event;
    }

    event.family = info->rtm_family;
    event.dst_prefix_len = info->rtm_dst_len;
    event.src_prefix_len = info->rtm_src_len;
    event.scope = info->rtm_scope;
    event.protocol = info->rtm_protocol;
    event.route_type = info->rtm_type;
    event.flags = info->rtm_flags;
    event.table = info->rtm_table;

    for_each_attr(header, info, [&](const rtattr* attr)
    {
        switch (attr->rta_type) {
        case RTA_TABLE: event.table = attribute_uint32(*attr); break;
        case RTA_DST: event.dst = attribute_address(*attr, event.family); break;
        case RTA_SRC: event.src = attribute_address(*attr, event.family); break;
        case RTA_GATEWAY: event.gateway = attribute_address(*attr, event.family); break;
        case RTA_PREFSRC: event.prefsrc = attribute_address(*attr, event.family); break;
        case RTA_OIF: event.oif_index = attribute_uint32(*attr); break;
        case RTA_PRIORITY: event.priority = attribute_uint32(*attr); break;
        default: break;
        }
    });

    if (event.oif_index != 0U) {
        event.oif = std::to_string(event.oif_index);
    }

    return event;
}

} // namespace nl
} // namespace llmx
