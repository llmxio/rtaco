#include "rtaco/nl_event.hxx"

#include <arpa/inet.h>
#include <cstring>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto LinkEvent::from_nlmsghdr(const nlmsghdr& header) -> LinkEvent {
    LinkEvent event{};

    switch (header.nlmsg_type) {
    case RTM_NEWLINK: event.type = LinkEvent::Type::NEW_LINK; break;
    case RTM_DELLINK: event.type = LinkEvent::Type::DELETE_LINK; break;
    default: event.type = LinkEvent::Type::UNKNOWN; break;
    }

    if (event.type == LinkEvent::Type::UNKNOWN) {
        return event;
    }

    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ifinfomsg))) {
        event.type = LinkEvent::Type::UNKNOWN;
        return event;
    }

    const auto* info = reinterpret_cast<const ifinfomsg*>(NLMSG_DATA(&header));
    if (info == nullptr) {
        event.type = LinkEvent::Type::UNKNOWN;
        return event;
    }

    event.index  = info->ifi_index;
    event.flags  = info->ifi_flags;
    event.change = info->ifi_change;
    event.name   = extract_ifname(header);

    return event;
}

} // namespace nl
} // namespace llmx
