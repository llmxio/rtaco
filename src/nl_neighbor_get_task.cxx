#include "rtaco/nl_neighbor_get_task.hxx"
#include "rtaco/nl_event.hxx"

#include <cerrno>
#include <optional>
#include <arpa/inet.h>
#include <array>
#include <string>

namespace llmx {
namespace nl {

NeighborGetTask::NeighborGetTask(Context& ctx, Socket& socket, IfIndex ifindex,
        uint32_t sequence, const Ip6Address& address)
    : NeighborTask{ctx, socket, ifindex, sequence}
    , address_{address} {}

void NeighborGetTask::prepare_request() {
    build_request(RTM_GETNEIGH, NLM_F_REQUEST, 0, 0, address_);
}

auto NeighborGetTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<NeighborEvent>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE: return handle_done();
    case NLMSG_ERROR: return handle_error(header);
    case RTM_NEWNEIGH: return handle_neighbor(header);
    default: return std::nullopt;
    }
}

auto NeighborGetTask::handle_done() -> expected<NeighborEvent> {
    return std::unexpected{from_errno(ENOENT)};
}

auto NeighborGetTask::handle_error(const nlmsghdr& header) -> expected<NeighborEvent> {
    const auto* err       = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code       = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        return std::unexpected{from_errno(ENOENT)};
    }

    return std::unexpected{error_code};
}

auto NeighborGetTask::handle_neighbor(const nlmsghdr& header)
        -> std::optional<expected<NeighborEvent>> {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type == NeighborEvent::Type::UNKNOWN) {
        return std::nullopt;
    }

    if (!event.lladdr.empty()) {
        return event;
    }

    return std::nullopt;
}

} // namespace nl
} // namespace llmx
