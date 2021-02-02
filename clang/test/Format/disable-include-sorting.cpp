// RUN: clang-format %s -style=LLVM | FileCheck %s
// RUN: clang-format %s -sort-includes -style="{SortIncludes: false}" | FileCheck %s
// RUN: clang-format %s -sort-includes=false | FileCheck %s -check-prefix=NOT-SORTED

#include <a>
#include <b>
// CHECK: <a>
// CHECK-NEXT: <b>
// NOT-SORTED: <b>
// NOT-SORTED-NEXT: <a>
