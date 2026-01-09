#pragma once

#include <chrono>

namespace llmx {

// Minimal with_timeout: return the awaitable as-is. For production code,
// this would integrate timers and cancellation.
template<typename Awaitable, typename Duration>
inline auto with_timeout(Awaitable&& a, Duration) -> Awaitable {
    return std::forward<Awaitable>(a);
}

} // namespace llmx
