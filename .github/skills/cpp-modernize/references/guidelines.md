---
name: cpp-modernize-guidelines
description: "Guidelines for the `cpp-modernize` skill: how to answer, formatting rules, and C++ style rules used by examples and advice."
---

# Guidelines

This file explains how `cpp-modernize` should respond and what it must include, plus a consolidated set of C++ code style rules used for examples and recommendations. These rules are intentionally **toolchain- and project-agnostic** and do not reference third-party or project-specific libraries.

## Core rules

- **Always target C++23.** Use modern language and library features (concepts, ranges, `std::span`, `std::format`, `std::expected`, etc.) only when they clarify intent, improve safety, or measurably improve performance.
- **Cite authoritative references.** For language or library features, include a cppreference.com link where relevant (e.g., [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)).
- **Small, compilable examples.** Examples must be minimal, self-contained, and valid C++23. Prefer 10–30 lines and use short one-line comments for clarity.
- **Explain trade-offs briefly.** Mention at most one trade-off (binary size, compile-time, toolchain requirement) when applicable.
- **Avoid environment assumptions.** If a suggestion requires flags or extra libraries (e.g., `-std=c++23`), mention it briefly.

---

## Code formatting & layout

- Filenames should be lowercase; underscores are allowed to separate words.
- C++ source files commonly use `.cxx` or `.cpp`; headers use `.h` or `.hpp`.
- Files must end with a single newline character.
- **Maximum line length:** 90 characters.
- **Indentation:** 4 spaces; do not use tabs.
- Separate top-level constructs (classes, functions, namespaces) with a single blank line.
- Group and sort `#include` directives, separated by a blank line between groups. Recommended order:
  1. Current file's header
  2. Project headers
  3. External library headers
  4. C++ standard library headers
  5. C standard library headers
- Use `#pragma once` for header guards.

### Spacing & braces

- The bodies of `if`, `for`, and other control blocks must always be enclosed in braces:

```cpp
if (a == b) {
    do_something();
}
```

- The opening brace of a function or method is separated from the signature by a space and remains on the same line. Empty function bodies are `{}` without a line break.

```cpp
void func_foo(Arg& a) {
    // ...
}

Foo::Foo() {}
```

- `case` labels align with `switch`; indent the body of each `case`.

### Asterisk/ampersand and const placement

- Put `*` and `&` adjacent to the type, not the variable name:

```cpp
int* ptr;
int& ref = value;
```

- `const` should precede the type:

```cpp
const int* ptr;
const int& ref;
```

### Literal & initialization style

- Floating-point literals include a decimal point and an appropriate suffix for `float` (e.g., `1.0f`).
- Prefer brace-initialization for fields and complex objects (`Type var{...};`).
- Initialize simple constants and variables using `=` for clarity (`auto val = 10;`).

---

## Naming conventions

- Types (structs, classes, enum classes, template parameters): `CamelCase`.
- Functions, methods, public members, and non-member functions: `snake_case`.
- Private member variables: `snake_case_` (trailing underscore).
- Local variables and parameters: `snake_case`.
- Compile-time constants: `UPPER_CASE`.
- Local non-compile-time constants: `snake_case`.
- Namespaces: lowercase, single word.

---

## Functions & complexity

- Keep functions short and focused where practical:
  - Recommended maximum function length: ~40 lines.
  - Recommended maximum nesting depth: 4 levels.
  - Recommended maximum parameters: 5 (use a struct for more).
  - Recommended maximum local variables: 10.
- Separate logical blocks within functions with a blank line to improve readability.
- Prefer `inline` functions for small utilities where appropriate.

---

## Conditionals & loops

- Avoid overly complex conditionals; use boolean variables to express intent when helpful.
- Parenthesize sub-expressions for clarity.
- Prefer `switch` over long `if-else` chains when applicable.
- Do not put unrelated expressions in the `for (...)` header.
- For infinite loops, prefer `while (true)`.

---

## Classes & members

- Use `struct` for plain aggregates; use `class` when behavior or encapsulation is required.
- Mark single-argument constructors `explicit` unless implicit conversion is desired.
- When overriding virtual functions, mark them `override` (or `final` where appropriate).
- Order class sections consistently (recommended): types and private fields, public API, protected, private methods. Keep declaration order consistent with definitions.
- Initialize all members in constructors and prefer brace-initialization for fields: `uint32_t field_{123};`.
- Access specifiers (public/protected/private) appear without indentation and should be separated by a blank line from other sections.

---

## Types & integers

- Prefer fixed-width integer types from `<cstdint>` (`int32_t`, `uint64_t`) for interfaces that need precise widths.
- Standard types like `size_t` and `ptrdiff_t` remain acceptable for their intended uses.
- Prefer `nullptr` over `NULL`.

---

## Comments & documentation

- Comments should start with a capital letter. For single-sentence comments, omit the final period.
- Use single-line `//` comments with a single space after `//`.
- Use `// TODO: <reason>` for short work-in-progress notes.

---

## Macros, RTTI & other constructs

- Prefer not to use macros; prefer `constexpr`, `inline` functions, `enum`s, and templates instead.
- If macros are required, keep them local, `#undef` them after use, avoid side effects, and favor uppercase names with a unique prefix.
- Avoid RTTI and user-defined literals unless there is a specific, well-justified need.
- Use of `goto` is discouraged.

---

## Example: short style snippets

```cpp
// Namespace (no indentation inside namespace)
namespace mylib {

class Foo {
public:
    Foo() = default;

    void do_work() { /* short function */ }

private:
    int counter_{};
};

} // namespace mylib
```

---

## References

- Use cppreference.com for language or library feature references when suggesting code changes (e.g., [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges)).
