#include "llmx/nl/netlink_neighbor_probe_task.h"

#include <cerrno>
#include <optional>

#include "llmx/core/logger.h"

namespace llmx {
namespace nl {

NeighborProbeTask::NeighborProbeTask(Context& ctx, Socket& socket, IfIndex ifindex,
        uint32_t sequence, const Ip6Address& address)
    : NeighborTask{ctx, socket, ifindex, sequence}
    , address_{address} {}

void NeighborProbeTask::prepare_request() {
    build_request(RTM_NEWNEIGH, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE,
            NUD_PROBE, NTF_USE, address_);
}

auto NeighborProbeTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<void>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    if (header.nlmsg_type == NLMSG_ERROR) {
        return handle_error(header);
    }

    return std::nullopt;
}

auto NeighborProbeTask::handle_error(const nlmsghdr& header) -> expected<void> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        LOG(DEBUG) << "Neighbor probe ack received";
        return {};
    }

    LOG(ERROR) << "Neighbor probe returned error: " << error_code.message();
    return std::unexpected{error_code};
}

} // namespace nl
} // namespace llmx
