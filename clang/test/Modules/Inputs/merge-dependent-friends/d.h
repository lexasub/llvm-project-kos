namespace N {
template <typename T> struct A { friend int f(A); };
} // namespace N
#include "c.h"
