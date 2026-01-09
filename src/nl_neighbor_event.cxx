#include "rtaco/nl_neighbor_event.hxx"

#include <arpa/inet.h>
#include <cstring>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto NeighborEvent::from_nlmsghdr(const nlmsghdr& header) -> NeighborEvent {
    NeighborEvent event{};
    event.type = header.nlmsg_type == RTM_NEWNEIGH ? NeighborEvent::Type::NEW_NEIGHBOR :
            header.nlmsg_type == RTM_DELNEIGH ? NeighborEvent::Type::DELETE_NEIGHBOR :
                                                NeighborEvent::Type::UNKNOWN;

    if (event.type == NeighborEvent::Type::UNKNOWN) {
        return event;
    }

    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ndmsg))) {
        event.type = NeighborEvent::Type::UNKNOWN;
        return event;
    }

    const auto* info = reinterpret_cast<const ndmsg*>(NLMSG_DATA(&header));
    if (info == nullptr) {
        event.type = NeighborEvent::Type::UNKNOWN;
        return event;
    }

    event.family = info->ndm_family;
    event.index = info->ndm_ifindex;
    event.state = static_cast<NeighborEvent::State>(info->ndm_state);
    event.flags = info->ndm_flags;
    event.neighbor_type = info->ndm_type;

    int attr_length = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(ndmsg)));
    if (attr_length <= 0) {
        return event;
    }

    auto* attr = reinterpret_cast<rtattr*>(
            reinterpret_cast<std::uintptr_t>(info) + NLMSG_ALIGN(sizeof(ndmsg)));
    for (; RTA_OK(attr, attr_length); attr = RTA_NEXT(attr, attr_length)) {
        switch (attr->rta_type) {
        case NDA_DST: event.address = attribute_address(*attr, event.family); break;
        case NDA_LLADDR: event.lladdr = attribute_hwaddr(*attr); break;
        default: break;
        }
    }

    return event;
}

} // namespace nl
} // namespace llmx
