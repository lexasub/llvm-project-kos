namespace N {
template <typename T> struct A { friend int f(A); };
} // namespace N
int b = f(N::A<int>());
