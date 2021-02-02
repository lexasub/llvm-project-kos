namespace n {
struct D {
  int i;
  static int anInt() { return 2; }
  int dump() { return i; }
};
} // namespace n

using namespace n;

int foo(D *D) {
  return D->dump(); //% self.expect("expression -- D->dump()",
                    //DATA_TYPES_DISPLAYED_CORRECTLY, substrs = ["int", "2"])
}

int main(int argc, char const *argv[]) {
  D myD{D::anInt()};
  foo(&myD);
  return 0;
}
