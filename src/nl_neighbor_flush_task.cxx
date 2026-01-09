#include "rtaco/nl_neighbor_flush_task.hxx"

#include <cerrno>
#include <optional>

namespace llmx {
namespace nl {

NeighborFlushTask::NeighborFlushTask(Context& ctx, Socket& socket, IfIndex ifindex,
        uint32_t sequence, const Ip6Address& address)
    : NeighborTask{ctx, socket, ifindex, sequence}
    , address_{address} {}

void NeighborFlushTask::prepare_request() {
    build_request(RTM_DELNEIGH, NLM_F_REQUEST | NLM_F_ACK, 0, 0, address_);
}

auto NeighborFlushTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<void, llmx_error_policy>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    if (header.nlmsg_type == NLMSG_ERROR) {
        return handle_error(header);
    }

    return std::nullopt;
}

auto NeighborFlushTask::handle_error(const nlmsghdr& header)
        -> expected<void, llmx_error_policy> {
    const auto* err       = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code       = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        return {};
    }

    return std::unexpected{error_code};
}

} // namespace nl
} // namespace llmx
