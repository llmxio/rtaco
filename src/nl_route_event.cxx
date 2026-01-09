#include "rtaco/nl_route_event.hxx"

#include <linux/netlink.h>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto RouteEvent::from_nlmsghdr(const nlmsghdr& header) -> RouteEvent {
    RouteEvent event{};

    switch (header.nlmsg_type) {
    case RTM_NEWROUTE: event.type = RouteEvent::Type::NEW_ROUTE; break;
    case RTM_DELROUTE: event.type = RouteEvent::Type::DELETE_ROUTE; break;
    default: event.type = RouteEvent::Type::UNKNOWN; break;
    }

    if (event.type == RouteEvent::Type::UNKNOWN) {
        return event;
    }

    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(rtmsg))) {
        event.type = RouteEvent::Type::UNKNOWN;
        return event;
    }

    const auto* info = reinterpret_cast<const rtmsg*>(NLMSG_DATA(&header));
    if (info == nullptr) {
        event.type = RouteEvent::Type::UNKNOWN;
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

    int attr_length = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(rtmsg)));
    if (attr_length > 0) {
        const rtattr* attr = RTM_RTA(info);
        for (; RTA_OK(attr, attr_length); attr = RTA_NEXT(attr, attr_length)) {
            switch (attr->rta_type) {
            case RTA_TABLE: event.table = attribute_uint32(*attr); break;
            case RTA_DST: event.dst = attribute_address(*attr, event.family); break;
            case RTA_SRC: event.src = attribute_address(*attr, event.family); break;
            case RTA_GATEWAY:
                event.gateway = attribute_address(*attr, event.family);
                break;
            case RTA_PREFSRC:
                event.prefsrc = attribute_address(*attr, event.family);
                break;
            case RTA_OIF: event.oif_index = attribute_uint32(*attr); break;
            case RTA_PRIORITY: event.priority = attribute_uint32(*attr); break;
            default: break;
            }
        }
    }

    if (event.oif_index != 0U) {
        event.oif = std::to_string(event.oif_index);
    }

    return event;
}

} // namespace nl
} // namespace llmx
