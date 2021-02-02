// RUN: %clang_cc1 -fsyntax-only -verify -std=c++11 %s

// FIXME: We should probably suppress the warning on reopening an inline
// namespace without the inline keyword if it's not the first opening of the
// namespace in the file, because there's no risk of the inlineness differing
// across TUs in that case.

namespace NIL {}        // namespace NIL
inline namespace NIL {} // namespace NIL
inline namespace IL {}  // namespace IL
namespace IL {}         // namespace IL

namespace {}        // namespace
inline namespace {} // namespace
namespace X {
inline namespace {} // namespace
namespace {}        // namespace
} // namespace X
