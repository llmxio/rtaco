#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <unordered_map>

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>

namespace llmx {
namespace nl {

// Remove trailing NUL characters from a string view and return an owning string.
inline constexpr auto trim_string(std::string_view sv) -> std::string {
    while (!sv.empty() && sv.back() == '\0') {
        sv.remove_suffix(1);
    }
    return std::string(sv);
}

template<size_t N>
inline constexpr auto trim_string(const std::array<char, N>& arr) -> std::string {
    return trim_string(std::string_view{arr.data(), arr.size()});
}

inline auto extract_ifname(const nlmsghdr& header) -> std::string {
    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ifinfomsg))) {
        return {};
    }

    const auto* info = reinterpret_cast<const ifinfomsg*>(NLMSG_DATA(&header));
    int attr_length = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(ifinfomsg)));

    if (attr_length <= 0) {
        return {};
    }

    for (auto* attr = IFLA_RTA(info); RTA_OK(attr, attr_length);
            attr = RTA_NEXT(attr, attr_length)) {
        if (attr->rta_type != IFLA_IFNAME) {
            continue;
        }

        const auto payload = static_cast<size_t>(RTA_PAYLOAD(attr));
        if (payload == 0U) {
            continue;
        }

        const auto* buffer = reinterpret_cast<const char*>(RTA_DATA(attr));
        if (buffer == nullptr) {
            continue;
        }

        return trim_string(buffer);
    }

    return {};
}

inline auto attribute_string(const rtattr& attr) -> std::string {
    const auto payload = static_cast<size_t>(RTA_PAYLOAD(&attr));
    if (payload == 0U) {
        return {};
    }

    const auto* buffer = reinterpret_cast<const char*>(RTA_DATA(&attr));
    if (buffer == nullptr) {
        return {};
    }

    return trim_string(buffer);
}

inline auto attribute_address(const rtattr& attr, uint8_t family) -> std::string {
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

inline auto attribute_uint32(const rtattr& attr) -> uint32_t {
    if (RTA_PAYLOAD(&attr) < sizeof(uint32_t)) {
        return 0U;
    }

    uint32_t value{};
    std::memcpy(&value, RTA_DATA(&attr), sizeof(value));
    return value;
}

inline auto attribute_hwaddr(const rtattr& attr) -> std::string {
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

template<typename T>
concept IsEnumeration = std::is_enum_v<std::remove_cvref_t<T>> ||
        std::is_scoped_enum_v<std::remove_cvref_t<T>>;

template<typename T>
concept IsNetlinkEvent = IsEnumeration<T> || std::integral<std::remove_cvref_t<T>>;

inline auto type_to_string(IsNetlinkEvent auto&& type) noexcept -> std::string_view {
    uint16_t type_value{};

    if constexpr (IsEnumeration<decltype(type)>) {
        type_value = static_cast<uint16_t>(std::to_underlying(type));
    } else {
        type_value = static_cast<uint16_t>(type);
    }

    static const std::unordered_map<uint16_t, std::string_view> type_names{
            {RTM_NEWLINK, "RTM_NEWLINK"},
            {RTM_GETLINK, "RTM_GETLINK"},
            {RTM_DELLINK, "RTM_DELLINK"},
            {RTM_NEWADDR, "RTM_NEWADDR"},
            {RTM_GETADDR, "RTM_GETADDR"},
            {RTM_DELADDR, "RTM_DELADDR"},
            {RTM_NEWROUTE, "RTM_NEWROUTE"},
            {RTM_GETROUTE, "RTM_GETROUTE"},
            {RTM_DELROUTE, "RTM_DELROUTE"},
            {RTM_NEWNEIGH, "RTM_NEWNEIGH"},
            {RTM_GETNEIGH, "RTM_GETNEIGH"},
            {RTM_DELNEIGH, "RTM_DELNEIGH"},
            {NLMSG_NOOP, "NLMSG_NOOP"},
            {NLMSG_ERROR, "NLMSG_ERROR"},
            {NLMSG_DONE, "NLMSG_DONE"},
            {NLMSG_OVERRUN, "NLMSG_OVERRUN"},
    };

    if (type_names.contains(type_value)) {
        return type_names.at(type_value);
    }

    return "UNKNOWN";
}

inline auto family_to_string(uint8_t family) noexcept -> std::string_view {
    static const std::unordered_map<uint8_t, std::string_view> family_names{
            {AF_UNSPEC, "AF_UNSPEC"},
            {AF_UNIX, "AF_UNIX"},
            {AF_INET, "AF_INET"},
            {AF_INET6, "AF_INET6"},
            {AF_NETLINK, "AF_NETLINK"},
            {AF_PACKET, "AF_PACKET"},
            {AF_BRIDGE, "AF_BRIDGE"},
            {AF_MPLS, "AF_MPLS"},
            {AF_CAN, "AF_CAN"},
            {AF_TIPC, "AF_TIPC"},
            {AF_BLUETOOTH, "AF_BLUETOOTH"},
            {AF_VSOCK, "AF_VSOCK"},
            {AF_XDP, "AF_XDP"},
    };

    if (family_names.contains(family)) {
        return family_names.at(family);
    }

    return "UNKNOWN";
}

inline auto protocol_to_string(uint8_t protocol) noexcept -> std::string_view {
    static const std::unordered_map<uint8_t, std::string_view> protocol_names{
            {RTPROT_UNSPEC, "RTPROT_UNSPEC"},
            {RTPROT_REDIRECT, "RTPROT_REDIRECT"},
            {RTPROT_KERNEL, "RTPROT_KERNEL"},
            {RTPROT_BOOT, "RTPROT_BOOT"},
            {RTPROT_STATIC, "RTPROT_STATIC"},
            {RTPROT_GATED, "RTPROT_GATED"},
            {RTPROT_RA, "RTPROT_RA"},
            {RTPROT_MRT, "RTPROT_MRT"},
            {RTPROT_ZEBRA, "RTPROT_ZEBRA"},
            {RTPROT_BIRD, "RTPROT_BIRD"},
            {RTPROT_DNROUTED, "RTPROT_DNROUTED"},
            {RTPROT_XORP, "RTPROT_XORP"},
            {RTPROT_NTK, "RTPROT_NTK"},
            {RTPROT_DHCP, "RTPROT_DHCP"},
            {RTPROT_MROUTED, "RTPROT_MROUTED"},
            {RTPROT_BABEL, "RTPROT_BABEL"},
            {RTPROT_BGP, "RTPROT_BGP"},
            {RTPROT_OPENR, "RTPROT_OPENR"},
    };

    if (protocol_names.contains(protocol)) {
        return protocol_names.at(protocol);
    }

    return "UNKNOWN";
}

inline auto route_type_to_string(uint8_t route_type) noexcept -> std::string_view {
    static const std::unordered_map<uint8_t, std::string_view> route_type_names{
            {RTN_UNSPEC, "RTN_UNSPEC"},
            {RTN_UNICAST, "RTN_UNICAST"},
            {RTN_LOCAL, "RTN_LOCAL"},
            {RTN_BROADCAST, "RTN_BROADCAST"},
            {RTN_ANYCAST, "RTN_ANYCAST"},
            {RTN_MULTICAST, "RTN_MULTICAST"},
            {RTN_BLACKHOLE, "RTN_BLACKHOLE"},
            {RTN_UNREACHABLE, "RTN_UNREACHABLE"},
            {RTN_PROHIBIT, "RTN_PROHIBIT"},
            {RTN_THROW, "RTN_THROW"},
            {RTN_NAT, "RTN_NAT"},
            {RTN_XRESOLVE, "RTN_XRESOLVE"},
    };

    if (route_type_names.contains(route_type)) {
        return route_type_names.at(route_type);
    }

    return "UNKNOWN";
}

} // namespace nl
} // namespace llmx
