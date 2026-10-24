# Coding Standard (CS4433 LMS)

Basis: C++ Core Guidelines (https://isocpp.github.io/CppCoreGuidelines), adapted as follows.

1. **Language:** C++17, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`). Builds clean with `-Wall -Wextra -Wpedantic -Wshadow`.
2. **Naming:** `PascalCase` types; `camelCase` functions and variables; `trailingUnderscore_` private data members; `kConstants`; interfaces start with `I`; namespaces lower-case (`lms`, `lms::data`, `lms::validation`).
3. **Files:** one main class per `.h/.cpp` pair, file named after the class; headers use `#pragma once`; include own header first.
4. **Ownership:** no `new`/`delete`, no raw owning pointers: `std::unique_ptr`/`make_unique`, RAII for DB handles, statements and transactions. Non-owning observers/references are explicit.
5. **Layering:** a layer may include only layers below it. Services depend on interfaces (`I*Repository`, `IPenaltyStrategy`, `IPasswordHasher`, `IClock`), never on SQLite or libsodium types.
6. **Errors:** exceptions derived from `lms::LmsError`; destructors never throw; the presentation layer catches `LmsError`, prints the message and continues. No error codes mixed with exceptions.
7. **Input:** every external value (console, file path, database text) is validated before use (`common/Validation.h`, `DateTime::parse`). Numbers are parsed with full-string checks.
8. **SQL:** only prepared statements with `?N` parameters; the only unparameterised `exec()` calls are fixed strings (`PRAGMA`, `BEGIN`, `COMMIT`, the schema script).
9. **Const-correctness:** getters are `const`; parameters are passed by `const&` unless they are cheap values; `override` on every virtual override; `final` on leaf classes.
10. **Comments:** explain *why* and state contracts (`///` on interfaces); do not restate the code.
11. **Functions:** aim for under ~40 lines and cyclomatic complexity under 10.
12. **Tests:** every business rule has a test that names the rule; boundaries are tested at, just below and just above the limit.
