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

#import <Matter/Matter.h>

// system dependencies
#import <XCTest/XCTest.h>

@interface MTRDeviceTypeTests : XCTestCase

@end

@implementation MTRDeviceTypeTests

- (void)testInvalidID
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(0x100000000)];
    XCTAssertNil(deviceType);
}

- (void)testUnknownID
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(0xFFF15000)];
    XCTAssertNil(deviceType);
}

- (void)testKnownNonUtilityID
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypeMicrowaveOvenID)];
    XCTAssertNotNil(deviceType);
    XCTAssertEqualObjects(deviceType.id, @(0x0079));
    XCTAssertEqualObjects(deviceType.name, @"Microwave Oven");
    XCTAssertFalse(deviceType.isUtility);
}

- (void)testKnownUtilityID
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypeRootNodeID)];
    XCTAssertNotNil(deviceType);
    XCTAssertEqualObjects(deviceType.id, @(0x0016));
    XCTAssertEqualObjects(deviceType.name, @"Root Node");
    XCTAssertTrue(deviceType.isUtility);
}

- (void)testRootNodeID
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypePowerSourceID)];
    XCTAssertNotNil(deviceType);
    XCTAssertEqualObjects(deviceType.id, @(0x0011));
    XCTAssertEqualObjects(deviceType.name, @"Power Source");
    XCTAssertTrue(deviceType.isUtility);
}

@end
