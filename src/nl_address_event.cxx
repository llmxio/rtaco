#include "rtaco/nl_address_event.hxx"

#include <arpa/inet.h>
#include <cstring>

#include "rtaco/nl_common.hxx"

namespace llmx {
namespace nl {

auto AddressEvent::from_nlmsghdr(const nlmsghdr& header) -> AddressEvent {
    AddressEvent event{};
    event.type = header.nlmsg_type == RTM_NEWADDR ? AddressEvent::Type::NEW_ADDRESS :
            header.nlmsg_type == RTM_DELADDR      ? AddressEvent::Type::DELETE_ADDRESS :
                                                    AddressEvent::Type::UNKNOWN;

    if (event.type == AddressEvent::Type::UNKNOWN) {
        return event;
    }

    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ifaddrmsg))) {
        event.type = AddressEvent::Type::UNKNOWN;
        return event;
    }

    const auto* info = reinterpret_cast<const ifaddrmsg*>(NLMSG_DATA(&header));
    if (info == nullptr) {
        event.type = AddressEvent::Type::UNKNOWN;
        return event;
    }

    event.family     = info->ifa_family;
    event.prefix_len = info->ifa_prefixlen;
    event.scope      = info->ifa_scope;
    event.flags      = info->ifa_flags;
    event.index      = static_cast<int>(info->ifa_index);

    int attr_length = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(ifaddrmsg)));
    if (attr_length <= 0) {
        return event;
    }

    const rtattr* attr = IFA_RTA(info);
    std::string local_address;
    std::string peer_address;
    std::string label;

    for (; RTA_OK(attr, attr_length); attr = RTA_NEXT(attr, attr_length)) {
        switch (attr->rta_type) {
        case IFA_LOCAL: local_address = attribute_address(*attr, event.family); break;
        case IFA_ADDRESS: peer_address = attribute_address(*attr, event.family); break;
        case IFA_LABEL: label = attribute_string(*attr); break;
        default: break;
        }
    }

    event.address = !local_address.empty() ? std::move(local_address) :
                                             std::move(peer_address);
    event.label   = std::move(label);

    return event;
}

} // namespace nl
} // namespace llmx
