#include "llmx/nl/netlink_neighbor_dump_task.h"

#include <cerrno>
#include <optional>

#include "llmx/core/logger.h"
#include "llmx/core/huge_mem_pool.h"

namespace llmx {
namespace nl {

NeighborDumpTask::NeighborDumpTask(Context& ctx, Socket& socket, IfIndex ifindex,
        uint32_t sequence) noexcept
    : NeighborTask{ctx, socket, ifindex, sequence}
    , learned_{HugeMemPool::instance()} {}

void NeighborDumpTask::prepare_request() {
    std::memset(&request_, 0, sizeof(request_));

    build_request(RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP, 0, 0, {});
}

auto NeighborDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<NeighborEventList, llmx_error_policy>> {
    if (header.nlmsg_seq != sequence()) {
        LOG(INFO) << "Skipping netlink msg with mismatched seq " << header.nlmsg_seq
                  << " (expected " << sequence() << ")";
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE:
        return handle_done();
    case NLMSG_ERROR:
        return handle_error(header);
    case RTM_NEWNEIGH:
        return dispatch_neighbor(header);
    default:
        return std::nullopt;
    }
}

auto NeighborDumpTask::handle_done() -> expected<NeighborEventList, llmx_error_policy> {
    LOG(INFO) << "Handling done: Neighbor dump completed, learned " << learned_.size()
              << " entries";

    return std::move(learned_);
}

auto NeighborDumpTask::handle_error(const nlmsghdr& header)
        -> expected<NeighborEventList, llmx_error_policy> {
    LOG(INFO) << "Handling neighbor dump error message";

    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        LOG(DEBUG) << "Neighbor dump ack received";
        return std::move(learned_);
    }

    LOG(ERROR) << "Neighbor dump returned error: " << error_code.message();
    return std::unexpected{error_code};
}

auto NeighborDumpTask::dispatch_neighbor(const nlmsghdr& header)
        -> std::optional<expected<NeighborEventList, llmx_error_policy>> {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type != NeighborEvent::Type::NEW_NEIGHBOR) {
        return std::nullopt;
    }

    if (event.family != AF_INET6) {
        return std::nullopt;
    }

    if (event.index <= 0) {
        return std::nullopt;
    }

    if (event.index > std::numeric_limits<IfIndex>::max()) {
        return std::nullopt;
    }

    const auto iface = static_cast<IfIndex>(event.index);

    auto& ctx = context();

    if (!ctx.port_table.port_id_for_ifindex(iface)) {
        return std::nullopt;
    }

    learned_.push_back(event);
    return std::nullopt;
}

} // namespace nl
} // namespace llmx
