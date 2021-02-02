// RUN: %clang_cc1 -fsyntax-only -verify %s

namespace c {
double xxx;
} // namespace c
namespace d {
float xxx;
}
namespace z {
namespace xxx {}
} // namespace z

void crash() {
  switch (xxx) {} // expected-error{{use of undeclared identifier 'xxx'; did you mean }}
}
