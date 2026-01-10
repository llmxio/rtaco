#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>

#include "rtaco/nl_control.hxx"

int main() {
    boost::asio::io_context io;

    llmx::nl::Control control{io};

    auto run = [&control]() -> boost::asio::awaitable<void>
    {
        auto result = co_await control.async_dump_neighbors();
        if (!result) {
            std::cerr << "async_dump_neighbors failed: " << result.error().message()
                      << "\n";
            co_return;
        }

        const auto& list = result.value();
        std::cout << "Neighbors: " << list.size() << "\n";
        for (const auto& n : list) {
            std::cout << "ifindex=" << n.index << " addr=" << n.address
                      << " state=" << n.state_to_string() << "\n";
        }
        co_return;
    };

    boost::asio::co_spawn(io, run(), boost::asio::detached);
    io.run();

    return 0;
}
