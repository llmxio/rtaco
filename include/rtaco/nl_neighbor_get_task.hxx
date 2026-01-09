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
            -> std::optional<expected<NeighborEvent>>;

private:
    auto handle_done() -> expected<NeighborEvent>;
    auto handle_error(const nlmsghdr& header) -> expected<NeighborEvent>;
    auto handle_neighbor(const nlmsghdr& header)
            -> std::optional<expected<NeighborEvent>>;
};

} // namespace nl
} // namespace llmx
