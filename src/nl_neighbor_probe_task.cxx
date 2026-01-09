#include "rtaco/nl_neighbor_probe_task.hxx"

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
        return {};
    }

    return std::unexpected{error_code};
}

} // namespace nl
} // namespace llmx
