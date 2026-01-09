#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

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
    AddressDumpTask(Context& ctx, Socket& socket, IfIndex ifindex,
            uint32_t sequence) noexcept;

    auto request_payload() const -> std::span<const std::byte>;

    void prepare_request();

    auto process_message(const nlmsghdr& header)
            -> std::optional<expected<AddressEventList, llmx_error_policy>>;

private:
    void build_request();

    auto handle_done() -> expected<AddressEventList, llmx_error_policy>;

    auto handle_error(const nlmsghdr& header)
            -> expected<AddressEventList, llmx_error_policy>;

    auto dispatch_address(const nlmsghdr& header)
            -> std::optional<expected<AddressEventList, llmx_error_policy>>;
};

} // namespace nl
} // namespace llmx
