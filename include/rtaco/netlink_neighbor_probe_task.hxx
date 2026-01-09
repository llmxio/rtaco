#pragma once

#include "llmx/nl/netlink_neighbor_task.h"

namespace llmx {
namespace nl {

class NeighborProbeTask : public NeighborTask<NeighborProbeTask, void> {
    Ip6Address address_;

public:
    NeighborProbeTask(Context& ctx, Socket& socket, IfIndex ifindex, uint32_t sequence,
            const Ip6Address& address);

    void prepare_request();
    auto process_message(const nlmsghdr& header) -> std::optional<expected<void>>;

private:
    auto handle_error(const nlmsghdr& header) -> expected<void>;
};

} // namespace nl
} // namespace llmx
