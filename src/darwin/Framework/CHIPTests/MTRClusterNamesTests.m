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

@interface MTRClusterNamesTests : XCTestCase

@end

@implementation MTRClusterNamesTests

- (void)testClusterNames
{
    XCTAssertEqualObjects(MTRClusterNameForID(MTRClusterIDTypeOnOffID), @"OnOff");
    XCTAssertEqualObjects(MTRClusterNameForID(0x0101), @"DoorLock");
    XCTAssertEqualObjects(MTRClusterNameForID(12345678), @"<Unknown clusterID 12345678>");
}

- (void)testAttributeNames
{
    XCTAssertEqualObjects(MTRAttributeNameForID(MTRClusterIDTypeOnOffID, MTRAttributeIDTypeClusterOnOffAttributeOnOffID), @"OnOff");
    XCTAssertEqualObjects(MTRAttributeNameForID(MTRClusterIDTypeOnOffID, MTRAttributeIDTypeClusterOnOffAttributeOnTimeID), @"OnTime");
    XCTAssertEqualObjects(MTRAttributeNameForID(12345678, 0), @"<Unknown clusterID 12345678>");
    XCTAssertEqualObjects(MTRAttributeNameForID(MTRClusterIDTypeOnOffID, 12345678), @"<Unknown attributeID 12345678>");
}

- (void)testCommandNames
{
    XCTAssertEqualObjects(MTRRequestCommandNameForID(MTRClusterIDTypeOnOffID, MTRCommandIDTypeClusterOnOffCommandOnID), @"On");
    XCTAssertEqualObjects(MTRRequestCommandNameForID(MTRClusterIDTypeOnOffID, MTRCommandIDTypeClusterOnOffCommandToggleID), @"Toggle");
    XCTAssertEqualObjects(MTRRequestCommandNameForID(MTRClusterIDTypeGroupsID, MTRCommandIDTypeClusterGroupsCommandAddGroupID), @"AddGroup");
    XCTAssertEqualObjects(MTRRequestCommandNameForID(12345678, 0), @"<Unknown clusterID 12345678>");
    XCTAssertEqualObjects(MTRRequestCommandNameForID(MTRClusterIDTypeAccessControlID, 12345678), @"<Unknown commandID 12345678>");

    XCTAssertEqualObjects(MTRResponseCommandNameForID(MTRClusterIDTypeGroupsID, MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID), @"AddGroupResponse");
    XCTAssertEqualObjects(MTRResponseCommandNameForID(12345678, 0), @"<Unknown clusterID 12345678>");
    XCTAssertEqualObjects(MTRResponseCommandNameForID(MTRClusterIDTypeAccessControlID, 12345678), @"<Unknown commandID 12345678>");
}

- (void)testEventNames
{
    XCTAssertEqualObjects(MTREventNameForID(MTRClusterIDTypeAccessControlID, MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID), @"AccessControlEntryChanged");
    XCTAssertEqualObjects(MTREventNameForID(12345678, 0), @"<Unknown clusterID 12345678>");
    XCTAssertEqualObjects(MTREventNameForID(MTRClusterIDTypeAccessControlID, 12345678), @"<Unknown eventID 12345678>");
}

@end
