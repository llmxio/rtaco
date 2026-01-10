#pragma once

#include <stdint.h>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <system_error>

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

struct nlmsghdr;

namespace llmx {
namespace nl {

class SocketGuard;

class NeighborGetTask : public NeighborTask<NeighborGetTask, NeighborEvent> {
    std::array<uint8_t, 16> address_;

public:
    NeighborGetTask(SocketGuard& socket_guard, uint16_t uint16_t, uint32_t sequence,
            std::span<uint8_t, 16> address);

    void prepare_request();
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;

private:
    auto handle_done() -> std::expected<NeighborEvent, std::error_code>;
    auto handle_error(const nlmsghdr& header)
            -> std::expected<NeighborEvent, std::error_code>;
    auto handle_neighbor(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;
};

} // namespace nl
} // namespace llmx
