#pragma once

#include <cstdint>

namespace llmx {

using IfIndex = uint32_t;

} // namespace llmx

// Unqualified aliases expected by existing headers
using IfIndex = llmx::IfIndex;

// Execution policy used by connect_to_event
enum class ExecPolicy {
    Sync,
    Async
};
