#include <iostream>
#include <thread>
#include <csignal>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include "rtaco/core/nl_listener.hxx"

// Simple example showing how to use llmx::rtaco::Listener to subscribe to
// link/address/neighbor/route events and print them.
int main() {
    boost::asio::io_context io;
    llmx::rtaco::Listener listener{io};

    auto work = boost::asio::make_work_guard(io);
    std::jthread io_thread([&io]() { io.run(); });

    // Connect handlers for events we care about
    listener.connect_to_event([](const llmx::rtaco::LinkEvent& ev)
    {
        std::cout << "LinkEvent: type=" << static_cast<int>(ev.type)
                  << " ifindex=" << ev.index << " name=" << ev.name
                  << " flags=" << std::to_underlying(ev.flags) << " change=" << ev.change
                  << "\n";
    });

    listener.connect_to_event([](const llmx::rtaco::AddressEvent& ev)
    {
        std::cout << "AddressEvent: type=" << static_cast<int>(ev.type)
                  << " ifindex=" << ev.index << " family=" << static_cast<int>(ev.family)
                  << " addr=" << ev.address
                  << " prefix_len=" << static_cast<int>(ev.prefix_len)
                  << " label=" << ev.label << "\n";
    });

    listener.connect_to_event([](const llmx::rtaco::NeighborEvent& ev)
    {
        std::cout << "NeighborEvent: type=" << static_cast<int>(ev.type)
                  << " ifindex=" << ev.index << " addr=" << ev.address
                  << " lladdr=" << ev.lladdr << " state=" << ev.state_to_string() << "\n";
    });

    listener.connect_to_event([](const llmx::rtaco::RouteEvent& ev)
    {
        std::cout << "RouteEvent: type=" << static_cast<int>(ev.type) << " dst=" << ev.dst
                  << " gateway=" << ev.gateway << " oif=" << ev.oif
                  << " table=" << ev.table << "\n";
    });

    // Log netlink errors if they occur (just print that an error happened)
    listener.connect_to_error([](const nlmsgerr&, const nlmsghdr&)
    { std::cerr << "Received netlink error message\n"; });

    // Start listening
    listener.start();

    // Stop on SIGINT/SIGTERM
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code&, int)
    {
        std::cout << "Stopping listener...\n";
        listener.stop();
        work.reset();
    });

    io_thread.join();

    return 0;
}
