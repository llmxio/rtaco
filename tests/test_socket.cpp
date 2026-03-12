#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>

#include <cerrno>

#include "rtaco/socket/nl_socket.hxx"
#include "rtaco/socket/nl_socket_guard.hxx"
#include "tests/support/nl_test_hooks.hxx"

using namespace llmx::rtaco;

TEST(SocketTest, DefaultClosed) {
    boost::asio::io_context io;
    Socket s(io, "test-socket");

    EXPECT_FALSE(s.is_open());

    auto rc = s.close();
    EXPECT_TRUE(static_cast<bool>(rc));

    auto rc2 = s.cancel();
    (void)rc2; // cancel() may fail on a non-open socket on some platforms; ensure it
               // doesn't throw
}

TEST(SocketGuardTest, StopNoThrow) {
    boost::asio::io_context io;
    SocketGuard g(io, "test-guard");

    // stop should be safe even if socket not open
    EXPECT_NO_THROW(g.stop());
}

TEST(SocketTest, OpenFailureReturnsExpected) {
    boost::asio::io_context io;
    Socket s(io, "test-socket");

    auto rc = s.open(-1, 0);
    ASSERT_FALSE(static_cast<bool>(rc));
    EXPECT_FALSE(s.is_open());
}

TEST(SocketTest, BindFailureReturnsExpected) {
    boost::asio::io_context io;
    Socket s(io, "test-socket");

    llmx::rtaco::test_hooks::reset();
    llmx::rtaco::test_hooks::fail_socket_bind_once(std::errc::address_in_use);

    auto rc = s.open(NETLINK_ROUTE, 0);
    ASSERT_FALSE(static_cast<bool>(rc));
    EXPECT_EQ(rc.error(), std::make_error_code(std::errc::address_in_use));
    EXPECT_FALSE(s.is_open());
}
