// RUN: %clang_cc1 -fsyntax-only -verify %s

// FIXME: embellish

namespace test0 {
namespace A {
class Foo {
};

void foo(const Foo &foo);
} // namespace A

class Test {
  enum E { foo = 0 };

  void test() {
    foo(A::Foo()); // expected-error {{not a function}}
  }
};
} // namespace test0

// If X contains [...] then Y is empty.
// - a declaration of a class member
namespace test_adl_suppression_by_class_member {
namespace N {
struct T {};
void f(T); // expected-note {{declared here}}
} // namespace N
struct S {
  void f();
  void test() {
    N::T t;
    f(t); // expected-error {{too many arguments}}
  }
};
} // namespace test_adl_suppression_by_class_member

// - a block-scope function declaration that is not a using-declaration
namespace test_adl_suppression_by_block_scope {
namespace N {
struct S {};
void f(S);
} // namespace N
namespace M {
void f(int);
} // namespace M
void test1() {
  N::S s;
  using M::f;
  f(s); // ok
}

void test2() {
  N::S s;
  extern void f(char); // expected-note {{passing argument to parameter here}}
  f(s);                // expected-error {{no viable conversion from 'N::S' to 'char'}}
}

void test3() {
  N::S s;
  extern void f(char); // expected-note {{candidate}}
  using M::f;
  f(s); // expected-error {{no matching function}}
}

void test4() {
  N::S s;
  using M::f;
  extern void f(char); // expected-note {{candidate}}
  f(s);                // expected-error {{no matching function}}
}

} // namespace test_adl_suppression_by_block_scope

// - a declaration that is neither a function nor a function template
namespace test_adl_suppression_by_non_function {
namespace N {
struct S {};
void f(S);
} // namespace N
void test() {
  extern void (*f)();
  N::S s;
  f(s); // expected-error {{too many arguments}}
}
} // namespace test_adl_suppression_by_non_function
