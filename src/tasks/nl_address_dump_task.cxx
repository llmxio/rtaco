#include "rtaco/tasks/nl_address_dump_task.hxx"

#include <cerrno>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory_resource>
#include <optional>
#include <span>
#include <system_error>
#include <utility>

#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "rtaco/events/nl_address_event.hxx"
#include "rtaco/tasks/nl_request_task.hxx"

namespace llmx {
namespace rtaco {

AddressDumpTask::AddressDumpTask(SocketGuard& socket_guard,
        std::pmr::memory_resource* pmr, uint16_t ifindex, uint32_t sequence) noexcept
    : AddressTask{socket_guard, ifindex, sequence}
    , learned_{pmr} {}

void AddressDumpTask::prepare_request() {
    build_request();
}

auto AddressDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<std::expected<AddressEventList, std::error_code>> {
    if (header.nlmsg_seq != sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE: return handle_done();
    case NLMSG_ERROR: return handle_error(header);
    case RTM_NEWADDR: return dispatch_address(header);
    default: return std::nullopt;
    }
}

auto AddressDumpTask::handle_done() -> std::expected<AddressEventList, std::error_code> {
    return std::move(learned_);
}

auto AddressDumpTask::handle_error(const nlmsghdr& header)
        -> std::expected<AddressEventList, std::error_code> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = std::make_error_code(static_cast<std::errc>(code));

    if (!error_code) {
        return std::move(learned_);
    }

    return std::unexpected{error_code};
}

auto AddressDumpTask::dispatch_address(const nlmsghdr& header)
        -> std::optional<std::expected<AddressEventList, std::error_code>> {
    const auto event = AddressEvent::from_nlmsghdr(header);

    if (event.type != AddressEvent::Type::NEW_ADDRESS) {
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
