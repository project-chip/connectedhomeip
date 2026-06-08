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

#import <XCTest/XCTest.h>

#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#define HAVE_NSTASK 0
#else
#define HAVE_NSTASK 1
#endif

@class MTRDeviceController;
@class MTRMockCB;

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestCase : XCTestCase

+ (void)setUp NS_REQUIRES_SUPER;
+ (void)tearDown NS_REQUIRES_SUPER;
- (void)setUp NS_REQUIRES_SUPER;
- (void)tearDown NS_REQUIRES_SUPER;

// It would be nice to do the leak-detection automatically, but running "leaks"
// on every single sub-test is slow, and some of our tests seem to have leaks
// outside Matter.framework.  So have it be opt-in for now, and improve later.
@property (nonatomic) BOOL detectLeaks;

// Creates a device controller on a new fabric with test keys and test storage.
//
// The controller will be shut down, and the controller factory stopped, at the
// end of the current test. Note that it is acceptable for the test itself to
// shut down the returned controller earlier.
- (MTRDeviceController *)createControllerOnTestFabric;

// Creates a device controller on a new fabric with test keys and test storage.
//
// The controller will be shut down, and the controller factory stopped, at the
// end of the test suite. Note that it is acceptable for the test itself to
// shut down the returned controller earlier.
+ (MTRDeviceController *)createControllerOnTestFabric;

// Hook for tests to notify that they have started a controller themselves that
// has test suite scope (and therefore the factory needs to stay alive that
// long).
+ (void)controllerWithSuiteScopeCreatedBySubclass;

// Provides access to the mock CoreBlueooth instance managed automatically by
// this class. Bluetooth mocking is enabled for all tests (even those that don't
// actively interact with it) to avoid issues with accessing the real Bluetooth
// implementation in CI.
@property (class, readonly) MTRMockCB * mockCoreBluetooth;

#if HAVE_NSTASK
/**
 * Create an NSTask for the given path.  Path should be relative to the Matter
 * SDK root.
 */
- (NSTask *)createTaskForPath:(NSString *)path;

/**
 * Same thing, but not tied to a specific testcase instance.
 */
+ (NSTask *)createTaskForPath:(NSString *)path;

/**
 * Run a task to completion and make sure it succeeds.
 */
- (void)runTask:(NSTask *)task;

/**
 * Launch a task.  The task will be automatically terminated when the testcase
 * tearDown happens.
 */
- (void)launchTask:(NSTask *)task;

/**
 * Launch a cross-test task.  The task will be automatically terminated when the testsuite
 * tearDown happens.
 */
+ (void)launchTask:(NSTask *)task;

/**
 * Terminates a task, and launches a new task with the same executable URL, standard output
 * and error output handlers as the task to be terminated. The new task will also get the
 * arguments of the task to be terminated, concatenated with the arguments passed in.
 */
- (NSTask *)relaunchTask:(NSTask *)task additionalArguments:(NSArray<NSString *> *)additionalArguments;
#endif // HAVE_NSTASK

/**
 * Get an absolute path from a path relative to the Matter SDK root.
 */
- (NSString *)absolutePathFor:(NSString *)matterRootRelativePath;

/**
 * Same thing, but not tied to a specific testcase instance.
 */
+ (NSString *)absolutePathFor:(NSString *)matterRootRelativePath;

@end

NS_ASSUME_NONNULL_END
