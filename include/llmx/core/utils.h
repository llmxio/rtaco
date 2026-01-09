#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

namespace llmx {

[[noreturn]] inline void failwith(const std::string& msg) {
    std::cerr << "FATAL: " << msg << std::endl;
    std::abort();
}

template<typename... Args>
[[noreturn]] inline void failwith(const std::string& fmt, Args&&...) {
    // Minimal implementation: just print the format string
    std::cerr << "FATAL: " << fmt << std::endl;
    std::abort();
}

} // namespace llmx
