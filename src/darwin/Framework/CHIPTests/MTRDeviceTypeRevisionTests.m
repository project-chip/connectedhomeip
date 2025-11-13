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

@interface MTRDeviceTypeRevisionTests : XCTestCase
@end

@implementation MTRDeviceTypeRevisionTests

- (void)testInvalidTypeID
{
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:(id _Nonnull) nil revision:@1]);
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0xC000 revision:@1]); // type > 0xBFFF
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@0x100000000 revision:@1]);
}

- (void)testInvalidRevision
{
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@1 revision:(id _Nonnull) nil]);
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@1 revision:@0]); // < 1
    XCTAssertNil([[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@1 revision:@0x10000]); // > 0xFFFF
}

- (void)testInitWithStruct
{
    MTRDescriptorClusterDeviceTypeStruct * strukt = [[MTRDescriptorClusterDeviceTypeStruct alloc] init];
    strukt.deviceType = @42;
    strukt.revision = @2;
    MTRDeviceTypeRevision * typeRev = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeStruct:strukt];
    XCTAssertNotNil(typeRev);
    XCTAssertEqualObjects(typeRev, [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@42 revision:@2]);
}

- (void)testTypeInformation
{
    __auto_type * typeRev = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypeRootNodeID) revision:@1];
    XCTAssertNotNil(typeRev);
    XCTAssertNotNil(typeRev.typeInformation);
    XCTAssertEqualObjects(typeRev.typeInformation.name, @"Root Node");
}

- (void)testEqualityAndCopying
{
    __auto_type * a1 = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypeMicrowaveOvenID) revision:@1];
    XCTAssertNotNil(a1);
    XCTAssertTrue([a1 isEqual:a1]);
    XCTAssertTrue([a1 isEqual:[a1 copy]]);
    XCTAssertFalse([a1 isEqual:nil]);
    XCTAssertFalse([a1 isEqual:@(MTRDeviceTypeIDTypeMicrowaveOvenID)]);

    __auto_type * a2 = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypeMicrowaveOvenID) revision:@1];
    XCTAssertNotNil(a2);
    XCTAssertEqual(a1.hash, a2.hash);
    XCTAssertTrue([a1 isEqual:a2]);
    XCTAssertTrue([a2 isEqual:a1]);

    __auto_type * b = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypePowerSourceID) revision:@1];
    XCTAssertNotNil(b);
    XCTAssertFalse([a1 isEqual:b]);
    XCTAssertFalse([b isEqual:a1]);

    __auto_type * c = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypeMicrowaveOvenID) revision:@2];
    XCTAssertNotNil(c);
    XCTAssertFalse([c isEqual:a1]);
    XCTAssertFalse([a1 isEqual:c]);
    XCTAssertFalse([c isEqual:b]);
    XCTAssertFalse([b isEqual:c]);
}

- (void)testSecureCoding
{
    MTRDeviceTypeRevision * a = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(MTRDeviceTypeIDTypeMicrowaveOvenID) revision:@1];
    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:a requiringSecureCoding:YES error:NULL];
    MTRDeviceTypeRevision * b = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRDeviceTypeRevision.class fromData:data error:NULL];
    XCTAssertNotNil(b);
    XCTAssertEqualObjects(b.deviceTypeID, a.deviceTypeID);
    XCTAssertEqualObjects(b.deviceTypeRevision, a.deviceTypeRevision);
    XCTAssertTrue([b isEqual:a]);
}

@end
