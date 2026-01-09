#pragma once

#include <optional>

namespace llmx {
namespace nl {

struct PortTable {
    // Minimal stub: pretend every ifindex maps to port id 1
    auto port_id_for_ifindex(unsigned /*ifindex*/) const -> std::optional<int> {
        return 1;
    }
};

struct Context {
    PortTable port_table;
};

} // namespace nl
} // namespace llmx
