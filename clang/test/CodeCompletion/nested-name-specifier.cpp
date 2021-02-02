namespace N {
struct A {};
namespace M {
struct C {};
}; // namespace M
} // namespace N

namespace N {
struct B {};
} // namespace N

N::
    // RUN: %clang_cc1 -fsyntax-only -code-completion-at=%s:12:4 %s -o - | FileCheck -check-prefix=CHECK-CC1 %s
    // CHECK-CC1: A
    // CHECK-CC1: B
    // CHECK-CC1: M
