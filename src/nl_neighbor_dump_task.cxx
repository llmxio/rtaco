#include "rtaco/nl_neighbor_dump_task.hxx"

#include <cerrno>
#include <optional>

namespace llmx {
namespace nl {

NeighborDumpTask::NeighborDumpTask(Socket& socket, std::pmr::memory_resource* pmr,
        uint16_t ifindex, uint32_t sequence) noexcept
    : NeighborTask{socket, ifindex, sequence}
    , learned_{pmr} {}

void NeighborDumpTask::prepare_request() {
    std::memset(&request_, 0, sizeof(request_));

    // For a dump request we don't have a destination address; pass an empty
    // fixed-size span so the `build_request` overload compiles and leaves the
    // request dst zero-initialized (we already memset above).
    // Use the request buffer itself (zeroed above) as the source span so
    // we have a fixed-size span of 16 bytes per the `build_request` API.
    build_request(RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP, 0, 0,
            std::span<uint8_t, 16>{request_.dst});
}

auto NeighborDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<NeighborEventList, llmx_error_policy>> {
    if (header.nlmsg_seq != sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE: return handle_done();
    case NLMSG_ERROR: return handle_error(header);
    case RTM_NEWNEIGH: return dispatch_neighbor(header);
    default: return std::nullopt;
    }
}

auto NeighborDumpTask::handle_done() -> expected<NeighborEventList, llmx_error_policy> {
    return std::move(learned_);
}

auto NeighborDumpTask::handle_error(const nlmsghdr& header)
        -> expected<NeighborEventList, llmx_error_policy> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        return std::move(learned_);
    }

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

    if (event.index > std::numeric_limits<uint16_t>::max()) {
        return std::nullopt;
    }

    learned_.push_back(event);
    return std::nullopt;
}

} // namespace nl
} // namespace llmx
