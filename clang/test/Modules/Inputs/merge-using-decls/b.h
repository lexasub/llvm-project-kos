struct X {
  int v;
  typedef int t;
};

struct YB {
  typedef YB Y;
  int value;
  typedef int type;
};

struct YBRev {
  typedef int value;
  int type;
};

template <typename T> struct C : X, T {
  using T::value;
  using typename T::type;
  using typename X::t;
  using X::v;
};

template <typename T> struct D : X, T {
  // Mismatch in type/non-type-ness.
  using T::type;
  using typename T::value;
  using typename X::t;
  using X::v;
};

#if __cplusplus <= 199711L // C++11 does not allow access declarations
template <typename T> struct E : X, T {
  // Mismatch in using/access-declaration-ness.
  T::value;
  X::v;
};
#endif

template <typename T> struct F : X, T {
  // Mismatch in nested-name-specifier.
  using T::Y::value;
  using typename ::X::t;
  using typename T::Y::type;
  using ::X::v;
};

// Force instantiation.
typedef C<YB>::type I;
typedef D<YBRev>::t I;

#if __cplusplus <= 199711L // C++11 does not allow access declarations
typedef E<YB>::type I;
#endif

typedef F<YB>::type I;
