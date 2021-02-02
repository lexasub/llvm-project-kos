// RUN: %clang_cl_asan -Od %s -Fe%t
// RUN: %run %t | FileCheck %s

#include <stdio.h>
#include <windows.h>

int main(void) {
  static const char *foo = "foobarspam";
  printf("Global string is `%s`\n", foo);
  // CHECK: Global string is `foobarspam`
  return 0;
}
