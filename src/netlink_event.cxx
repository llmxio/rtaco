#include "rtaco/nl_event.hxx"

#include <arpa/inet.h>
#include <cstring>

#include "llmx/net/utils.h"

namespace llmx {
namespace nl {

auto extract_ifname(const nlmsghdr& header) -> std::string {
    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ifinfomsg))) {
        return {};
    }

    const auto* info = reinterpret_cast<const ifinfomsg*>(NLMSG_DATA(&header));
    int attr_length  = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(ifinfomsg)));

    if (attr_length <= 0) {
        return {};
    }

    for (auto* attr = IFLA_RTA(info); RTA_OK(attr, attr_length);
            attr    = RTA_NEXT(attr, attr_length)) {
        if (attr->rta_type != IFLA_IFNAME) {
            continue;
        }

        const auto payload = static_cast<size_t>(RTA_PAYLOAD(attr));
        if (payload == 0U) {
            continue;
        }

        const auto* data = reinterpret_cast<const char*>(RTA_DATA(attr));
        if (data == nullptr) {
            continue;
        }

        return trim_string(data);
    }

    return {};
}

auto attribute_string(const rtattr& attr) -> std::string {
    const auto payload = static_cast<size_t>(RTA_PAYLOAD(&attr));
    if (payload == 0U) {
        return {};
    }

    const auto* data = reinterpret_cast<const char*>(RTA_DATA(&attr));
    if (data == nullptr) {
        return {};
    }

    return trim_string(data);
}

auto attribute_address(const rtattr& attr, uint8_t family) -> std::string {
    int address_family = 0;

    switch (family) {
    case AF_INET: address_family = AF_INET; break;
    case AF_INET6: address_family = AF_INET6; break;
    default: return {};
    }

    const void* data = RTA_DATA(&attr);
    if (data == nullptr) {
        return {};
    }

    std::array<char, INET6_ADDRSTRLEN> buffer{};
    if (::inet_ntop(address_family, data, buffer.data(), buffer.size()) == nullptr) {
        return {};
    }

    return trim_string(buffer);
}

auto attribute_uint32(const rtattr& attr) -> uint32_t {
    if (RTA_PAYLOAD(&attr) < sizeof(uint32_t)) {
        return 0U;
    }

    uint32_t value{};
    std::memcpy(&value, RTA_DATA(&attr), sizeof(value));
    return value;
}

auto attribute_hwaddr(const rtattr& attr) -> std::string {
    const auto payload = static_cast<size_t>(RTA_PAYLOAD(&attr));
    if (payload == 0U) {
        return {};
    }

    const auto* data = reinterpret_cast<const uint8_t*>(RTA_DATA(&attr));
    if (data == nullptr) {
        return {};
    }

    std::string value;
    value.reserve(payload * 3U);

    constexpr char kHex[] = "0123456789abcdef";
    for (size_t i = 0; i < payload; ++i) {
        if (i != 0U) {
            value.push_back(':');
        }
        const auto byte = data[i];
        value.push_back(kHex[(byte >> 4U) & 0x0FU]);
        value.push_back(kHex[byte & 0x0FU]);
    }

    return value;
}

auto LinkEvent::from_nlmsghdr(const nlmsghdr& header) -> LinkEvent {
    LinkEvent event{};
    event.type = header.nlmsg_type == RTM_NEWLINK ? LinkEvent::Type::NEW_LINK :
            header.nlmsg_type == RTM_DELLINK      ? LinkEvent::Type::DELETE_LINK :
                                                    LinkEvent::Type::UNKNOWN;

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

auto RouteEvent::from_nlmsghdr(const nlmsghdr& header) -> RouteEvent {
    RouteEvent event{};
    event.type = header.nlmsg_type == RTM_NEWROUTE ? RouteEvent::Type::NEW_ROUTE :
            header.nlmsg_type == RTM_DELROUTE      ? RouteEvent::Type::DELETE_ROUTE :
                                                     RouteEvent::Type::UNKNOWN;

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

    event.family         = info->rtm_family;
    event.dst_prefix_len = info->rtm_dst_len;
    event.src_prefix_len = info->rtm_src_len;
    event.scope          = info->rtm_scope;
    event.protocol       = info->rtm_protocol;
    event.route_type     = info->rtm_type;
    event.flags          = info->rtm_flags;
    event.table          = info->rtm_table;

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
        event.oif = ifindex_to_string(event.oif_index);
    }

    return event;
}

} // namespace nl
} // namespace llmx
