#pragma once

#include <boost/asio/io_context.hpp>

namespace llmx {

class IoPool {
public:
    static boost::asio::io_context& query() {
        static boost::asio::io_context ctx{};
        return ctx;
    }

    static auto executor() -> boost::asio::io_context::executor_type {
        return query().get_executor();
    }
};

} // namespace llmx
