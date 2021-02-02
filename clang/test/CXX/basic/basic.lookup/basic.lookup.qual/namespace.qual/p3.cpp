// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

// This is basically paraphrased from the standard.

namespace Root {
int i = 0;
void f();
} // namespace Root

namespace A {
using namespace Root;
}

namespace B {
using namespace Root;
}

namespace AB {
using namespace A;
using namespace B;
} // namespace AB

void test() {
  if (AB::i)
    AB::f();
}

namespace C {
using Root::f;
using Root::i;
} // namespace C

namespace AC {
using namespace A;
using namespace C;
} // namespace AC

void test2() {
  if (AC::i)
    AC::f();
}
