#pragma once

#include "rtaco/nl_neighbor_task.hxx"

namespace llmx {
namespace nl {

class NeighborProbeTask : public NeighborTask<NeighborProbeTask, void> {
    std::array<uint8_t, 16> address_;

public:
    NeighborProbeTask(Socket& socket, uint16_t uint16_t, uint32_t sequence,
            std::span<uint8_t, 16> address);

    void prepare_request();
    auto process_message(const nlmsghdr& header) -> std::optional<expected<void>>;

private:
    auto handle_error(const nlmsghdr& header) -> expected<void>;
};

} // namespace nl
} // namespace llmx
