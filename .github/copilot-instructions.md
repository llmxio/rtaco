# Copilot instructions — rtaco

Short, actionable guidance for AI contributors working on rtaco (C++23, Linux).

## Quick project summary

- RTACO is a coroutine-first, RTNL-only control-plane library for Linux (C++23). See `README.md`.
- Primary API surface: header files under `include/rtaco/` (e.g. `nl_control.hxx`, `nl_listener.hxx`).
- Implementation lives in `src/` (task implementations, socket guards, etc.).
- Key external deps (build-time/runtime): Boost.Asio (awaitables/co_spawn), Linux `netlink` headers, and llmx libraries (`llmx/core`, `llmx/net`, `llmx/nl`).

## Design & architecture notes (what an agent must know)

- Coroutine-first design: prefer `async_*` methods that return `boost::asio::awaitable<expected<T, std::error_code>>`.
  - Example: `Control::async_dump_routes()` is the awaitable implementation; `Control::dump_routes()` wraps it with `co_spawn(..., use_future)` and `.get()` for a synchronous API.
- Error handling uses `expected<T, std::error_code>` and `std::unexpected` for failure returns; avoid throwing exceptions for control-plane errors.
- IPC model: NETLINK_ROUTE socket per `Socket`/`SocketGuard`, tasks build requests and implement `async_run()` which co_awaits netlink replies and returns an `expected` result.
- Event delivery: `Listener` exposes typed signals (Link/Address/Route/Neighbor) using `Signal`/`boost::signals2`; register handlers with `connect_to_event()` and an `ExecPolicy`.
- Platform constraints: Linux-only (NETLINK/AF_INET6 specific), many code paths assume `RT_TABLE_MAIN`, IPv6 family filtering, and kernel capabilities (requires appropriate privileges to send some requests).

## Conventions and patterns to follow

- Naming: `async_*` for awaitable asynchronous APIs; synchronous wrappers exist and should mirror async names without the prefix (e.g., `async_get_neighbor` / `get_neighbor`).
- Tasks: derive from `RequestTask/RouteTask/...`, accept `(SocketGuard&, uint16_t ifindex, uint32_t sequence)` in constructors, and implement `process_message()` / `handle_*` helpers (see `src/nl_route_dump_task.cxx`).
- Logging: use the project's logging facility (for example, `llmx/core/logger`) for instrumentation and debugging messages.
- Resource semantics: prefer deleted copy constructors, defaulted/move where appropriate, and `noexcept` for basic operations (existing code follows this idiom).
- Error conversion: convert kernel errors via helpers like `from_errno()` and return `std::unexpected{error}` where appropriate (see `handle_error()` in dump tasks).

## Build, test, and debug workflow

- Platform: build and test on Linux (netlink code won't run on other OSes).
- Build: repository is configured to use CMake (C++23) and the workspace has standard CMake tasks in the editor (`CMake: Configure`, `CMake: Build`, `CMake: Install`). Use those tasks or run the equivalent `cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=RELEASE` then `cmake --build build`.
- Run-time: running netlink control/listener functionality generally requires netlink privileges (either run as root or give the process appropriate capabilities, e.g., CAP_NET_ADMIN).
- Debugging tips:
  - Use `ip` and `ip -6 neigh` / `ip -6 route` to generate events and validate behavior.
  - For unit-like tests, run small binaries under root or in a network namespace to avoid affecting host state.
- Inspect logged messages produced by the project's logging facility for sequence mismatches or netlink errors.

## What to change and how to add features

- Adding a new control operation:
  1. Add a new Task class (header in `include/rtaco/` and implementation in `src/`) that follows the existing task pattern: constructor `(SocketGuard&, uint16_t ifindex, uint32_t sequence)`, `prepare_request()`, `process_message()`.
  2. Add `async_*` method in `include/rtaco/nl_control.hxx` that constructs the task and `co_await`'s `async_run()`; add a synchronous wrapper using `co_spawn(..., use_future)` if needed.
  3. Use `expected<T, std::error_code>` for results and `std::unexpected` for failures.
  4. Add logging (INFO/WARN/ERROR) where helpful.

## Files to consult for examples

- `include/rtaco/nl_control.hxx` + `src/nl_control.cxx` (async vs sync wrapper pattern)
- `include/rtaco/nl_listener.hxx` + `src/nl_listener.cxx` (event handling + signals)
- `src/nl_route_dump_task.cxx` (task message processing and filtering)
- `include/rtaco/nl_socket_guard.hxx` + `src/nl_socket_guard.cxx` (socket lifecycle management)

## Do NOT do

- Don't change public API shape without a clear migration plan (both `async_*` and sync wrappers are part of the public API surface).
- Don't replace `expected<>`-based error flows with exceptions—preserve the project's error handling style.
- NEVER use the unsigned literal `U` for the value `0`; write `0` instead of `0U`.
- ALWAYS use parentheses around bitshift operators (e.g., prefer `(1 << 3)` and `(flags & (1 << n))`).

---

If anything in these instructions is unclear or you'd like more examples (e.g., a template for new Tasks or a checklist for PRs), tell me what to add and I'll iterate. ✨
