#include "llmx/nl/netlink_route_dump_task.h"

#include <cerrno>
#include <optional>
#include <utility>

#include "llmx/core/logger.h"
#include "llmx/core/huge_mem_pool.h"

namespace llmx {
namespace nl {

RouteDumpTask::RouteDumpTask(Context& ctx, Socket& socket, IfIndex ifindex,
        uint32_t sequence) noexcept
    : RouteTask{ctx, socket, ifindex, sequence}
    , learned_{HugeMemPool::instance()} {}

void RouteDumpTask::prepare_request() {
    build_request();
}

auto RouteDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<RouteEventList, llmx_error_policy>> {
    if (header.nlmsg_seq != sequence()) {
        LOG(WARN) << "Sequence mismatch: " << header.nlmsg_seq << " != " << sequence();
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE:
        return handle_done();
    case NLMSG_ERROR:
        return handle_error(header);
    case RTM_NEWROUTE:
        return dispatch_route(header);
    default:
        return std::nullopt;
    }
}

auto RouteDumpTask::handle_done() -> expected<RouteEventList, llmx_error_policy> {
    LOG(INFO) << "Handling done: Route dump completed, learned " << learned_.size()
              << " routes";

    return std::move(learned_);
}

auto RouteDumpTask::handle_error(const nlmsghdr& header)
        -> expected<RouteEventList, llmx_error_policy> {
    LOG(INFO) << "Handling route dump error message";

    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        LOG(DEBUG) << "Route dump ack received";
        return std::move(learned_);
    }

    LOG(ERROR) << "Route dump returned error: " << error_code.message();
    return std::unexpected{error_code};
}

auto RouteDumpTask::dispatch_route(const nlmsghdr& header)
        -> std::optional<expected<RouteEventList, llmx_error_policy>> {
    const auto event = RouteEvent::from_nlmsghdr(header);

    if (event.type != RouteEvent::Type::NEW_ROUTE) {
        return std::nullopt;
    }

    if (event.family != AF_INET6) {
        return std::nullopt;
    }

    if (event.table != RT_TABLE_MAIN) {
        return std::nullopt;
    }

    if (event.oif_index == 0U) {
        return std::nullopt;
    }

    if (event.oif_index > std::numeric_limits<IfIndex>::max()) {
        return std::nullopt;
    }

    const auto iface = static_cast<IfIndex>(event.oif_index);

    auto& ctx = context();

    if (!ctx.port_table.port_id_for_ifindex(iface)) {
        return std::nullopt;
    }

    learned_.push_back(event);
    return std::nullopt;
}

} // namespace nl
} // namespace llmx
