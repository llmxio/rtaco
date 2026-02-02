#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <thread>

#include "rtaco/core/nl_signal.hxx"

using namespace llmx::rtaco;

TEST(SignalTest, SyncAndAsyncSlots) {
    boost::asio::io_context io;

    Signal<int(int, int)> sig(io.get_executor());

    auto conn1 = sig.connect([](int a, int b) { return a + b; }, ExecPolicy::Sync);

    auto conn2 = sig.connect([](int a, int b) { return a * b; }, ExecPolicy::Async);

    // Run the io_context on a background thread so async slots can execute
    auto work = boost::asio::make_work_guard(io);
    std::thread runner([&io] { io.run(); });

    // Emit will block until all async slots complete (combiner collects values).
    auto results = sig.emit(3, 4);

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], 7);
    EXPECT_EQ(results[1], 12);

    work.reset();
    io.stop();
    runner.join();
}
