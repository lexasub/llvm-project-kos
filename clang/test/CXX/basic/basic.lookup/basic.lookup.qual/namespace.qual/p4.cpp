// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

namespace A {
int a;
}

namespace C {
int c;
}

namespace B {
using namespace C;
int b;
} // namespace B

namespace C {
using namespace B;
using namespace A;
} // namespace C

void test() {
  C::a++;
  C::b++;
  C::c++;
}
