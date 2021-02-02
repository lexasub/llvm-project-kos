// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

namespace A {
struct B {};
void operator+(B, B);
} // namespace A

using A::operator+;
