// RUN: %clang_cc1 -fsyntax-only -Wall -verify -std=c++11 %s
// RUN: %clang_cc1 -fsyntax-only -Wall -Wno-inline-namespace-reopened-noninline -DSILENCE -verify -std=c++11 %s

namespace X {
#ifndef SILENCE
inline namespace {} // namespace
namespace {}        // namespace
#else
// expected-no-diagnostics
inline namespace {}
namespace {}
#endif
} // namespace X
