// RUN: %clang_cc1 -fsyntax-only -verify -std=c++98 -pedantic %s

// Intentionally compiled as C++03 to test the extension warning.

namespace a {}        // namespace a
namespace a {}        // namespace a
inline namespace b {} // namespace b
inline namespace b {} // namespace b
inline namespace {}   // namespace
