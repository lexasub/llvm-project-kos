// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

namespace test0 {
namespace ns0 {
class tag;
int tag();
} // namespace ns0

namespace ns1 {
using ns0::tag;
}

namespace ns2 {
using ns0::tag;
}

using ns1::tag;
using ns2::tag;
} // namespace test0

// PR 5752
namespace test1 {
namespace ns {
void foo();
}

using ns::foo;
void foo(int);

namespace ns {
using test1::foo;
}
} // namespace test1

// PR 14768
namespace PR14768 {
template <typename eT> class Mat;
template <typename eT> class Col : public Mat<eT> {
  using Mat<eT>::operator();
  using Col<eT>::operator();
  void operator()();
};
} // namespace PR14768
