#pragma once

#include <memory_resource>

namespace llmx {

class HugeMemPool {
public:
    static std::pmr::memory_resource* instance() noexcept {
        return std::pmr::get_default_resource();
    }
};

} // namespace llmx
