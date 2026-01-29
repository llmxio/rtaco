# RTACO

[![License](https://img.shields.io/github/license/llmxio/rtaco)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)
![Platform: Linux](https://img.shields.io/badge/platform-Linux-informational)
[![Boost.Asio](https://img.shields.io/badge/Boost.Asio-awaitables-informational)](https://www.boost.org/doc/libs/latest/doc/html/boost_asio.html)

RTACO (RTnetlink Asio COroutines) is a coroutine-first, RTNL-only control-plane library for Linux.
It provides Boost.Asio awaitable APIs for NETLINK_ROUTE transactions and for subscribing to kernel notifications (link, address, route, neighbor).

## Scope

- Linux only (NETLINK_ROUTE / RTNETLINK).
- Control-plane only - no dataplane.
- Results use `std::expected<..., std::error_code>`.

## API at a glance

- `llmx::rtaco::Control` ([include/rtaco/nl_control.hxx](include/rtaco/nl_control.hxx))
  - Dumps: `dump_routes()`, `dump_addresses()`, `dump_links()`, `dump_neighbors()`.
  - Awaitables: `async_dump_routes()`, `async_dump_addresses()`, `async_dump_links()`, `async_dump_neighbors()`.
  - Neighbor ops: `probe_neighbor()`, `flush_neighbor()`, `get_neighbor()` and async variants.

- `llmx::rtaco::Listener` ([include/rtaco/nl_listener.hxx](include/rtaco/nl_listener.hxx))
  - Starts a netlink receive loop and emits typed events via `Signal`.
  - Subscribe via `connect_to_event(...)` for `LinkEvent`, `AddressEvent`, `RouteEvent`, `NeighborEvent`.
  - Use `ExecPolicy::Sync` for inline handlers, or `ExecPolicy::Async` to post handlers onto the executor.

## Build

Dependencies:

- C++23 compiler on Linux
- Boost (Asio, Signals2, System)
- Linux netlink headers (typically from `linux-libc-dev` / kernel headers)

Configure and build (Ninja):

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DRTACO_BUILD_EXAMPLES=ON
cmake --build build
```

Install:

```bash
cmake --install build
```

Consume from CMake:

```cmake
find_package(llmx-rtaco CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE llmx::rtaco)
```

## Usage (minimal)

```cpp
#include <boost/asio/io_context.hpp>

#include <rtaco/nl_control.hxx>
#include <rtaco/nl_listener.hxx>

auto main() -> int {
    boost::asio::io_context io;

    llmx::rtaco::Control ctl{io};
    if (auto routes = ctl.dump_routes(); !routes) {
        // routes.error() is a std::error_code
        return 1;
    }

    llmx::rtaco::Listener listener{io};
    listener.connect_to_event([](const llmx::rtaco::RouteEvent& ev) {
        // do something with ev
    });
    listener.start();

    io.run();
    return 0;
}
```

For a more complete setup, see:

- [examples/nl_control_example.cxx](examples/nl_control_example.cxx)
- [examples/nl_listener_example.cxx](examples/nl_listener_example.cxx)

## Privileges

Some operations (for example neighbor probe/flush) typically require `CAP_NET_ADMIN`.
The examples may need to be run under `sudo` or with capabilities, depending on your environment.

## License

See [LICENSE](LICENSE).
