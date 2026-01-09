#pragma once

#include "rtaco/nl_neighbor_task.hxx"

namespace llmx {
namespace nl {

class NeighborFlushTask : public NeighborTask<NeighborFlushTask, void> {
    Ip6Address address_;

public:
    NeighborFlushTask(Context& ctx, Socket& socket, IfIndex ifindex, uint32_t sequence,
            const Ip6Address& address);

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<expected<void, llmx_error_policy>>;

private:
    auto handle_error(const nlmsghdr& header) -> expected<void, llmx_error_policy>;
};

} // namespace nl
} // namespace llmx
