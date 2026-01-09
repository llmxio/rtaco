#include "rtaco/nl_route_dump_task.hxx"

#include <cerrno>
#include <optional>
#include <utility>

#include "llmx/core/huge_mem_pool.h"

namespace llmx {
namespace nl {

RouteDumpTask::RouteDumpTask(Socket& socket, uint16_t uint16_t,
        uint32_t sequence) noexcept
    : RouteTask{socket, uint16_t, sequence}
    , learned_{HugeMemPool::instance()} {}

void RouteDumpTask::prepare_request() {
    build_request();
}

auto RouteDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<RouteEventList, llmx_error_policy>> {
    if (header.nlmsg_seq != sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE: return handle_done();
    case NLMSG_ERROR: return handle_error(header);
    case RTM_NEWROUTE: return dispatch_route(header);
    default: return std::nullopt;
    }
}

auto RouteDumpTask::handle_done() -> expected<RouteEventList, llmx_error_policy> {
    return std::move(learned_);
}

auto RouteDumpTask::handle_error(const nlmsghdr& header)
        -> expected<RouteEventList, llmx_error_policy> {
    const auto* err       = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code       = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        return std::move(learned_);
    }

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

    if (event.oif_index > std::numeric_limits<uint16_t>::max()) {
        return std::nullopt;
    }

    learned_.push_back(event);
    return std::nullopt;
}

} // namespace nl
} // namespace llmx
