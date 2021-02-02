
namespace ns {
int func(void) { return 0; }
} // namespace ns

extern "C" int foo(void) { return ns::func(); }
