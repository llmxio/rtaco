#!/usr/bin/env python3
"""Minimal mock runner for `cpp-modernize` examples.

It applies a small, deterministic transformation to input examples for smoke tests.
"""

import sys


def modernize_text(s: str) -> str:
    s = s.strip()
    if "sprintf" in s or "strcat" in s:
        return (
            "Summary: Replace unsafe C string handling with `std::string`/`std::string_view` and standard algorithms.\n\n"
            "    #include <string>\n    #include <string_view>\n    #include <iostream>\n\n    void join(std::span<const std::string_view> parts) {\n        std::string out;\n        for (auto sv : parts) out += sv;\n        std::cout << out << '\\n';\n    }\n\n"
            "Reference: [cppreference - basic_string](https://en.cppreference.com/w/cpp/string/basic_string)"
        )
    if "snprintf" in s:
        return (
            "Summary: Use `std::format` for type-safe formatting and clearer intent; note C++23 toolchain flag may be required.\n\n"
            '    #include <format>\n    #include <cstdio>\n\n    auto s = std::format("user={} id={}", name, id);\n    std::puts(s.c_str());\n\n'
            "Reference: [cppreference - format](https://en.cppreference.com/w/cpp/utility/format)"
        )
    if "const int* data, size_t n" in s or "pointer and length" in s:
        return (
            "Summary: Use `std::span` for non-owning views and range-based iteration for clarity and safety.\n\n"
            "    #include <span>\n\n    void process(std::span<const int> data) {\n        for (int v : data) handle(v); // range-based loop\n    }\n\n"
            "Reference: [cppreference - span](https://en.cppreference.com/w/cpp/container/span)"
        )
    if "load_config(Config& out)" in s or "out-parameter" in s:
        return (
            "Summary: Return `std::expected<T, E>` to encode success or a recoverable error rather than using out-params and bool.\n\n"
            '    #include <expected>\n    #include <string>\n\n    struct Error { int code; std::string message; };\n\n    std::expected<Config, Error> load_config() {\n        if (!file_exists()) return std::unexpected(Error{1, "file missing"});\n        return Config{/*...*/};\n    }\n\n'
            "Reference: [cppreference - expected](https://en.cppreference.com/w/cpp/utility/expected)"
        )
    if "coroutine" in s or "co_yield" in s:
        return (
            "Summary: Use coroutines for lazy generators and asynchronous flows; show a minimal generator example (requires C++20 coroutine support).\n\n"
            "    #include <coroutine>\n    #include <optional>\n\n    // Minimal generator-like coroutine (illustrative)\n    struct generator {\n        struct promise_type {\n            std::optional<int> value_;\n            generator get_return_object() { return generator{}; }\n            std::suspend_always initial_suspend() { return {}; }\n            std::suspend_always final_suspend() noexcept { return {}; }\n            std::suspend_always yield_value(int v) { value_ = v; return {}; }\n            void return_void() {}\n            void unhandled_exception() { std::terminate(); }\n        };\n    };\n\n"
            "Reference: [cppreference - coroutines](https://en.cppreference.com/w/cpp/language/coroutines)"
        )
    if "concept" in s or "Incrementable" in s:
        return (
            "Summary: Use Concepts to express template requirements and improve diagnostics; show a simple `Incrementable` example.\n\n"
            "    template<typename T>\n    concept Incrementable = requires(T a) { ++a; };\n\n    template<Incrementable T>\n    T inc(T x) { ++x; return x; }\n\n"
            "Reference: [cppreference - concepts](https://en.cppreference.com/w/cpp/language/concepts)"
        )
    if "views::filter" in s or "ranges pipeline" in s:
        return (
            "Summary: Use `std::ranges` pipelines (`views::filter` and `views::transform`) for clear, composable transformations.\n\n"
            "    #include <ranges>\n\n    auto evens_doubled = in | std::views::filter([](int v){ return v % 2 == 0; })\n                              | std::views::transform([](int v){ return v * 2; });\n\n    for (int v : evens_doubled) process(v);\n\n"
            "Reference: [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)"
        )
    if (
        "floating" in s
        or "precision" in s
        or "advanced std::format" in s
        or "padded" in s
        or "hex" in s
        or "std::format" in s
    ):
        return (
            "Summary: Use `std::format` format specifiers for alignment, precision and integer formats (e.g., hex with zero-padding).\n\n"
            '    #include <format>\n    auto s = std::format("{:<10} id={:04X} val={:8.2f}", name, id, val); // name left-aligned, id hex padded, val width+precision\n    std::puts(s.c_str());\n\n'
            "Reference: [cppreference - format](https://en.cppreference.com/w/cpp/utility/format)"
        )
    if "const int* data, size_t n" in s or "pointer and length" in s:
        return (
            "Summary: Use `std::span` for non-owning views and range-based iteration for clarity and safety.\n\n"
            "```cpp\n#include <span>\n\nvoid process(std::span<const int> data) {\n    for (int v : data) handle(v); // range-based loop\n}\n```\n\n"
            "Reference: [cppreference - span](https://en.cppreference.com/w/cpp/container/span)"
        )
    if "load_config(Config& out)" in s or "out-parameter" in s:
        return (
            "Summary: Return `std::expected<T, E>` to encode success or a recoverable error rather than using out-params and bool.\n\n"
            '```cpp\n#include <expected>\n#include <string>\n\nstruct Error { int code; std::string message; };\n\nstd::expected<Config, Error> load_config() {\n    if (!file_exists()) return std::unexpected(Error{1, "file missing"});\n    return Config{/*...*/};\n}\n```\n\n'
            "Reference: [cppreference - expected](https://en.cppreference.com/w/cpp/utility/expected)"
        )
    if "for (size_t i = 0; i < n; ++i) v.push_back(arr[i]);" in s:
        return (
            "Summary: Use iterator- or range-based construction to avoid manual loops.\n\n"
            "    #include <vector>\n    #include <ranges>\n\n    void copy(const int* arr, size_t n) {\n        std::vector<int> v(std::ranges::begin(std::span(arr, n)), std::ranges::end(std::span(arr, n)));\n    }\n\n"
            "Reference: [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)"
        )
    return s


if __name__ == "__main__":
    data = sys.stdin.read()
    out = modernize_text(data)
    sys.stdout.write(out)
