#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <linux/neighbour.h>
#include <linux/rtnetlink.h>

struct nlmsghdr;

namespace llmx {
namespace rtaco {

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

using NeighborEventList = std::pmr::vector<NeighborEvent>;

} // namespace rtaco
} // namespace llmx
