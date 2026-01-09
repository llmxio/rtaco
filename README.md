# RTACO

[![CI](https://img.shields.io/github/actions/workflow/status/YOUR_GH_ORG/llmx-rtaco/ci.yml?branch=main)](https://github.com/YOUR_GH_ORG/llmx-rtaco/actions)
[![License](https://img.shields.io/github/license/YOUR_GH_ORG/llmx-rtaco)](LICENSE)
![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)
![Platform: Linux](https://img.shields.io/badge/platform-Linux-informational)
[![Boost.Asio](https://img.shields.io/badge/Boost.Asio-awaitables-informational)](https://www.boost.org/doc/libs/latest/doc/html/boost_asio.html)

**RTACO** (RTnetlink Asio COroutines) is the **coroutine-first, RTNL-only** control-plane library for modern **C++23**.
It provides `co_await`-friendly APIs for **NETLINK_ROUTE** transactions and **kernel notification streams** (routes, links, addresses, neighbors), built on **Boost.Asio executors**.

**Tags:** `netlink` `rtnetlink` `rtnl` `NETLINK_ROUTE` `asio` `awaitable` `coroutines` `c++23` `linux` `routing`

- Docs: `docs/`
- Examples: `examples/`
- API: `include/rtaco/`
