#include <gtest/gtest.h>

#include <array>
#include <vector>

#include <boost/asio/io_context.hpp>

#include "rtaco/socket/nl_socket_guard.hxx"
#include "rtaco/tasks/nl_neighbor_dump_task.hxx"
#include "rtaco/tasks/nl_neighbor_flush_task.hxx"
#include "rtaco/tasks/nl_neighbor_get_task.hxx"
#include "rtaco/tasks/nl_neighbor_probe_task.hxx"

using namespace llmx::rtaco;

namespace {

auto make_malformed_error(uint32_t seq) -> std::vector<uint8_t> {
    std::vector<uint8_t> buf(NLMSG_SPACE(sizeof(nlmsgerr)) - 1, 0);
    auto* header = reinterpret_cast<nlmsghdr*>(buf.data());
    header->nlmsg_len = NLMSG_LENGTH(sizeof(nlmsgerr)) - 1;
    header->nlmsg_type = NLMSG_ERROR;
    header->nlmsg_seq = seq;
    return buf;
}

} // namespace

TEST(NeighborTaskTest, GetTaskMalformedErrorReturnsEproto) {
    boost::asio::io_context io;
    SocketGuard guard{io, "test-neigh-get"};
    std::array<uint8_t, 16> addr{};
    NeighborGetTask task{guard, 1, 77, std::span<uint8_t, 16>{addr}};

    auto message = make_malformed_error(77);
    const auto* header = reinterpret_cast<const nlmsghdr*>(message.data());

    auto result = task.process_message(*header);
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->has_value());
    EXPECT_EQ(result->error(), std::make_error_code(std::errc::protocol_error));
}

TEST(NeighborTaskTest, DumpTaskMalformedErrorReturnsEproto) {
    boost::asio::io_context io;
    SocketGuard guard{io, "test-neigh-dump"};
    NeighborDumpTask task{guard, std::pmr::get_default_resource(), 1, 78};

    auto message = make_malformed_error(78);
    const auto* header = reinterpret_cast<const nlmsghdr*>(message.data());

    auto result = task.process_message(*header);
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->has_value());
    EXPECT_EQ(result->error(), std::make_error_code(std::errc::protocol_error));
}

TEST(NeighborTaskTest, FlushTaskMalformedErrorReturnsEproto) {
    boost::asio::io_context io;
    SocketGuard guard{io, "test-neigh-flush"};
    std::array<uint8_t, 16> addr{};
    NeighborFlushTask task{guard, 1, 79, std::span<uint8_t, 16>{addr}};

    auto message = make_malformed_error(79);
    const auto* header = reinterpret_cast<const nlmsghdr*>(message.data());

    auto result = task.process_message(*header);
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->has_value());
    EXPECT_EQ(result->error(), std::make_error_code(std::errc::protocol_error));
}

TEST(NeighborTaskTest, ProbeTaskMalformedErrorReturnsEproto) {
    boost::asio::io_context io;
    SocketGuard guard{io, "test-neigh-probe"};
    std::array<uint8_t, 16> addr{};
    NeighborProbeTask task{guard, 1, 80, std::span<uint8_t, 16>{addr}};

    auto message = make_malformed_error(80);
    const auto* header = reinterpret_cast<const nlmsghdr*>(message.data());

    auto result = task.process_message(*header);
    ASSERT_TRUE(result.has_value());
    ASSERT_FALSE(result->has_value());
    EXPECT_EQ(result->error(), std::make_error_code(std::errc::protocol_error));
}
