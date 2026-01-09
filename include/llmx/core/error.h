#pragma once

#include <expected>
#include <system_error>

using llmx_error_policy = std::error_code;

// Provide a simple expected alias matching usage in the repo
template<typename T, typename E = llmx_error_policy>
using expected = std::expected<T, E>;

inline auto from_errno(int code) -> std::error_code {
    // Negative codes may be passed by callers; normalize to positive values
    if (code < 0) {
        code = -code;
    }
    return std::error_code(code, std::generic_category());
}
