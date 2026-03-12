#pragma once

#include <atomic>
#include <cerrno>
#include <system_error>

namespace llmx {
namespace rtaco::test_hooks {

inline std::atomic_int socket_open_errno{0};
inline std::atomic_int socket_bind_errno{0};
inline std::atomic_int gate_wait_errno{0};

inline void reset() {
    socket_open_errno.store(0, std::memory_order_relaxed);
    socket_bind_errno.store(0, std::memory_order_relaxed);
    gate_wait_errno.store(0, std::memory_order_relaxed);
}

inline void fail_socket_open_once(std::errc code) {
    socket_open_errno.store(static_cast<int>(code), std::memory_order_relaxed);
}

inline void fail_socket_bind_once(std::errc code) {
    socket_bind_errno.store(static_cast<int>(code), std::memory_order_relaxed);
}

inline void fail_gate_wait_once(std::errc code) {
    gate_wait_errno.store(static_cast<int>(code), std::memory_order_relaxed);
}

inline auto consume_socket_open_error() -> std::error_code {
    auto value = socket_open_errno.exchange(0, std::memory_order_relaxed);
    if (value == 0) {
        return {};
    }

    return {value, std::generic_category()};
}

inline auto consume_socket_bind_error() -> std::error_code {
    auto value = socket_bind_errno.exchange(0, std::memory_order_relaxed);
    if (value == 0) {
        return {};
    }

    return {value, std::generic_category()};
}

inline auto consume_gate_wait_error() -> std::error_code {
    auto value = gate_wait_errno.exchange(0, std::memory_order_relaxed);
    if (value == 0) {
        return {};
    }

    return {value, std::generic_category()};
}

} // namespace rtaco::test_hooks
} // namespace llmx
