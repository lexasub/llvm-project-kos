// RUN: clang-format -style=LLVM -output-replacements-xml -sort-includes %s \
// RUN:   | FileCheck -strict-whitespace %s

// CHECK: <?xml
// CHECK-NEXT: {{<replacements.*incomplete_format='false'}}
// CHECK-NEXT: {{<replacement.*#include &lt;a>&#10;#include &lt;b><}}
// CHECK-NEXT: {{<replacement.*>&#10;<}}
// CHECK-NEXT: {{<replacement.*> <}}
#include <a>
#include <b>

int a;
int *b;
