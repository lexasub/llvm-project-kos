#include <cstdio>
#include <cstdlib>

int main() {
  printf("MY_TEST_ENV_VAR=%s\n", getenv("MY_TEST_ENV_VAR"));

  return 0;
}
