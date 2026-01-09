#pragma once

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

namespace llmx {
namespace nl {

class NeighborGetTask : public NeighborTask<NeighborGetTask, NeighborEvent> {
    std::array<uint8_t, 16> address_;

public:
    NeighborGetTask(Socket& socket, uint16_t uint16_t, uint32_t sequence,
            std::span<uint8_t, 16> address);

    void prepare_request();
    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;

private:
    auto handle_done() -> std::expected<NeighborEvent, std::error_code>;
    auto handle_error(const nlmsghdr& header) -> std::expected<NeighborEvent, std::error_code>;
    auto handle_neighbor(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEvent, std::error_code>>;
};

} // namespace nl
} // namespace llmx
