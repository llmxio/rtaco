#pragma once

#include <stdint.h>
#include <array>
#include <expected>
#include <optional>
#include <span>
#include <system_error>

#include "rtaco/nl_neighbor_task.hxx"

struct nlmsghdr;

namespace llmx {
namespace nl {

class SocketGuard;

class NeighborFlushTask : public NeighborTask<NeighborFlushTask, void> {
    std::array<uint8_t, 16> address_;

public:
    NeighborFlushTask(SocketGuard& socket_guard, uint16_t ifindex, uint32_t sequence,
            std::span<uint8_t, 16> address);

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<void, std::error_code>>;

private:
    auto handle_error(const nlmsghdr& header) -> std::expected<void, std::error_code>;
};

} // namespace nl
} // namespace llmx
