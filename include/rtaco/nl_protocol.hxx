#pragma once

#include <boost/asio/basic_raw_socket.hpp>
#include <boost/asio/detail/socket_types.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>

namespace llmx {
namespace nl {

template<typename Protocol>
class Endpoint {
public:
    using protocol_type = Protocol;
    using data_type = boost::asio::detail::socket_addr_type;

    Endpoint() noexcept {
        sockaddr_.nl_family = AF_NETLINK;
        sockaddr_.nl_groups = AF_UNSPEC;
        sockaddr_.nl_pid = AF_UNSPEC;
    }

    Endpoint(uint32_t groups, uint32_t pid = AF_UNSPEC) noexcept {
        sockaddr_.nl_family = AF_NETLINK;
        sockaddr_.nl_groups = groups;
        sockaddr_.nl_pid = pid;
    }

    Endpoint(const Endpoint& other) noexcept = default;
    Endpoint& operator=(const Endpoint& other) noexcept = default;

    auto protocol() const noexcept -> protocol_type {
        return protocol_type{};
    }

    auto data() noexcept -> data_type* {
        return reinterpret_cast<data_type*>(&sockaddr_);
    }

    auto data() const noexcept -> const data_type* {
        return reinterpret_cast<const data_type*>(&sockaddr_);
    }

    auto size() const noexcept -> size_t {
        return sizeof(sockaddr_);
    }

    void resize(size_t) noexcept {}

    auto capacity() const noexcept -> size_t {
        return sizeof(sockaddr_);
    }

    friend bool operator==(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return std::memcmp(&lhs.sockaddr_, &rhs.sockaddr_, sizeof(sockaddr_nl)) == 0;
    }

    friend bool operator!=(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator<(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return std::memcmp(&lhs.sockaddr_, &rhs.sockaddr_, sizeof(sockaddr_nl)) < 0;
    }

    friend bool operator>(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Endpoint& lhs, const Endpoint& rhs) noexcept {
        return !(lhs < rhs);
    }

private:
    sockaddr_nl sockaddr_{};
};

class Protocol {
public:
    using endpoint = Endpoint<Protocol>;
    using socket = boost::asio::basic_raw_socket<Protocol>;

    Protocol() = default;

    explicit Protocol(int proto) noexcept
        : proto_{proto} {}

    auto type() const noexcept -> int {
        return SOCK_RAW;
    }

    auto protocol() const noexcept -> int {
        return proto_;
    }

    auto family() const noexcept -> int {
        return AF_NETLINK;
    }

private:
    int proto_{0};
};

} // namespace nl
} // namespace llmx
