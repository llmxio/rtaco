# AGENTS.md

Practical guide for agentic coding assistants in this repository.

## 1) Project Overview

- Name: `llmx-rtaco` (`llmx::rtaco` alias)
- Language: C++23
- Platform: Linux only (RTNETLINK / `NETLINK_ROUTE`)
- Build system: CMake (>= 3.22), usually with Ninja
- Main code: `include/rtaco/` and `src/`
- Tests: GoogleTest + CTest (`tests/`)

## 2) Important Paths

- Public API headers: `include/rtaco/`
- Core implementation: `src/core/`
- Task/event/socket code: `src/tasks/`, `src/events/`, `src/socket/`
- Test setup: `tests/CMakeLists.txt`
- CMake options/tooling: `cmake/`
- CI workflow: `.github/workflows/ci.yml`
- Copilot rules: `.github/copilot-instructions.md`
- Formatting rules: `.clang-format`

## 3) Build Commands

Run from repository root.

### Configure (release)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Configure with tests

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DRTACO_BUILD_TESTS=ON
```

### Configure with examples

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DRTACO_BUILD_EXAMPLES=ON
```

### Build and install

```bash
cmake --build build -j
cmake --install build
```

### Build docs

```bash
cmake -S . -B build -G Ninja -DRTACO_BUILD_DOCS=ON
cmake --build build --target docs
```

## 4) Test Commands

Tests are available only if configured with `-DRTACO_BUILD_TESTS=ON`.

### Run all tests

```bash
ctest --test-dir build --output-on-failure
```

### Run one test case (preferred)

```bash
ctest --test-dir build -R '^SignalTest\.SyncAndAsyncSlots$' --output-on-failure
```

### List all discovered tests

```bash
ctest --test-dir build -N
```

### Run one gtest directly (fallback)

```bash
./build/tests/test_rtaco --gtest_filter=SignalTest.SyncAndAsyncSlots
```

## 5) Lint / Formatting / Checks

There is no dedicated lint target in CMake.

### Format changed files

```bash
clang-format -i <file1> <file2>
```

`.clang-format` is Google-derived with 4-space indent and column limit 90.

### Baseline verification before finishing

```bash
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## 6) Code Style Guidelines

Follow existing conventions in nearby files before introducing new patterns.

### Naming

- Classes/types: `PascalCase` (`Control`, `RouteDumpTask`)
- Methods/functions/variables: `snake_case` (`dump_routes`, `acquire_socket_token`)
- Async APIs: prefix with `async_`
- Private fields: trailing underscore (`socket_guard_`, `sequence_`)

### Includes

- Keep the established include grouping:
  1) matching project header
  2) C++ standard headers
  3) third-party headers
  4) project headers
- Do not auto-reorder includes (`SortIncludes: Never` in `.clang-format`).

### Formatting

- Use 4 spaces, no tabs.
- Keep lines around 90 columns when practical.
- Apply `clang-format` to touched files.

### Types and APIs

- Keep async-first API symmetry: `async_xxx()` + sync `xxx()` wrapper.
- Use fixed-width integer types for protocol/kernel boundaries.
- Prefer `std::span` for non-owning byte/data views.
- Keep copy/move/noexcept patterns consistent with neighboring code.

### Error handling

- Use `std::expected<T, std::error_code>` as the default error model.
- Return failures with `std::unexpected{err}`.
- Avoid exception-based control flow for netlink/control-plane operations.
- Convert errno/kernel failures into `std::error_code` consistently.

### Netlink/task behavior

- Follow task shape (`prepare_request`, `process_message`, `handle_*`).
- Keep sequence checks (`header.nlmsg_seq`) in message processing paths.
- Keep filtering explicit and conservative for route/address/link/neighbor events.

## 7) Copilot Rules To Preserve

From `.github/copilot-instructions.md`:

- Do not replace `expected<>`-based flows with exceptions.
- Do not change public API shape without a clear migration plan.
- NEVER use `0U`; write `0`.
- ALWAYS parenthesize bitshifts (for example `(1 << n)`).
- Keep async/sync naming symmetry (`async_get_neighbor` / `get_neighbor`).
- Assume Linux runtime and privilege constraints (`CAP_NET_ADMIN` for some ops).

## 8) Cursor Rules Status

- No `.cursorrules` file found.
- No `.cursor/rules/` directory found.
- Re-check and merge those rules here if they are added later.

## 9) Agent Workflow

- Read both header and source before changing API behavior.
- Keep patches focused; avoid unrelated refactors.
- Run targeted tests first, then full test suite when scope is broad.
- If behavior is unclear, add/adjust tests before rewriting internals.
