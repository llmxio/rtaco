#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/rtnetlink.h>

#include "rtaco/nl_utils.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

struct RouteEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_ROUTE = RTM_NEWROUTE,
        DELETE_ROUTE = RTM_DELROUTE,
    };

    enum class Flags : uint32_t {
        NONE = 0,
        NOTIFY = (1u << 8),          // 0x100 Notify user of route change
        CLONED = (1u << 9),          // 0x200 This route is cloned
        EQUALIZE = (1u << 10),       // 0x400 Multipath equalizer / equalized route
        PREFIX = (1u << 11),         // 0x800 Prefix addresses
        LOOKUP_TABLE = (1u << 12),   // 0x1000 set rtm_table to FIB lookup result
        FIB_MATCH = (1u << 13),      // 0x2000 return full fib lookup match
        OFFLOAD = (1u << 14),        // 0x4000 route is offloaded
        TRAP = (1u << 15),           // 0x8000 route is trapping packets
        OFFLOAD_FAILED = (1u << 29), // 0x20000000 route offload failed
    };

    Type type{Type::UNKNOWN};
    uint8_t family{0};
    uint8_t dst_prefix_len{0};
    uint8_t src_prefix_len{0};
    uint8_t scope{0};
    uint8_t protocol{0};
    uint8_t route_type{0};
    Flags flags{Flags::NONE};
    uint32_t table{0};
    uint32_t priority{0};
    uint32_t oif_index{0};
    std::string dst{};
    std::string src{};
    std::string gateway{};
    std::string prefsrc{};
    std::string oif{};

    /** @brief Parse a RouteEvent from a netlink message header.
     *
     * Extracts route attributes and fills a RouteEvent structure.
     */
    static auto from_nlmsghdr(const nlmsghdr& header) -> RouteEvent;
};

using RouteEventList = std::pmr::vector<RouteEvent>;

template<>
struct enable_bitmask_operators<RouteEvent::Flags> : std::true_type {};

} // namespace rtaco
} // namespace llmx
