namespace N {
struct SA {
  friend struct foo;
};
} // namespace N
namespace N {
struct foo;
}
