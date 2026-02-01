#include "rtaco/tasks/nl_link_dump_task.hxx"

#include <cstdint>
#include <expected>
#include <limits>
#include <memory_resource>
#include <optional>
#include <system_error>
#include <utility>
#include <cstring>
#include <cerrno>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/events/nl_link_event.hxx"
#include "rtaco/tasks/nl_link_task.hxx"

namespace llmx {
namespace rtaco {

LinkDumpTask::LinkDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
        uint16_t ifindex, uint32_t sequence) noexcept
    : LinkTask{socket_guard, ifindex, sequence}
    , learned_{pmr} {}

void LinkDumpTask::prepare_request() {
    std::memset(&request_, 0, sizeof(request_));

    build_request(NLM_F_REQUEST | NLM_F_DUMP);
}

auto LinkDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<std::expected<LinkEventList, std::error_code>> {
    if (header.nlmsg_seq != sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE: return handle_done();
    case NLMSG_ERROR: return handle_error(header);
    case RTM_NEWLINK: return dispatch_link(header);
    default: return std::nullopt;
    }
}

auto LinkDumpTask::handle_done() -> std::expected<LinkEventList, std::error_code> {
    return std::move(learned_);
}

auto LinkDumpTask::handle_error(const nlmsghdr& header)
        -> std::expected<LinkEventList, std::error_code> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = std::make_error_code(static_cast<std::errc>(code));

    if (!error_code) {
        return std::move(learned_);
    }

    return std::unexpected{error_code};
}

auto LinkDumpTask::dispatch_link(const nlmsghdr& header)
        -> std::optional<std::expected<LinkEventList, std::error_code>> {
    const auto event = LinkEvent::from_nlmsghdr(header);

    if (event.type != LinkEvent::Type::NEW_LINK) {
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

} // namespace rtaco
} // namespace llmx
