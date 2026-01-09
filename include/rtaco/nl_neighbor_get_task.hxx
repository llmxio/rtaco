#pragma once

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_event.hxx"

namespace llmx {
namespace nl {

class NeighborGetTask : public NeighborTask<NeighborGetTask, NeighborEvent> {
    Ip6Address address_;

public:
    NeighborGetTask(Context& ctx, Socket& socket, IfIndex ifindex, uint32_t sequence,
            const Ip6Address& address);

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
