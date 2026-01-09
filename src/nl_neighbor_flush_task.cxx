#include "rtaco/nl_neighbor_flush_task.hxx"

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <cstddef>
#include <cerrno>
#include <optional>

namespace llmx {
namespace nl {
class Socket;
}
} // namespace llmx

namespace llmx {
namespace nl {

NeighborFlushTask::NeighborFlushTask(Socket& socket, uint16_t ifindex, uint32_t sequence,
        std::span<uint8_t, 16> address)
    : NeighborTask{socket, ifindex, sequence} {
    for (std::size_t i = 0; i < 16; ++i) {
        address_[i] = address[i];
    }
}

void NeighborFlushTask::prepare_request() {
    build_request(RTM_DELNEIGH, NLM_F_REQUEST | NLM_F_ACK, 0, 0, address_);
}

auto NeighborFlushTask::process_message(const nlmsghdr& header)
        -> std::optional<std::expected<void, std::error_code>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    if (header.nlmsg_type == NLMSG_ERROR) {
        return handle_error(header);
    }

    return std::nullopt;
}

auto NeighborFlushTask::handle_error(const nlmsghdr& header)
        -> std::expected<void, std::error_code> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = std::make_error_code(static_cast<std::errc>(code));

    if (!error_code) {
        return {};
    }

    return std::unexpected{error_code};
}

} // namespace nl
} // namespace llmx
