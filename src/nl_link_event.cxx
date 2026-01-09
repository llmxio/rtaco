#include "rtaco/nl_link_event.hxx"

#include <linux/netlink.h>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto LinkEvent::from_nlmsghdr(const nlmsghdr& header) -> LinkEvent {
    using enum LinkEvent::Type;

    LinkEvent event{};
    switch (header.nlmsg_type) {
    case RTM_NEWLINK: event.type = NEW_LINK; break;
    case RTM_DELLINK: event.type = DELETE_LINK; break;
    default: event.type = UNKNOWN; break;
    }

    if (event.type == UNKNOWN) {
        return event;
    }

    const auto* info = get_msg_payload<ifinfomsg>(header);
    if (info == nullptr) {
        event.type = UNKNOWN;
        return event;
    }

    event.index = info->ifi_index;
    event.flags = info->ifi_flags;
    event.change = info->ifi_change;
    event.name = extract_ifname(header);

    return event;
}

} // namespace nl
} // namespace llmx
