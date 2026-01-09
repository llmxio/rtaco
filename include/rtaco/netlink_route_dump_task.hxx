#pragma once

#include "llmx/nl/netlink_route_task.h"

namespace llmx {
namespace nl {

class RouteDumpTask : public RouteTask<RouteDumpTask, RouteEventList> {
    RouteEventList learned_;

public:
    RouteDumpTask(Context& ctx, Socket& socket, IfIndex ifindex,
            uint32_t sequence) noexcept;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<expected<RouteEventList, llmx_error_policy>>;

private:
    auto handle_done() -> expected<RouteEventList, llmx_error_policy>;

    auto handle_error(const nlmsghdr& header)
            -> expected<RouteEventList, llmx_error_policy>;

    auto dispatch_route(const nlmsghdr& header)
            -> std::optional<expected<RouteEventList, llmx_error_policy>>;
};

} // namespace nl
} // namespace llmx
