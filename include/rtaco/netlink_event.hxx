#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <net/if.h>
#include <linux/netlink.h>
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>
#include <linux/socket.h>

namespace llmx {
namespace nl {

struct LinkEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_LINK = RTM_NEWLINK,
        DELETE_LINK = RTM_DELLINK,
    };

    Type type{Type::UNKNOWN};
    int index{0};
    uint32_t flags{0};
    uint32_t change{0};
    std::string name{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> LinkEvent;
};

struct AddressEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_ADDRESS = RTM_NEWADDR,
        DELETE_ADDRESS = RTM_DELADDR,
    };

    Type type{Type::UNKNOWN};
    int index{0};
    uint8_t prefix_len{0U};
    uint8_t scope{0U};
    uint32_t flags{0U};
    uint8_t family{0U};
    std::string address{};
    std::string label{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> AddressEvent;
};

struct RouteEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_ROUTE = RTM_NEWROUTE,
        DELETE_ROUTE = RTM_DELROUTE,
    };

    Type type{Type::UNKNOWN};
    uint8_t family{0U};
    uint8_t dst_prefix_len{0U};
    uint8_t src_prefix_len{0U};
    uint8_t scope{0U};
    uint8_t protocol{0U};
    uint8_t route_type{0U};
    uint32_t flags{0U};
    uint32_t table{0U};
    uint32_t priority{0U};
    uint32_t oif_index{0U};
    std::string dst{};
    std::string src{};
    std::string gateway{};
    std::string prefsrc{};
    std::string oif{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> RouteEvent;
};

struct NeighborEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_NEIGHBOR = RTM_NEWNEIGH,
        DELETE_NEIGHBOR = RTM_DELNEIGH,
    };

    enum class State : uint16_t {
        NONE = NUD_NONE,
        INCOMPLETE = NUD_INCOMPLETE,
        REACHABLE = NUD_REACHABLE,
        STALE = NUD_STALE,
        DELAY = NUD_DELAY,
        PROBE = NUD_PROBE,
        FAILED = NUD_FAILED,
        NOARP = NUD_NOARP,
        PERMANENT = NUD_PERMANENT,
    };

    Type type{Type::UNKNOWN};
    int index{0};
    uint8_t family{0U};
    State state{State::NONE};
    uint8_t flags{0U};
    uint8_t neighbor_type{0U};
    std::string address{};
    std::string lladdr{};

    auto state_to_string() const -> std::string {
        if (std::to_underlying(state) == NUD_NONE) {
            return "NUD_NONE";
        }

        struct StateName {
            uint16_t mask;
            std::string_view name;
        };

        static constexpr std::array<StateName, 8> state_names{{
                {NUD_INCOMPLETE, "NUD_INCOMPLETE"},
                {NUD_REACHABLE, "NUD_REACHABLE"},
                {NUD_STALE, "NUD_STALE"},
                {NUD_DELAY, "NUD_DELAY"},
                {NUD_PROBE, "NUD_PROBE"},
                {NUD_FAILED, "NUD_FAILED"},
                {NUD_NOARP, "NUD_NOARP"},
                {NUD_PERMANENT, "NUD_PERMANENT"},
        }};

        std::string result{};
        for (const auto& entry : state_names) {
            if ((std::to_underlying(state) & entry.mask) != 0U) {
                if (!result.empty()) {
                    result += '|';
                }
                result.append(entry.name);
            }
        }

        return result.empty() ? "UNKNOWN" : result;
    }

    static auto from_nlmsghdr(const nlmsghdr& header) -> NeighborEvent;
};

template<typename T>
concept IsEnumeration = std::is_enum_v<std::remove_cvref_t<T>> ||
        std::is_scoped_enum_v<std::remove_cvref_t<T>>;

template<typename T>
concept IsNetlinkEvent = IsEnumeration<T> || std::integral<std::remove_cvref_t<T>>;

static constexpr auto type_to_string(IsNetlinkEvent auto&& type) noexcept
        -> std::string_view {
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

static constexpr auto family_to_string(uint8_t family) noexcept -> std::string_view {
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

static constexpr auto protocol_to_string(uint8_t protocol) noexcept -> std::string_view {
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

static constexpr auto route_type_to_string(uint8_t route_type) noexcept
        -> std::string_view {
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

using NeighborEventList = std::pmr::vector<NeighborEvent>;
using RouteEventList = std::pmr::vector<RouteEvent>;
using LinkEventList = std::pmr::vector<LinkEvent>;
using AddressEventList = std::pmr::vector<AddressEvent>;

} // namespace nl
} // namespace llmx
