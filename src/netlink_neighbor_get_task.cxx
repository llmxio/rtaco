#include "llmx/nl/netlink_neighbor_get_task.h"

#include <cerrno>
#include <optional>

#include "llmx/core/logger.h"

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
        -> std::optional<expected<EtherAddr>> {
    if (header.nlmsg_seq != this->sequence()) {
        return std::nullopt;
    }

    switch (header.nlmsg_type) {
    case NLMSG_DONE:
        return handle_done();
    case NLMSG_ERROR:
        return handle_error(header);
    case RTM_NEWNEIGH:
        return handle_neighbor(header);
    default:
        return std::nullopt;
    }
}

auto NeighborGetTask::handle_done() -> expected<EtherAddr> {
    LOG(DEBUG) << "Neighbor get done without response";
    return std::unexpected{from_errno(ENOENT)};
}

auto NeighborGetTask::handle_error(const nlmsghdr& header) -> expected<EtherAddr> {
    const auto* err = reinterpret_cast<const nlmsgerr*>(NLMSG_DATA(&header));
    const auto code = err != nullptr ? -err->error : EPROTO;
    const auto error_code = from_errno(code);

    if (!error_code) {
        LOG(DEBUG) << "Neighbor get ack received (entry not found)";
        return std::unexpected{from_errno(ENOENT)};
    }

    LOG(ERROR) << "Neighbor get returned error: " << error_code.message();
    return std::unexpected{error_code};
}

auto NeighborGetTask::handle_neighbor(const nlmsghdr& header)
        -> std::optional<expected<EtherAddr>> {
    if (header.nlmsg_len < NLMSG_LENGTH(sizeof(ndmsg))) {
        LOG(WARN) << "Neighbor message too short";
        return std::nullopt;
    }

    const auto* info = reinterpret_cast<const ndmsg*>(NLMSG_DATA(&header));
    if (info == nullptr) {
        return std::nullopt;
    }

    int attr_length = static_cast<int>(header.nlmsg_len) -
            static_cast<int>(NLMSG_LENGTH(sizeof(ndmsg)));
    if (attr_length <= 0) {
        return std::nullopt;
    }

    auto* attr = reinterpret_cast<rtattr*>(
            reinterpret_cast<std::uintptr_t>(info) + NLMSG_ALIGN(sizeof(ndmsg)));

    for (; RTA_OK(attr, attr_length); attr = RTA_NEXT(attr, attr_length)) {
        if (attr->rta_type != NDA_LLADDR) {
            continue;
        }

        const auto payload = static_cast<size_t>(RTA_PAYLOAD(attr));
        if (payload != EtherAddr::SIZE) {
            LOG(WARN) << "NDA_LLADDR has unexpected size: " << payload;
            continue;
        }

        const auto* data = reinterpret_cast<const uint8_t*>(RTA_DATA(attr));
        if (data == nullptr) {
            continue;
        }

        const auto mac = EtherAddr::from_bytes(data[0], data[1], data[2], data[3],
                data[4], data[5]);
        LOG(DEBUG) << "Neighbor get resolved to " << mac.to_string();
        return mac;
    }

    return std::nullopt;
}

} // namespace nl
} // namespace llmx
