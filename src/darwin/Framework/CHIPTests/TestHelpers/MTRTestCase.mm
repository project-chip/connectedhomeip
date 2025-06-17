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

#import "MTRTestCase.h"

#import "MTRMockCB.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

#include <stdlib.h>
#include <unistd.h>

#if HAVE_NSTASK
// Tasks that are not scoped to a specific test, but rather to a specific test suite.
static NSMutableSet<NSTask *> * sRunningCrossTestTasks;

static void TerminateTask(NSTask * task)
{
    NSLog(@"Terminating task %@", task);

    [task terminate]; // Sends SIGTERM

    // Wait up to 10 seconds for graceful shutdown
    BOOL terminated = NO;
    for (int i = 0; i < 100; i++) {
        if (![task isRunning]) {
            terminated = YES;
            break;
        }
        [NSThread sleepForTimeInterval:0.1];
    }

    if (!terminated) {
        NSLog(@"Force killing unresponsive task %@", task);
        kill(task.processIdentifier, SIGKILL);
    }

    [task waitUntilExit];
}

static void ClearTaskSet(NSMutableSet<NSTask *> * __strong & tasks)
{
    for (NSTask * task in tasks) {
        TerminateTask(task);
    }
    tasks = nil;
}
#endif // HAVE_NSTASK

// Controllers are managed in a weak NSHashTable to allow tests
// to manually shutdown and free controllers where necessary.
// Additionally we need to keep track of when to shut down
// the controller factory.
typedef NS_ENUM(NSInteger, MTRTestScope) {
    MTRTestScopeNone = 0,
    MTRTestScopeSuite,
    MTRTestScopeTestCase,
};
static MTRTestScope sControllerFactoryScope;
static NSHashTable<MTRDeviceController *> * sStartedControllers;

static void ClearControllerSet(NSHashTable<MTRDeviceController *> * __strong & controllers, MTRTestScope scope)
{
    for (MTRDeviceController * controller in controllers) {
        [controller shutdown];
        XCTAssertFalse([controller isRunning]);
    }
    controllers = nil;

    if (sControllerFactoryScope == scope) {
        [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
        sControllerFactoryScope = MTRTestScopeNone;
    }
}

static MTRMockCB * sMockCB;

@implementation MTRTestCase {
#if HAVE_NSTASK
    NSMutableSet<NSTask *> * _runningTasks;
#endif // NSTask
    NSHashTable<MTRDeviceController *> * _startedControllers;
}

+ (void)setUp
{
    [super setUp];

    sMockCB = [[MTRMockCB alloc] init];

    sControllerFactoryScope = MTRTestScopeNone;
    sStartedControllers = [NSHashTable weakObjectsHashTable];

#ifdef DEBUG
    // Force our controllers to only advertise on localhost, to avoid DNS-SD
    // crosstalk.
    [MTRDeviceController forceLocalhostAdvertisingOnly];
#endif // DEBUG

#if HAVE_NSTASK
    sRunningCrossTestTasks = [[NSMutableSet alloc] init];
#endif // HAVE_NSTASK
}

+ (void)tearDown
{
#if HAVE_NSTASK
    ClearTaskSet(sRunningCrossTestTasks);
#endif // HAVE_NSTASK

    ClearControllerSet(sStartedControllers, MTRTestScopeSuite);

    [sMockCB stopMocking];
    sMockCB = nil;
}

+ (MTRMockCB *)mockCoreBluetooth
{
    return sMockCB;
}

- (void)setUp
{
#if HAVE_NSTASK
    _runningTasks = [[NSMutableSet alloc] init];
#endif // HAVE_NSTASK
    _startedControllers = [NSHashTable weakObjectsHashTable];
}

- (void)tearDown
{
#if defined(ENABLE_LEAK_DETECTION) && ENABLE_LEAK_DETECTION
    /**
     * Unfortunately, doing this in "+ (void)tearDown" (the global suite teardown)
     * does not trigger a test failure even if the XCTAssertEqual below fails.
     */
    if (_detectLeaks) {
        int pid = getpid();
        __auto_type * cmd = [NSString stringWithFormat:@"leaks %d", pid];
        int ret = system(cmd.UTF8String);
        if (WIFSIGNALED(ret)) {
            XCTFail(@"leaks unexpectedly stopped by signal %d", WTERMSIG(ret));
        }
        XCTAssertTrue(WIFEXITED(ret), "leaks did not run to completion");
        // The exit status is 0 if no leaks detected, 1 if leaks were detected,
        // something else on error.
        if (WEXITSTATUS(ret) == 1) {
            XCTFail(@"LEAKS DETECTED");
        } else if (WEXITSTATUS(ret) != 0) {
            // leaks failed to actually run correctly.  Ideally we would fail
            // our tests in that case, but this seems to be happening a fair
            // amount, and randomly, on the ARM GitHub runners, with errors
            // like:
            //
            // *** getStackLoggingSharedMemoryAddressFromTask: couldn't find ___mach_stack_logging_shared_memory_address in target task
            // *** task_malloc_get_all_zones: error 1 reading num_zones at 0
            // *** task_malloc_get_all_zones: error 1 reading num_zones at 0
            // *** task_malloc_get_all_zones: error 1 reading num_zones at 0
            // [fatal] unable to instantiate a memory scanner.
            //
            // Just log and ignore for now.
            NSLog(@"leaks failed to run, exit status: %d", WEXITSTATUS(ret));
        }
    }
#endif

#if HAVE_NSTASK
    ClearTaskSet(_runningTasks);
#endif // HAVE_NSTASK

    ClearControllerSet(_startedControllers, MTRTestScopeTestCase);

    [super tearDown];
}

+ (MTRDeviceController *)_createControllerOnTestFabricWithFactoryScope:(MTRTestScope)scope
{
    __auto_type * factory = MTRDeviceControllerFactory.sharedInstance;
    if (sControllerFactoryScope == MTRTestScopeNone) {
        __auto_type * storage = [[MTRTestStorage alloc] init];
        __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
        XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);
        sControllerFactoryScope = scope;
    } else if (sControllerFactoryScope == MTRTestScopeTestCase && scope == MTRTestScopeSuite) {
        sControllerFactoryScope = MTRTestScopeSuite; // extend factory lifetime
    }

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@1 nocSigner:testKeys];
    params.vendorID = @0xFFF1;
    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    return controller;
}

+ (MTRDeviceController *)createControllerOnTestFabric
{
    MTRDeviceController * controller = [self _createControllerOnTestFabricWithFactoryScope:MTRTestScopeSuite];
    [sStartedControllers addObject:controller];
    return controller;
}

- (MTRDeviceController *)createControllerOnTestFabric
{
    MTRDeviceController * controller = [self.class _createControllerOnTestFabricWithFactoryScope:MTRTestScopeTestCase];
    [_startedControllers addObject:controller];
    return controller;
}

+ (void)controllerWithSuiteScopeCreatedBySubclass
{
    sControllerFactoryScope = MTRTestScopeSuite; // Make sure we don't shut down the controller too early.
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

    [sRunningCrossTestTasks addObject:task];
}

- (NSTask *)relaunchTask:(NSTask *)task additionalArguments:(NSArray<NSString *> *)additionalArguments
{
    TerminateTask(task);
    [_runningTasks removeObject:task];

    NSTask * newTask = [[NSTask alloc] init];
    [newTask setExecutableURL:task.executableURL];
    NSMutableArray<NSString *> * arguments = [task.arguments mutableCopy];
    if (additionalArguments != nil) {
        [arguments addObjectsFromArray:additionalArguments];
    }
    [newTask setArguments:arguments];
    newTask.standardOutput = task.standardOutput;
    newTask.standardError = task.standardError;

    [self launchTask:newTask];
    return newTask;
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
