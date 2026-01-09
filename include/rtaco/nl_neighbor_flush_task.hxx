#pragma once

#include "rtaco/nl_neighbor_task.hxx"

namespace llmx {
namespace nl {

class NeighborFlushTask : public NeighborTask<NeighborFlushTask, void> {
    std::array<uint8_t, 16> address_;

public:
    NeighborFlushTask(Socket& socket, uint16_t ifindex, uint32_t sequence,
            std::span<uint8_t, 16> address);

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<expected<void, llmx_error_policy>>;

private:
    auto handle_error(const nlmsghdr& header) -> expected<void, llmx_error_policy>;
};

} // namespace nl
} // namespace llmx
