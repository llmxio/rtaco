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
        auto result = co_await control.async_dump_routes();
        if (!result) {
            std::cerr << "async_dump_routes failed: " << result.error().message() << "\n";
            co_return;
        }

        const auto& list = result.value();
        std::cout << "Routes: " << list.size() << "\n";
        for (const auto& r : list) {
            std::cout << "dst=" << r.dst << " gateway=" << r.gateway << " oif=" << r.oif
                      << " table=" << r.table << " proto=" << static_cast<int>(r.protocol)
                      << "\n";
        }
        co_return;
    };

    boost::asio::co_spawn(io, run(), boost::asio::detached);
    io.run();

    return 0;
}
