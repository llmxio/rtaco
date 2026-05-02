#include <gtest/gtest.h>

#include <array>

#include "rtaco/tasks/nl_request_task.hxx"

using namespace llmx::rtaco;

TEST(RequestTaskTest, ValidateReceivedSizeRejectsFullBuffer) {
    constexpr size_t kCapacity = 4096;
    auto result = detail::validate_received_size(kCapacity, kCapacity);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), std::make_error_code(std::errc::message_size));
}

TEST(RequestTaskTest, ValidateReceivedSizeAcceptsPartialBuffer) {
    constexpr size_t kCapacity = 4096;
    auto result = detail::validate_received_size(kCapacity - 1, kCapacity);
    EXPECT_TRUE(result.has_value());
}
