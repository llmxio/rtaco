#pragma once

#include <functional>
#include <future>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

namespace llmx {
namespace rtaco {

enum class ExecPolicy {
    Sync,
    Async
};

namespace detail {

template<typename T>
auto make_ready_shared_future(T&& value) -> std::shared_future<std::decay_t<T>> {
    std::promise<std::decay_t<T>> promise{};
    promise.set_value(std::forward<T>(value));
    return promise.get_future().share();
}

inline auto make_ready_shared_future() -> std::shared_future<void> {
    std::promise<void> promise{};
    promise.set_value();
    return promise.get_future().share();
}

template<typename R>
struct WaitAllCombiner {
    using result_type = std::vector<R>;

    template<typename InputIterator>
    auto operator()(InputIterator first, InputIterator last) const -> result_type {
        result_type results{};
        for (; first != last; ++first) {
            results.push_back((*first).get());
        }
        return results;
    }
};

template<>
struct WaitAllCombiner<void> {
    using result_type = void;

    template<typename InputIterator>
    void operator()(InputIterator first, InputIterator last) const {
        for (; first != last; ++first) {
            (*first).get();
        }
    }
};

template<typename Signature>
struct SignalTraits;

template<typename R, typename... Args>
struct SignalTraits<R(Args...)> {
    using result_t = R;
    using args_tuple = std::tuple<Args...>;
    using default_combiner = WaitAllCombiner<R>;
};

} // namespace detail

template<typename Signature,
        typename Combiner = typename detail::SignalTraits<Signature>::default_combiner>
class Signal;

template<typename R, typename... Args, typename Combiner>
class Signal<R(Args...), Combiner> {
public:
    using slot_t = std::function<R(Args...)>;
    using future_t = std::shared_future<R>;
    using signal_t = boost::signals2::signal<future_t(Args...), Combiner>;
    using connection_t = boost::signals2::connection;
    using result_t = typename signal_t::result_type;

    explicit Signal(boost::asio::any_io_executor executor)
        : executor_{executor} {}

    template<typename Slot>
    auto connect(Slot&& slot, ExecPolicy policy = ExecPolicy::Sync) -> connection_t {
        auto slot_fn = slot_t{std::forward<Slot>(slot)};

        return signal_
                .connect([slot_fn = std::move(slot_fn), policy, executor = executor_](
                                 Args... args) mutable -> future_t
        {
            auto args_pack = std::make_shared<std::tuple<std::decay_t<Args>...>>(
                    std::forward<Args>(args)...);

            if (policy == ExecPolicy::Sync) {
                if constexpr (std::is_void_v<R>) {
                    std::apply(slot_fn, *args_pack);
                    return detail::make_ready_shared_future();
                } else {
                    return detail::make_ready_shared_future(
                            std::apply(slot_fn, *args_pack));
                }
            }

            if constexpr (std::is_void_v<R>) {
                auto coroutine =
                        [slot_fn, args_pack,
                                executor]() mutable -> boost::asio::awaitable<void>
                {
                    co_await boost::asio::post(executor, boost::asio::use_awaitable);
                    std::apply(slot_fn, *args_pack);
                };

                boost::asio::co_spawn(executor, std::move(coroutine),
                        boost::asio::detached);
                return detail::make_ready_shared_future();
            }

            auto coroutine = [slot_fn, args_pack,
                                     executor]() mutable -> boost::asio::awaitable<R>
            {
                co_await boost::asio::post(executor, boost::asio::use_awaitable);
                co_return std::apply(slot_fn, *args_pack);
            };

            auto future = boost::asio::co_spawn(executor, std::move(coroutine),
                    boost::asio::use_future);
            return future.share();
        });
    }

    auto emit(Args... args) -> result_t {
        return signal_(std::forward<Args>(args)...);
    }

    auto operator()(Args... args) -> result_t {
        return emit(std::forward<Args>(args)...);
    }

private:
    boost::asio::any_io_executor executor_;
    signal_t signal_{};
};

} // namespace rtaco
} // namespace llmx
