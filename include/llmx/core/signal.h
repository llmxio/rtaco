#pragma once

#include <boost/signals2.hpp>

namespace llmx {

template<typename Sig>
class Signal {
public:
    using signal_type = boost::signals2::signal<Sig>;

    Signal() = default;

    // Accept an executor or anything else and ignore it (compat shim)
    template<typename Executor>
    Signal(Executor&&) {}

    template<typename Slot, typename Policy>
    auto connect(Slot&& s, Policy) {
        return sig_.connect(std::forward<Slot>(s));
    }

    template<typename Slot>
    auto connect(Slot&& s) {
        return sig_.connect(std::forward<Slot>(s));
    }

    // Allow invoking the signal like a callable
    template<typename... Args>
    void operator()(Args&&... args) {
        sig_(std::forward<Args>(args)...);
    }

    auto operator()() -> signal_type& {
        return sig_;
    }

private:
    signal_type sig_;
};

} // namespace llmx
