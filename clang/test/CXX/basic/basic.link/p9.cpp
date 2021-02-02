// RUN: %clang_cc1 -fsyntax-only -verify %s

// FIXME: This test is woefully incomplete.
namespace N {} // namespace N

// First bullet: two names with external linkage that refer to
// different kinds of entities.
void f() {
  int N(); // expected-error{{redefinition}} expected-warning{{interpreted as a function declaration}} expected-note {{replace parentheses with an initializer}}
}
