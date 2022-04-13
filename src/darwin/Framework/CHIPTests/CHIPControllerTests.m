//
//  CHIPControllerTests.m
//  CHIPControllerTests
/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import <CHIP/CHIP.h>

// system dependencies
#import <XCTest/XCTest.h>

static uint16_t kTestVendorId = 0xFFF1u;

@interface CHIPControllerTests : XCTestCase

@end

@implementation CHIPControllerTests

- (void)testStackLifecycle
{
    __auto_type * stack = [MatterStack singletonStack];
    XCTAssertNotNil(stack);
    XCTAssertFalse([stack isRunning]);

    __auto_type * stackParams = [[MatterStackStartupParams alloc] initWithStorage:nil];
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);

    // Now try to restart the stack.
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);
}

- (void)testControllerLifecycle
{
    __auto_type * stack = [MatterStack singletonStack];
    XCTAssertNotNil(stack);

    __auto_type * stackParams = [[MatterStackStartupParams alloc] initWithStorage:nil];
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [stack startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // now try to restart the controller
    controller = [stack startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);
}

- (void)testStackShutdownShutsDownController
{
    __auto_type * stack = [MatterStack singletonStack];
    XCTAssertNotNil(stack);

    __auto_type * stackParams = [[MatterStackStartupParams alloc] initWithStorage:nil];
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [stack startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);
    XCTAssertFalse([controller isRunning]);
}

- (void)testControllerMultipleShutdown
{
    __auto_type * stack = [MatterStack singletonStack];
    XCTAssertNotNil(stack);

    __auto_type * stackParams = [[MatterStackStartupParams alloc] initWithStorage:nil];
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [stack startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    for (int i = 0; i < 5; i++) {
        [controller shutdown];
        XCTAssertFalse([controller isRunning]);
    }

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);
}

- (void)testControllerInvalidAccess
{
    __auto_type * stack = [MatterStack singletonStack];
    XCTAssertNotNil(stack);

    __auto_type * stackParams = [[MatterStackStartupParams alloc] initWithStorage:nil];
    XCTAssertTrue([stack startup:stackParams]);
    XCTAssertTrue([stack isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [stack startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    [controller shutdown];

    XCTAssertFalse([controller isRunning]);
    XCTAssertFalse([controller getConnectedDevice:1234
                                            queue:dispatch_get_main_queue()
                                completionHandler:^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                                    XCTAssertEqual(error.code, CHIPErrorCodeInvalidState);
                                }]);

    [stack shutdown];
    XCTAssertFalse([stack isRunning]);
}

@end
