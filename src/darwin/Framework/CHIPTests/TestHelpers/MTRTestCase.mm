/**
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h>

#import "MTRTestCase.h"

@implementation MTRTestCase

/**
 * Unfortunately, doing this in "+ (void)tearDown" (the global suite teardown)
 * does not trigger a test failure even if the XCTAssertEqual fails.
 */
- (void)tearDown
{
#if 0
#if defined(ENABLE_LEAK_DETECTION) && ENABLE_LEAK_DETECTION
    if (_detectLeaks) {
        int pid = getpid();
        __auto_type * cmd = [NSString stringWithFormat:@"leaks %d", pid];
        int ret = system(cmd.UTF8String);
        XCTAssertEqual(ret, 0, "LEAKS DETECTED");
    }
#endif
#endif

    [super tearDown];
}

@end
