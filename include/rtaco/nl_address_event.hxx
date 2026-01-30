#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/rtnetlink.h>

#include "rtaco/nl_utils.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

struct AddressEvent {
    enum class Type : uint16_t {
        UNKNOWN = 0,
        NEW_ADDRESS = RTM_NEWADDR,
        DELETE_ADDRESS = RTM_DELADDR,
    };

    enum class Flags : uint32_t {
        NONE = 0,
        SECONDARY = (1u << 0),       // 0x001
        TEMPORARY = SECONDARY,       // alias for SECONDARY
        NODAD = (1u << 1),           // 0x002
        OPTIMISTIC = (1u << 2),      // 0x004
        DADFAILED = (1u << 3),       // 0x008
        HOMEADDRESS = (1u << 4),     // 0x010
        DEPRECATED = (1u << 5),      // 0x020
        TENTATIVE = (1u << 6),       // 0x040
        PERMANENT = (1u << 7),       // 0x080
        MANAGETEMPADDR = (1u << 8),  // 0x100
        NOPREFIXROUTE = (1u << 9),   // 0x200
        MCAUTOJOIN = (1u << 10),     // 0x400
        STABLE_PRIVACY = (1u << 11), // 0x800
    };

    Type type{Type::UNKNOWN};
    int index{0};
    uint8_t prefix_len{0};
    uint8_t scope{0};
    Flags flags{Flags::NONE};
    uint8_t family{0};
    std::string address{};
    std::string label{};

    static auto from_nlmsghdr(const nlmsghdr& header) -> AddressEvent;
};

using AddressEventList = std::pmr::vector<AddressEvent>;

template<>
struct enable_bitmask_operators<AddressEvent::Flags> : std::true_type {};

} // namespace rtaco
} // namespace llmx
