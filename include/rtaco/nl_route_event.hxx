#pragma once

#include <cstdint>
#include <string>
#include <memory_resource>
#include <vector>

#include <linux/rtnetlink.h>

namespace llmx {
namespace nl {

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

using RouteEventList = std::pmr::vector<RouteEvent>;

} // namespace nl
} // namespace llmx
