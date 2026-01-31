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
namespace rtaco {

/** @brief Netlink-specific endpoint wrapper for Boost.Asio.
 *
 * Encapsulates a `sockaddr_nl` instance and provides the interface required
 * by Boost.Asio raw socket types (data/size/capacity, etc.).
 */
template<typename Protocol>
class Endpoint {
public:
    using protocol_type = Protocol;
    using data_type = boost::asio::detail::socket_addr_type;

    /** @brief Default construct an Endpoint with unspecified pid/groups. */
    Endpoint() noexcept {
        sockaddr_.nl_family = AF_NETLINK;
        sockaddr_.nl_groups = AF_UNSPEC;
        sockaddr_.nl_pid = AF_UNSPEC;
    }

    /** @brief Construct an Endpoint for the given multicast groups and pid. */
    Endpoint(uint32_t groups, uint32_t pid = AF_UNSPEC) noexcept {
        sockaddr_.nl_family = AF_NETLINK;
        sockaddr_.nl_groups = groups;
        sockaddr_.nl_pid = pid;
    }

    Endpoint(const Endpoint& other) noexcept = default;
    Endpoint& operator=(const Endpoint& other) noexcept = default;

    /** @brief Return the protocol type instance for this endpoint. */
    auto protocol() const noexcept -> protocol_type {
        return protocol_type{};
    }

    /** @brief Get mutable pointer to raw socket address storage. */
    auto data() noexcept -> data_type* {
        return reinterpret_cast<data_type*>(&sockaddr_);
    }

    /** @brief Get const pointer to raw socket address storage. */
    auto data() const noexcept -> const data_type* {
        return reinterpret_cast<const data_type*>(&sockaddr_);
    }

    /** @brief Size of this endpoint's address structure. */
    auto size() const noexcept -> size_t {
        return sizeof(sockaddr_);
    }

    void resize(size_t) noexcept {}

    /** @brief Capacity of the address storage. */
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

/** @brief Protocol descriptor for netlink sockets.
 *
 * Holds the protocol number and provides typedefs and helpers used by the
 * Boost.Asio `basic_raw_socket` instantiation for netlink communication.
 */
class Protocol {
public:
    using endpoint = Endpoint<Protocol>;
    using socket = boost::asio::basic_raw_socket<Protocol>;

    Protocol() = default;

    /** @brief Construct a Protocol wrapper for a given netlink protocol. */
    explicit Protocol(int proto) noexcept
        : proto_{proto} {}

    /** @brief Socket type for this protocol (SOCK_RAW). */
    auto type() const noexcept -> int {
        return SOCK_RAW;
    }

    /** @brief Underlying protocol number used with socket(2). */
    auto protocol() const noexcept -> int {
        return proto_;
    }

    /** @brief Address family (AF_NETLINK). */
    auto family() const noexcept -> int {
        return AF_NETLINK;
    }

private:
    int proto_{0};
};

} // namespace rtaco
} // namespace llmx
