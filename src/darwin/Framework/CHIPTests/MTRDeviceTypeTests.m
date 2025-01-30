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

- (void)testPowerSource
{
    __auto_type * deviceType = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypePowerSourceID)];
    XCTAssertNotNil(deviceType);
    XCTAssertEqualObjects(deviceType.id, @(0x0011));
    XCTAssertEqualObjects(deviceType.name, @"Power Source");
    XCTAssertTrue(deviceType.isUtility);
}

- (void)testEqualityAndCopying
{
    __auto_type * a1 = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypeMicrowaveOvenID)];
    XCTAssertNotNil(a1);
    XCTAssertTrue([a1 isEqual:a1]);
    XCTAssertFalse([a1 isEqual:nil]);
    XCTAssertFalse([a1 isEqual:@(MTRDeviceTypeIDTypeMicrowaveOvenID)]);

    __auto_type * a2 = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypeMicrowaveOvenID)];
    XCTAssertNotNil(a2);
    XCTAssertEqual(a1.hash, a2.hash);
    XCTAssertTrue([a1 isEqual:a2]);
    XCTAssertTrue([a2 isEqual:a1]);

    __auto_type * b = [MTRDeviceType deviceTypeForID:@(MTRDeviceTypeIDTypePowerSourceID)];
    XCTAssertNotNil(b);
    XCTAssertFalse([a1 isEqual:b]);
    XCTAssertFalse([b isEqual:a1]);
}

@end
