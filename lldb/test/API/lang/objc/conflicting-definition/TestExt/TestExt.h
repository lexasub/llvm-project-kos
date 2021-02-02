#import <Test/Test.h>
#import <TestExt/Foo.h>
struct CMTimeRange;

@interface Test (Stuff)
- (void)doSomethingElse:(CMTimeRange *)range_ptr;
@end
