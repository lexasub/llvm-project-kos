// RUN: %clang_cc1 -fsyntax-only -verify -std=c++1z %s

namespace [[deprecated]] {} // namespace ]

namespace [[deprecated]] N { // expected-note 4{{'N' has been explicitly marked deprecated here}}
int X;
int Y = X; // Ok
int f();
} // namespace N

int N::f() { // Ok
  return Y;  // Ok
}

void f() {
  int Y = N::f(); // expected-warning {{'N' is deprecated}}
  using N::X;     // expected-warning {{'N' is deprecated}}
  int Z = X;      //Ok
}

void g() {
  using namespace N; // expected-warning {{'N' is deprecated}}
  int Z = Y;         // Ok
}

namespace M = N; // expected-warning {{'N' is deprecated}}

// Shouldn't diag:
[[nodiscard, deprecated("")]] int PR37935();
