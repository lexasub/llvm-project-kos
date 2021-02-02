// RUN: %clang_cc1 -fsyntax-only -verify %s

// C++03 [namespace.udecl]p11: (per DR101)
//   If a function declaration in namespace scope or block scope has
//   the same name and the same parameter types as a function
//   introduced by a using-declaration, and the declarations do not declare the
//   same function, the program is ill-formed. [Note: two using-declarations may
//   introduce functions with the same name and the same parameter types. If,
//   for a call to an unqualified function name, function overload resolution
//   selects the functions introduced by such using-declarations, the function
//   call is ill-formed.]
//
// FIXME: DR565 introduces parallel wording here for function templates.

namespace test0 {
namespace ns {
void foo();
} // namespace ns
int foo(void); // expected-note {{conflicting declaration}}
using ns::foo; // expected-error {{target of using declaration conflicts with declaration already in scope}}
} // namespace test0

namespace test1 {
namespace ns {
void foo();
} // namespace ns
using ns::foo; //expected-note {{using declaration}}
int foo(void); // expected-error {{declaration conflicts with target of using declaration already in scope}}
} // namespace test1

namespace test2 {
namespace ns {
void foo();
} // namespace ns
void test0() {
  int foo(void); // expected-note {{conflicting declaration}}
  using ns::foo; // expected-error {{target of using declaration conflicts with declaration already in scope}}
}

void test1() {
  using ns::foo; //expected-note {{using declaration}}
  int foo(void); // expected-error {{declaration conflicts with target of using declaration already in scope}}
}
} // namespace test2

namespace test3 {
namespace ns {
void foo();
} // namespace ns
class Test0 {
  void test() {
    int foo(void); // expected-note {{conflicting declaration}}
    using ns::foo; // expected-error {{target of using declaration conflicts with declaration already in scope}}
  }
};

class Test1 {
  void test() {
    using ns::foo; //expected-note {{using declaration}}
    int foo(void); // expected-error {{declaration conflicts with target of using declaration already in scope}}
  }
};
} // namespace test3

namespace test4 {
namespace ns {
void foo();
} // namespace ns
template <typename> class Test0 {
  void test() {
    int foo(void); // expected-note {{conflicting declaration}}
    using ns::foo; // expected-error {{target of using declaration conflicts with declaration already in scope}}
  }
};

template <typename> class Test1 {
  void test() {
    using ns::foo; //expected-note {{using declaration}}
    int foo(void); // expected-error {{declaration conflicts with target of using declaration already in scope}}
  }
};
} // namespace test4

// FIXME: we should be able to diagnose both of these, but we can't.
namespace test5 {
namespace ns {
void foo(int);
}
template <typename T> class Test0 {
  void test() {
    int foo(T);
    using ns::foo;
  }
};

template <typename T> class Test1 {
  void test() {
    using ns::foo;
    int foo(T);
  }
};

template class Test0<int>;
template class Test1<int>;
} // namespace test5

namespace test6 {
namespace ns {
void foo();
} // namespace ns
using ns::foo; // expected-note {{using declaration}}
namespace ns {
using test6::foo;
void foo() {}
} // namespace ns
void foo(); // expected-error {{declaration conflicts with target of using declaration already in scope}}
} // namespace test6

namespace test7 {
void foo();
using test7::foo;
void foo() {}
} // namespace test7
