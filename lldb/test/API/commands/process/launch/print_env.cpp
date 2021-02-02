#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  char *evil = getenv("EVIL");

  return 0; // Set breakpoint here.
}
