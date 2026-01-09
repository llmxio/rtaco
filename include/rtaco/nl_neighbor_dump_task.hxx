#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <linux/neighbour.h>
#include <linux/rtnetlink.h>

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

namespace llmx {
namespace nl {

class NeighborDumpTask : public NeighborTask<NeighborDumpTask, NeighborEventList> {
    NeighborEventList learned_;

public:
    NeighborDumpTask(Socket& socket, std::pmr::memory_resource* pmr, uint16_t ifindex,
            uint32_t sequence) noexcept;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEventList, std::error_code>>;

private:
    auto handle_done() -> std::expected<NeighborEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<NeighborEventList, std::error_code>;

    auto dispatch_neighbor(const nlmsghdr& header)
            -> std::optional<std::expected<NeighborEventList, std::error_code>>;
};

} // namespace nl
} // namespace llmx
