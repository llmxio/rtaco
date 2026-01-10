#include "rtaco/nl_address_event.hxx"

#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <string>
#include <linux/rtnetlink.h>
#include <utility>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto AddressEvent::from_nlmsghdr(const nlmsghdr& header) -> AddressEvent {
    using enum AddressEvent::Type;

    AddressEvent event{};
    switch (header.nlmsg_type) {
    case RTM_NEWADDR: event.type = NEW_ADDRESS; break;
    case RTM_DELADDR: event.type = DELETE_ADDRESS; break;
    default: event.type = UNKNOWN; break;
    }

    if (event.type == UNKNOWN) {
        return event;
    }

    const auto* info = get_msg_payload<ifaddrmsg>(header);
    if (info == nullptr) {
        event.type = UNKNOWN;
        return event;
    }

    event.family = info->ifa_family;
    event.prefix_len = info->ifa_prefixlen;
    event.scope = info->ifa_scope;
    event.flags = info->ifa_flags;
    event.index = static_cast<int>(info->ifa_index);

    std::string local_address;
    std::string peer_address;
    std::string label;

    for_each_attr(header, info, [&](const rtattr* attr)
    {
        switch (attr->rta_type) {
        case IFA_LOCAL: local_address = attribute_address(*attr, event.family); break;
        case IFA_ADDRESS: peer_address = attribute_address(*attr, event.family); break;
        case IFA_LABEL: label = attribute_string(*attr); break;
        default: break;
        }
    });

    event.address = !local_address.empty() ? std::move(local_address) :
                                             std::move(peer_address);
    event.label = std::move(label);

    return event;
}

} // namespace nl
} // namespace llmx
