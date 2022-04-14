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

- (void)testFactoryLifecycle
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);

    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:nil];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);

    // Now try to restart the factory.
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerLifecycle
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:nil];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // now try to restart the controller
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testFactoryShutdownShutsDownController
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:nil];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
    XCTAssertFalse([controller isRunning]);
}

- (void)testControllerMultipleShutdown
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:nil];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    for (int i = 0; i < 5; i++) {
        [controller shutdown];
        XCTAssertFalse([controller isRunning]);
    }

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerInvalidAccess
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:nil];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] init];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    [controller shutdown];

    XCTAssertFalse([controller isRunning]);
    XCTAssertFalse([controller getConnectedDevice:1234
                                            queue:dispatch_get_main_queue()
                                completionHandler:^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                                    XCTAssertEqual(error.code, CHIPErrorCodeInvalidState);
                                }]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

@end
