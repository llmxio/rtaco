#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <memory_resource>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

namespace llmx {
namespace nl {

struct AddressEvent {
    enum class Type : uint16_t {
        UNKNOWN        = 0,
        NEW_ADDRESS    = RTM_NEWADDR,
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

using AddressEventList = std::pmr::vector<AddressEvent>;

} // namespace nl
} // namespace llmx
