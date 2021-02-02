namespace N {
template <typename> struct A {
  int n;
  A() : n() {}
};

// Trigger instantiation of definition of A<int>.
struct C {
  A<int> a;
};
} // namespace N

// Merge in another declaration and update records.
#include "b1.h"
