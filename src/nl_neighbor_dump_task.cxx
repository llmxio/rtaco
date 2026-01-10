#include "rtaco/nl_neighbor_dump_task.hxx"

#include <cstdint>
#include <cstring>
#include <expected>
#include <limits>
#include <memory_resource>
#include <optional>
#include <span>
#include <system_error>
#include <utility>
#include <cerrno>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

namespace llmx {
namespace nl {

NeighborDumpTask::NeighborDumpTask(SocketGuard& socket_guard,
        std::pmr::memory_resource* pmr, uint16_t ifindex, uint32_t sequence) noexcept
    : NeighborTask{socket_guard, ifindex, sequence}
    , learned_{pmr} {}

void NeighborDumpTask::prepare_request() {
    std::memset(&request_, 0, sizeof(request_));

    build_request(RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP, 0, 0,
            std::span<uint8_t, 16>{request_.dst});
}

auto NeighborDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<std::expected<NeighborEventList, std::error_code>> {
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

auto NeighborDumpTask::handle_done()
        -> std::expected<NeighborEventList, std::error_code> {
    return std::move(learned_);
}

auto NeighborDumpTask::handle_error(const nlmsghdr& header)
        -> std::expected<NeighborEventList, std::error_code> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = std::make_error_code(static_cast<std::errc>(code));

    if (!error_code) {
        return std::move(learned_);
    }

    return std::unexpected{error_code};
}

auto NeighborDumpTask::dispatch_neighbor(const nlmsghdr& header)
        -> std::optional<std::expected<NeighborEventList, std::error_code>> {
    const auto event = NeighborEvent::from_nlmsghdr(header);

    if (event.type != NeighborEvent::Type::NEW_NEIGHBOR) {
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
