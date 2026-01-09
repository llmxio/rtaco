#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory_resource>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "rtaco/nl_route_event.hxx"

#include <net/if.h>
#include <linux/netlink.h>
#include <linux/neighbour.h>
#include <linux/rtnetlink.h>
#include <linux/socket.h>
#include "rtaco/nl_common.hxx"

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

using LinkEventList = std::pmr::vector<LinkEvent>;

} // namespace nl
} // namespace llmx
