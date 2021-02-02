// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

namespace N1 {
struct X {};
int &f(void *);
} // namespace N1

namespace N2 {
template <typename T> struct Y {};
} // namespace N2

namespace N3 {
void test() {
  int &ir = f((N2::Y<N1::X> *)0);
}
} // namespace N3

int g(void *);
long g(N1::X);

namespace N1 {
void h(int (*)(void *));
}

void test() {
  h((&g));
}
