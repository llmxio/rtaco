#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <linux/rtnetlink.h>

struct nlmsghdr;

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
