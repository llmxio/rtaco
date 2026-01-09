#pragma once

#include <array>
#include <cstdint>

namespace llmx {
namespace net {

struct Ip6Addr {
    static constexpr std::size_t BYTES_SIZE = 16;
    using Bytes                             = std::array<uint8_t, BYTES_SIZE>;
};

using Ip6Address = Ip6Addr::Bytes;

} // namespace net

// For compatibility with existing code which uses unqualified Ip6Addr/Ip6Address
using Ip6Addr    = llmx::net::Ip6Addr;
using Ip6Address = llmx::net::Ip6Address;

} // namespace llmx
