#pragma once

#include "llmx/nl/netlink_neighbor_task.h"

namespace llmx {
namespace nl {

class NeighborGetTask : public NeighborTask<NeighborGetTask, EtherAddr> {
    Ip6Address address_;

public:
    NeighborGetTask(Context& ctx, Socket& socket, IfIndex ifindex, uint32_t sequence,
            const Ip6Address& address);

    void prepare_request();
    auto process_message(const nlmsghdr& header) -> std::optional<expected<EtherAddr>>;

private:
    auto handle_done() -> expected<EtherAddr>;
    auto handle_error(const nlmsghdr& header) -> expected<EtherAddr>;
    auto handle_neighbor(const nlmsghdr& header) -> std::optional<expected<EtherAddr>>;
};

} // namespace nl
} // namespace llmx
