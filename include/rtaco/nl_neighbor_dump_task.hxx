#pragma once

#include <cstdint>
#include <expected>
#include <memory_resource>
#include <optional>
#include <system_error>

#include "rtaco/nl_neighbor_task.hxx"
#include "rtaco/nl_neighbor_event.hxx"

struct nlmsghdr;

namespace llmx {
namespace rtaco {

class SocketGuard;

class NeighborDumpTask : public NeighborTask<NeighborDumpTask, NeighborEventList> {
    NeighborEventList learned_;

public:
    NeighborDumpTask(SocketGuard& socket_guard, std::pmr::memory_resource* pmr,
            uint16_t ifindex, uint32_t sequence) noexcept;

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

} // namespace rtaco
} // namespace llmx
