#include <iostream>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/detached.hpp>

#include "rtaco/core/nl_control.hxx"

int main() {
    boost::asio::io_context io;
    llmx::rtaco::Control control{io};

    auto work = boost::asio::make_work_guard(io);
    std::jthread io_thread([&io]() { io.run(); });

    auto dump_neighbors = [&control]() -> boost::asio::awaitable<void>
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

    auto dump_routes = [&control]() -> boost::asio::awaitable<void>
    {
        auto rc = co_await control.async_dump_routes();
        if (!rc) {
            std::cerr << "async_dump_routes failed: " << rc.error().message() << "\n";
            co_return;
        }

        std::cout << "Routes: " << rc.value().size() << "\n";

        for (const auto& r : rc.value()) {
            std::cout << "dst=" << r.dst << " gateway=" << r.gateway << " oif=" << r.oif
                      << " table=" << r.table << " proto=" << static_cast<int>(r.protocol)
                      << "\n";
        }

        co_return;
    };

    auto dump_addresses = [&control]() -> boost::asio::awaitable<void>
    {
        auto rc = co_await control.async_dump_addresses();

        if (!rc) {
            std::cerr << "async_dump_addresses failed: " << rc.error().message() << "\n";
            co_return;
        }

        std::cout << "Addresses: " << rc.value().size() << "\n";

        for (const auto& a : rc.value()) {
            std::cout << "ifindex=" << a.index << " addr=" << a.address
                      << " prefix_len=" << static_cast<int>(a.prefix_len)
                      << " label=" << a.label << "\n";
        }

        co_return;
    };

    auto dump_links = [&control]() -> boost::asio::awaitable<void>
    {
        auto rc = co_await control.async_dump_links();

        if (!rc) {
            std::cerr << "async_dump_links failed: " << rc.error().message() << "\n";
            co_return;
        }

        std::cout << "Links: " << rc.value().size() << "\n";

        for (const auto& l : rc.value()) {
            std::cout << "ifindex=" << l.index << " name=" << l.name
                      << " flags=" << std::to_underlying(l.flags)
                      << " change=" << l.change << "\n";
        }

        co_return;
    };

    boost::asio::co_spawn(io, dump_addresses(), boost::asio::detached);
    boost::asio::co_spawn(io, dump_routes(), boost::asio::detached);
    boost::asio::co_spawn(io, dump_neighbors(), boost::asio::detached);
    boost::asio::co_spawn(io, dump_links(), boost::asio::detached);

    work.reset();
    io_thread.join();

    return 0;
}
