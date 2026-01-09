#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <expected>

#include <linux/rtnetlink.h>

#include "rtaco/nl_address_event.hxx"
#include "rtaco/nl_request_task.hxx"

namespace llmx {
namespace nl {

struct AddressRequest {
    nlmsghdr header;
    ifaddrmsg message;
};

class AddressDumpTask : public RequestTask<AddressDumpTask, AddressEventList> {
    AddressRequest request_;
    AddressEventList learned_;

public:
    AddressDumpTask(Socket& socket, std::pmr::memory_resource* pmr, uint16_t ifindex,
            uint32_t sequence) noexcept;

    auto request_payload() const -> std::span<const std::byte>;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;

private:
    void build_request();

    auto handle_done() -> std::expected<AddressEventList, std::error_code>;

    auto handle_error(const nlmsghdr& header)
            -> std::expected<AddressEventList, std::error_code>;

    auto dispatch_address(const nlmsghdr& header)
            -> std::optional<std::expected<AddressEventList, std::error_code>>;
};

} // namespace nl
} // namespace llmx
