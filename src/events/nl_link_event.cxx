#include "rtaco/events/nl_link_event.hxx"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/core/nl_common.hxx"

namespace llmx {
namespace rtaco {

auto LinkEvent::from_nlmsghdr(const nlmsghdr& header) -> LinkEvent {
    LinkEvent event{};
    switch (header.nlmsg_type) {
    case RTM_NEWLINK: event.type = Type::NEW_LINK; break;
    case RTM_DELLINK: event.type = Type::DELETE_LINK; break;
    default: event.type = Type::UNKNOWN; break;
    }

    if (event.type == Type::UNKNOWN) {
        return event;
    }

    const auto* info = get_msg_payload<ifinfomsg>(header);
    if (info == nullptr) {
        event.type = Type::UNKNOWN;
        return event;
    }

    event.index = info->ifi_index;
    event.flags = static_cast<Flags>(info->ifi_flags);
    event.change = info->ifi_change;
    event.name = extract_ifname(header);

    return event;
}

} // namespace rtaco
} // namespace llmx
