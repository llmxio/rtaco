---
name: cpp-modernize
description: "Provide expert C++23 modernization advice, idiomatic refactorings, and examples that reference cppreference.com. Use when a user asks to modernize, refactor, or explain C++ code; always target C++23 and cite cppreference.com for language and library details."
---

# cpp-modernize

This skill provides senior-level guidance for modernizing C++ code to idiomatic, high-performance C++23. Use it when you want concrete refactorings, API recommendations, and short, precise code examples.

## When to use

- Ask to modernize legacy or pre-C++11/14 code to C++23 idioms.
- Request replacements for deprecated or unsafe constructs (e.g., raw pointers, manual resource management, `auto_ptr`).
- Ask for performance-minded rewrites that prefer expressive standard library components (ranges, algorithms, `std::span`, `std::expected`, `std::format`, etc.).
- Request explanations of C++23 features with example code and references to cppreference.com.

## Quick rules and non-negotiables

- **Target standard:** All examples and guidance must assume **C++23**. Use features from C++20/C++23 only when they help clarity, safety, or performance.
- **Authoritative reference:** Always reference the relevant cppreference.com page(s) for the constructs or library components used (e.g., [cppreference - ranges](https://en.cppreference.com/w/cpp/ranges) for ranges). Cite them inline or in a short reference section.
- **Include examples:** Provide short, self-contained example snippets that compile under C++23 and illustrate the suggested change.
- **Prefer standard solutions:** Prefer standard library facilities before third-party libraries unless the latter are explicitly requested.
- **Explain trade-offs briefly:** For each suggestion, mention at most one performance or compatibility trade-off.
- **No code generation beyond examples:** Don’t produce large generated codebases—provide concise examples and clear instructions for application.

## Output requirements

- Include a short explanation (1–3 sentences).
- Provide a code snippet demonstrating the fix or modernization (annotated with one-line comments when helpful).
- Add a one-line reference link to the cppreference.com page that justifies the recommended feature or pattern.

## References

- See `references/guidelines.md` for detailed style notes, code style rules, and formatting conventions.
- See `references/examples.md` for sample prompts and expected outputs.
