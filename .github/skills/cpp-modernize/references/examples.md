# Examples

Input:

```text
Modernize this function that concatenates strings using `sprintf` and raw buffers.

void join(const char** parts, size_t count) {
    char buf[1024];
    buf[0] = '\0';
    for (size_t i = 0; i < count; ++i) {
        strcat(buf, parts[i]);
        if (i + 1 < count) strcat(buf, ",");
    }
    puts(buf);
}
```

Expected output:

```text
Summary: Replace unsafe C string handling with `std::string`/`std::string_view` and standard algorithms.

    #include <string>
    #include <string_view>
    #include <iostream>

    void join(std::span<const std::string_view> parts) {
        std::string out;
        for (auto sv : parts) out += sv;
        std::cout << out << '\n';
    }

Reference: [cppreference - basic_string](https://en.cppreference.com/w/cpp/string/basic_string)
```

Input:

```text
Suggest a replacement for this C-style loop that copies into a std::vector:

void copy(const int* arr, size_t n) {
    std::vector<int> v;
    for (size_t i = 0; i < n; ++i) v.push_back(arr[i]);
}
```

Expected output:

```text
Summary: Use iterator- or range-based construction to avoid manual loops.

    #include <vector>
    #include <ranges>

    void copy(const int* arr, size_t n) {
        std::vector<int> v(std::ranges::begin(std::span(arr, n)), std::ranges::end(std::span(arr, n)));
    }

Reference: [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)
```

Input:

```text
Modernize this function that takes a pointer and length into a safer interface:

void process(const int* data, size_t n) {
    for (size_t i = 0; i < n; ++i) handle(data[i]);
}
```

Expected output:

```text
Summary: Use `std::span` for non-owning views and range-based iteration for clarity and safety.

    #include <span>

    void process(std::span<const int> data) {
        for (int v : data) handle(v); // range-based loop
    }

Reference: [cppreference - span](https://en.cppreference.com/w/cpp/container/span)
```

Input:

```text
Suggest a modern return type for this function that uses an out-parameter and a bool success return:

bool load_config(Config& out) {
    if (!file_exists()) return false;
    out = ...; // load config
    return true;
}
```

Expected output:

```text
Summary: Return `std::expected<T, E>` to encode success or a recoverable error rather than using out-params and bool.

    #include <expected>
    #include <string>

    struct Error { int code; std::string message; };

    std::expected<Config, Error> load_config() {
        if (!file_exists()) return std::unexpected(Error{1, "file missing"});
        return Config{/*...*/};
    }

Reference: [cppreference - expected](https://en.cppreference.com/w/cpp/utility/expected)
```

Input:

```text
Replace this snprintf usage with a modern alternative:

char buf[128];
snprintf(buf, sizeof buf, "user=%s id=%d", name, id);
puts(buf);
```

Expected output:

```text
Summary: Use `std::format` for type-safe formatting and clearer intent; note C++23 toolchain flag may be required.

    #include <format>
    #include <cstdio>

    auto s = std::format("user={} id={}", name, id);
    std::puts(s.c_str());

Reference: [cppreference - format](https://en.cppreference.com/w/cpp/utility/format)
```

Input:

```text
Show a simple coroutine-based producer that yields integers lazily.

/* Show a minimal generator-like coroutine */
// generator<int> counter() { for (int i = 0; i < 3; ++i) co_yield i; }
```

Expected output:

```text
Summary: Use coroutines for lazy generators and asynchronous flows; show a minimal generator example (requires C++20 coroutine support).

    #include <coroutine>
    #include <optional>

    // Minimal generator-like coroutine (illustrative)
    struct generator {
        struct promise_type {
            std::optional<int> value_;
            generator get_return_object() { return generator{}; }
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            std::suspend_always yield_value(int v) { value_ = v; return {}; }
            void return_void() {}
            void unhandled_exception() { std::terminate(); }
        };
    };

Reference: [cppreference - coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
```

Input:

```text
Show how to use Concepts to constrain templates, e.g., an Incrementable concept.

template<typename T> concept Incrementable = requires(T a) { ++a; };
```

Expected output:

```text
Summary: Use Concepts to express template requirements and improve diagnostics; show a simple `Incrementable` example.

    template<typename T>
    concept Incrementable = requires(T a) { ++a; };

    template<Incrementable T>
    T inc(T x) { ++x; return x; }

Reference: [cppreference - concepts](https://en.cppreference.com/w/cpp/language/concepts)
```

Input:

```text
Rewrite this using a ranges pipeline with filter and transform:

std::vector<int> in = ...; std::vector<int> out; for (int v : in) if (v%2==0) out.push_back(v*2);
```

Expected output:

```text
Summary: Use `std::ranges` pipelines (`views::filter` and `views::transform`) for clear, composable transformations.

    #include <ranges>

    auto evens_doubled = in | std::views::filter([](int v){ return v % 2 == 0; })
                              | std::views::transform([](int v){ return v * 2; });

    for (int v : evens_doubled) process(v);

Reference: [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)
```

Input:

```text
Show an advanced `std::format` example: padded float and hex formatting.

float val = 3.14159; int id = 255; std::string name = "bob";
```

Expected output:

```text
Summary: Use `std::format` format specifiers for alignment, precision and integer formats (e.g., hex with zero-padding).

    #include <format>
    auto s = std::format("{:<10} id={:04X} val={:8.2f}", name, id, val); // name left-aligned, id hex padded, val width+precision
    std::puts(s.c_str());

Reference: [cppreference - format](https://en.cppreference.com/w/cpp/utility/format)
```
