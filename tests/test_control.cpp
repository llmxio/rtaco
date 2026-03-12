#include <gtest/gtest.h>

#include <array>
#include <thread>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include "rtaco/core/nl_control.hxx"
#include "tests/support/nl_test_hooks.hxx"

using namespace llmx::rtaco;

TEST(ControlTest, DumpRoutesReturnsExpectedOnGateWaitFailure) {
    boost::asio::io_context io;
    auto work = boost::asio::make_work_guard(io);
    std::thread runner([&io] { io.run(); });

    Control control(io);

    llmx::rtaco::test_hooks::reset();
    llmx::rtaco::test_hooks::fail_gate_wait_once(std::errc::io_error);

    auto result = control.dump_routes();

    ASSERT_FALSE(static_cast<bool>(result));
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::io_error));

    control.stop();
    work.reset();
    io.stop();
    runner.join();
}
