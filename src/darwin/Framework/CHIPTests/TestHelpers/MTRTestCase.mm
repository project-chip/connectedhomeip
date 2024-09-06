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

#if HAVE_NSTASK
// Tasks that are not scoped to a specific test, but rather to a specific test suite.
static NSMutableSet<NSTask *> * runningCrossTestTasks;

static void ClearTaskSet(NSMutableSet<NSTask *> * __strong & tasks)
{
    for (NSTask * task in tasks) {
        NSLog(@"Terminating task %@", task);
        [task terminate];
    }
    tasks = nil;
}
#endif // HAVE_NSTASK

@implementation MTRTestCase {
#if HAVE_NSTASK
    NSMutableSet<NSTask *> * _runningTasks;
#endif // NSTask
}

+ (void)setUp
{
    [super setUp];

#if HAVE_NSTASK
    runningCrossTestTasks = [[NSMutableSet alloc] init];
#endif // HAVE_NSTASK
}

+ (void)tearDown
{
#if HAVE_NSTASK
    ClearTaskSet(runningCrossTestTasks);
#endif // HAVE_NSTASK
}

- (void)setUp
{
#if HAVE_NSTASK
    _runningTasks = [[NSMutableSet alloc] init];
#endif // HAVE_NSTASK
}

- (void)tearDown
{
#if defined(ENABLE_LEAK_DETECTION) && ENABLE_LEAK_DETECTION
    if (_detectLeaks) {
        int pid = getpid();
        __auto_type * cmd = [NSString stringWithFormat:@"leaks %d", pid];
        int ret = system(cmd.UTF8String);
        /**
         * Unfortunately, doing this in "+ (void)tearDown" (the global suite teardown)
         * does not trigger a test failure even if the XCTAssertEqual fails.
         */
        XCTAssertEqual(ret, 0, "LEAKS DETECTED");
    }
#endif

#if HAVE_NSTASK
    ClearTaskSet(_runningTasks);
#endif // HAVE_NSTASK

    [super tearDown];
}

#if HAVE_NSTASK
- (NSTask *)createTaskForPath:(NSString *)path
{
    return [self.class createTaskForPath:path];
}

+ (NSTask *)createTaskForPath:(NSString *)path
{
    NSTask * task = [[NSTask alloc] init];
    [task setLaunchPath:[self absolutePathFor:path]];
    return task;
}

- (void)runTask:(NSTask *)task
{
    NSError * launchError;
    [task launchAndReturnError:&launchError];
    XCTAssertNil(launchError);

    [task waitUntilExit];
    XCTAssertEqual([task terminationStatus], 0);
}

+ (void)doLaunchTask:(NSTask *)task
{
    NSError * launchError;
    [task launchAndReturnError:&launchError];
    XCTAssertNil(launchError);
}

- (void)launchTask:(NSTask *)task
{
    [self.class doLaunchTask:task];

    [_runningTasks addObject:task];
}

+ (void)launchTask:(NSTask *)task
{
    [self doLaunchTask:task];

    [runningCrossTestTasks addObject:task];
}
#endif // HAVE_NSTASK

- (NSString *)absolutePathFor:(NSString *)matterRootRelativePath
{
    return [self.class absolutePathFor:matterRootRelativePath];
}

+ (NSString *)absolutePathFor:(NSString *)matterRootRelativePath
{
    // Start with the absolute path to our file, then remove the suffix that
    // comes after the path to the Matter SDK root.
    NSString * pathToTest = [NSString stringWithUTF8String:__FILE__];
    NSMutableArray * pathComponents = [[NSMutableArray alloc] init];
    [pathComponents addObject:[pathToTest substringToIndex:(pathToTest.length - @"src/darwin/Framework/CHIPTests/TestHelpers/MTRTestCase.mm".length)]];
    [pathComponents addObjectsFromArray:[matterRootRelativePath pathComponents]];
    return [NSString pathWithComponents:pathComponents];
}

@end
