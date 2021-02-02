// Check that we properly report mmap failure.
// RUN: %clangxx_asan %s -o %t && not %run %t 2>&1 | FileCheck %s
#include <assert.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

static volatile void *x;

int main(int argc, char **argv) {
  struct rlimit mmap_resource_limit = {0, 0};
  assert(0 == setrlimit(RLIMIT_AS, &mmap_resource_limit));
  x = malloc(10000000);
  // CHECK: ERROR: Failed to mmap
  return 0;
}
