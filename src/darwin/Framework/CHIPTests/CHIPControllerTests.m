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

@interface CHIPControllerTests : XCTestCase

@end

@implementation CHIPControllerTests

- (void)testControllerLifecycle
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertTrue([controller startup:nil]);
    XCTAssertTrue([controller shutdown]);

    // now try to restart the controller
    XCTAssertTrue([controller startup:nil]);
    XCTAssertTrue([controller shutdown]);
}

- (void)testControllerMultipleStartup
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    for (int i = 0; i < 5; i++) {
        XCTAssertTrue([controller startup:nil]);
    }
    XCTAssertTrue([controller shutdown]);
}

- (void)testControllerMultipleShutdown
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertTrue([controller startup:nil]);
    for (int i = 0; i < 5; i++) {
        XCTAssertTrue([controller shutdown]);
    }
}

- (void)testControllerInvalidAccess
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    NSError * error;
    XCTAssertFalse([controller isRunning]);
    XCTAssertFalse([controller getConnectedDevice:1234
                                completionHandler:^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                                }
                                            queue:dispatch_get_main_queue()
                                            error:&error]);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidState);
    XCTAssertFalse([controller unpairDevice:1 error:&error]);
    XCTAssertEqual(error.code, CHIPErrorCodeInvalidState);
}

@end
