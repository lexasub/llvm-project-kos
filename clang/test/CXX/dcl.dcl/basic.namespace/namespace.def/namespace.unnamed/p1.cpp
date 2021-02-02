// RUN: %clang_cc1 -emit-llvm-only -verify %s

// This lame little test was ripped straight from the standard.
namespace {
int i; // expected-note {{candidate}}
}
void test0() { i++; }

namespace A {
namespace {
int i; // expected-note {{candidate}}
int j;
} // namespace
void test1() { i++; }
} // namespace A

using namespace A;

void test2() {
  i++; // expected-error {{reference to 'i' is ambiguous}}
  A::i++;
  j++;
}

// Test that all anonymous namespaces in a translation unit are
// considered the same context.
namespace {
class Test3 {}; // expected-note {{previous definition}}
} // namespace
namespace {
class Test3 {}; // expected-error {{redefinition of 'Test3'}}
} // namespace

namespace test4 {
namespace {
class Test4 {}; // expected-note {{previous definition}}
} // namespace
namespace {
class Test4 {}; // expected-error {{redefinition of 'Test4'}}
} // namespace
} // namespace test4
