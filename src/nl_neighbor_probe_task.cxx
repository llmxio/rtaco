#include "rtaco/nl_neighbor_probe_task.hxx"

#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <cstddef>
#include <cerrno>
#include <optional>

namespace llmx {
namespace nl {

NeighborProbeTask::NeighborProbeTask(Socket& socket, uint16_t uint16_t, uint32_t sequence,
        std::span<uint8_t, 16> address)
    : NeighborTask{socket, uint16_t, sequence} {
    for (std::size_t i = 0; i < 16; ++i) {
        address_[i] = address[i];
    }
}

void NeighborProbeTask::prepare_request() {
    build_request(RTM_NEWNEIGH, NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE,
            NUD_PROBE, NTF_USE, address_);
}

auto NeighborProbeTask::process_message(const nlmsghdr& header)
        -> std::optional<std::expected<void, std::error_code>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    if (header.nlmsg_type == NLMSG_ERROR) {
        return handle_error(header);
    }

    return std::nullopt;
}

auto NeighborProbeTask::handle_error(const nlmsghdr& header)
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
