#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/rtnetlink.h>

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
        UNKNOWN = 0,
        UP = 1 << 0,
        BROADCAST = 1 << 1,
        DEBUG = 1 << 2,
        LOOPBACK = 1 << 3,
        POINTOPOINT = 1 << 4,
        NOTRAILERS = 1 << 5,
        RUNNING = 1 << 6,
        NOARP = 1 << 7,
        PROMISC = 1 << 8,
        ALLMULTI = 1 << 9,
        MASTER = 1 << 10,
        SLAVE = 1 << 11,
        MULTICAST = 1 << 12,
        PORTSEL = 1 << 13,
        AUTOMEDIA = 1 << 14,
        DYNAMIC = 1 << 15,
        LOWER_UP = 1 << 16,
        DORMANT = 1 << 17,
        ECHO = 1 << 18,
    };

    Type type{Type::UNKNOWN};
    int index{0};
    Flags flags{Flags::UNKNOWN};
    uint32_t change{0};
    std::string name{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> LinkEvent;
};

using LinkEventList = std::pmr::vector<LinkEvent>;

} // namespace rtaco
} // namespace llmx
