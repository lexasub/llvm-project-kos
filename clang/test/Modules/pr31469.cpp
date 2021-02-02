// RUN: rm -rf %t
// RUN: %clang_cc1 -std=c++11 -I%S/Inputs/PR31469 -verify %s
// RUN: %clang_cc1 -std=c++11 -I%S/Inputs/PR31469 -fmodules -fmodules-local-submodule-visibility \
// RUN:    -fimplicit-module-maps -fmodules-cache-path=%t  -verify %s

#include "empty.h"
#include "textual.h"

namespace A {
template <class _Tp> void f();
}

A::list<int> use;

// expected-no-diagnostics
