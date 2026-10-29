/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#import "MTRTestDeclarations.h"

@interface MTRDeviceReadThroughTests : XCTestCase
@end

@implementation MTRDeviceReadThroughTests

- (MTRReadParams *)paramsAssumingUnknownAttributesReportable:(BOOL)reportable
{
    MTRReadParams * params = [[MTRReadParams alloc] init];
    params.assumeUnknownAttributesReportable = reportable;
    return params;
}

- (void)testChangesOmittedAttributeNeedsReadThrough
{
    MTRAttributePath * utcTime = [MTRAttributePath attributePathWithEndpointID:@(0)
                                                                     clusterID:@(MTRClusterIDTypeTimeSynchronizationID)
                                                                   attributeID:@(MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID)];
    XCTAssertTrue([MTRDevice _attributeReadNeedsReadThroughForPath:utcTime params:nil]);
    // assumeUnknownAttributesReportable only applies to attributes that are not in the spec.
    XCTAssertTrue([MTRDevice _attributeReadNeedsReadThroughForPath:utcTime params:[self paramsAssumingUnknownAttributesReportable:YES]]);
    XCTAssertTrue([MTRDevice _attributeReadNeedsReadThroughForPath:utcTime params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

- (void)testReportableAttributeDoesNotNeedReadThrough
{
    MTRAttributePath * onOff = [MTRAttributePath attributePathWithEndpointID:@(1)
                                                                   clusterID:@(MTRClusterIDTypeOnOffID)
                                                                 attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)];
    XCTAssertFalse([MTRDevice _attributeReadNeedsReadThroughForPath:onOff params:nil]);
    XCTAssertFalse([MTRDevice _attributeReadNeedsReadThroughForPath:onOff params:[self paramsAssumingUnknownAttributesReportable:YES]]);
    // assumeUnknownAttributesReportable only applies to attributes that are not in the spec.
    XCTAssertFalse([MTRDevice _attributeReadNeedsReadThroughForPath:onOff params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

- (void)testUnknownAttributeNeedsReadThroughOnlyWhenParamsSayItIsNotReportable
{
    MTRAttributePath * vendorAttribute = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(0xFFF1FC99) attributeID:@(0x0001)];
    XCTAssertFalse([MTRDevice _attributeReadNeedsReadThroughForPath:vendorAttribute params:nil]);
    // assumeUnknownAttributesReportable defaults to YES.
    XCTAssertFalse([MTRDevice _attributeReadNeedsReadThroughForPath:vendorAttribute params:[[MTRReadParams alloc] init]]);
    XCTAssertTrue([MTRDevice _attributeReadNeedsReadThroughForPath:vendorAttribute params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

@end
