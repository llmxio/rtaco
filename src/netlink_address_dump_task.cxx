#include "rtaco/nl_address_dump_task.hxx"

#include <cerrno>
#include <limits>
#include <optional>
#include <utility>

#include "llmx/core/huge_mem_pool.h"

namespace llmx {
namespace nl {

AddressDumpTask::AddressDumpTask(Socket& socket, uint16_t uint16_t,
        uint32_t sequence) noexcept
    : RequestTask{socket, uint16_t, sequence}
    , request_{}
    , learned_{HugeMemPool::instance()} {}

auto AddressDumpTask::request_payload() const -> std::span<const std::byte> {
    return {reinterpret_cast<const std::byte*>(&request_), request_.header.nlmsg_len};
}

void AddressDumpTask::prepare_request() {
    build_request();
}

auto AddressDumpTask::process_message(const nlmsghdr& header)
        -> std::optional<expected<AddressEventList, llmx_error_policy>> {
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

void AddressDumpTask::build_request() {
    request_.header.nlmsg_len   = NLMSG_LENGTH(sizeof(ifaddrmsg));
    request_.header.nlmsg_type  = RTM_GETADDR;
    request_.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    request_.header.nlmsg_seq   = sequence();
    request_.header.nlmsg_pid   = 0;

    request_.message.ifa_family    = AF_INET6;
    request_.message.ifa_prefixlen = 0;
    request_.message.ifa_flags     = 0;
    request_.message.ifa_scope     = RT_SCOPE_UNIVERSE;
    request_.message.ifa_index     = static_cast<int>(uint16_t());
}

auto AddressDumpTask::handle_done() -> expected<AddressEventList, llmx_error_policy> {
    return std::move(learned_);
}

auto AddressDumpTask::handle_error(const nlmsghdr& header)
        -> expected<AddressEventList, llmx_error_policy> {
    const auto* err       = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code       = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        return std::move(learned_);
    }

    return std::unexpected{error_code};
}

auto AddressDumpTask::dispatch_address(const nlmsghdr& header)
        -> std::optional<expected<AddressEventList, llmx_error_policy>> {
    const auto event = AddressEvent::from_nlmsghdr(header);

    if (event.type != AddressEvent::Type::NEW_ADDRESS) {
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
