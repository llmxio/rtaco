#pragma once

#include <expected>

#include "rtaco/nl_route_task.hxx"
#include "rtaco/nl_route_event.hxx"

namespace llmx {
namespace nl {

class RouteDumpTask : public RouteTask<RouteDumpTask, RouteEventList> {
    RouteEventList learned_;

public:
    RouteDumpTask(Socket& socket, std::pmr::memory_resource* pmr, uint16_t ifindex,
            uint32_t sequence) noexcept;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<RouteEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<RouteEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<RouteEventList, std::error_code>;

    auto dispatch_route(const nlmsghdr& header)
            -> std::optional<std::expected<RouteEventList, std::error_code>>;
};

} // namespace nl
} // namespace llmx
