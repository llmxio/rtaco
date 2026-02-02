#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>

#include "rtaco/socket/nl_socket.hxx"
#include "rtaco/socket/nl_socket_guard.hxx"

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
