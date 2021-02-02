struct X {
  int v;
  typedef int t;
};

struct YA {
  int value;
  typedef int type;
};

template <typename T> struct C : X, T {
  using T::value;
  using typename T::type;
  using typename X::t;
  using X::v;
};

template <typename T> struct D : X, T {
  using T::value;
  using typename T::type;
  using typename X::t;
  using X::v;
};

template <typename T> struct E : X, T {
  using T::value;
  using typename T::type;
  using typename X::t;
  using X::v;
};

template <typename T> struct F : X, T {
  using T::value;
  using typename T::type;
  using typename X::t;
  using X::v;
};

// Force instantiation.
typedef C<YA>::type I;
typedef D<YA>::type I;
typedef E<YA>::type I;
typedef F<YA>::type I;
