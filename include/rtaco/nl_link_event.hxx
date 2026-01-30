#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/rtnetlink.h>

#include "rtaco/nl_utils.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

struct LinkEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_LINK = RTM_NEWLINK,
        DELETE_LINK = RTM_DELLINK,
    };

    enum class Flags : uint32_t {
        UNKNOWN = 0,             // 0x00000000
        UP = (1u << 0),          // 0x00000001
        BROADCAST = (1u << 1),   // 0x00000002
        DEBUG = (1u << 2),       // 0x00000004
        LOOPBACK = (1u << 3),    // 0x00000008
        POINTOPOINT = (1u << 4), // 0x00000010
        NOTRAILERS = (1u << 5),  // 0x00000020
        RUNNING = (1u << 6),     // 0x00000040
        NOARP = (1u << 7),       // 0x00000080
        PROMISC = (1u << 8),     // 0x00000100
        ALLMULTI = (1u << 9),    // 0x00000200
        MASTER = (1u << 10),     // 0x00000400
        SLAVE = (1u << 11),      // 0x00000800
        MULTICAST = (1u << 12),  // 0x00001000
        PORTSEL = (1u << 13),    // 0x00002000
        AUTOMEDIA = (1u << 14),  // 0x00004000
        DYNAMIC = (1u << 15),    // 0x00008000
        LOWER_UP = (1u << 16),   // 0x00010000
        DORMANT = (1u << 17),    // 0x00020000
        ECHO = (1u << 18),       // 0x00040000
    };

    Type type{Type::UNKNOWN};
    int index{0};
    Flags flags{Flags::UNKNOWN};
    uint32_t change{0};
    std::string name{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> LinkEvent;
};

using LinkEventList = std::pmr::vector<LinkEvent>;

template<>
struct enable_bitmask_operators<LinkEvent::Flags> : std::true_type {};

} // namespace rtaco
} // namespace llmx
