#include <gtest/gtest.h>

#include "rtaco/tasks/nl_request_task.hxx"

using namespace llmx::rtaco;

struct RequestBehaviorChecker {
    void prepare_request() {}
    std::span<const uint8_t> request_payload() const {
        return {};
    }
    auto process_message(const nlmsghdr&)
            -> std::optional<std::expected<int, std::error_code>> {
        return std::nullopt;
    }
};

static_assert(request_behavior<RequestBehaviorChecker, int>,
        "RequestBehaviorChecker should satisfy request_behavior concept");

TEST(RequestTaskCompileTest, ConceptSatisfied) {
    SUCCEED();
}
