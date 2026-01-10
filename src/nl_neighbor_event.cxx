#include "rtaco/nl_neighbor_event.hxx"

#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto NeighborEvent::from_nlmsghdr(const nlmsghdr& header) -> NeighborEvent {
    using enum NeighborEvent::Type;

    NeighborEvent event{};
    switch (header.nlmsg_type) {
    case RTM_NEWNEIGH: event.type = NEW_NEIGHBOR; break;
    case RTM_DELNEIGH: event.type = DELETE_NEIGHBOR; break;
    default: event.type = UNKNOWN; break;
    }

    if (event.type == UNKNOWN) {
        return event;
    }

    const auto* info = get_msg_payload<ndmsg>(header);
    if (info == nullptr) {
        event.type = UNKNOWN;
        return event;
    }

    event.family = info->ndm_family;
    event.index = info->ndm_ifindex;
    event.state = static_cast<NeighborEvent::State>(info->ndm_state);
    event.flags = info->ndm_flags;
    event.neighbor_type = info->ndm_type;

    for_each_attr(header, info, [&](const rtattr* attr)
    {
        switch (attr->rta_type) {
        case NDA_DST: event.address = attribute_address(*attr, event.family); break;
        case NDA_LLADDR: event.lladdr = attribute_hwaddr(*attr); break;
        default: break;
        }
    });

    return event;
}

} // namespace nl
} // namespace llmx
