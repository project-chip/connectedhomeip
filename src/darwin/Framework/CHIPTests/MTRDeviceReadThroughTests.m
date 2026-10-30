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

#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

@interface MTRDeviceReadThroughTests : MTRTestCase
@end

@implementation MTRDeviceReadThroughTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:[[MTRTestStorage alloc] init]];
    factoryParams.port = @(kLocalPort);
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    _controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(_controller);

    // No device has this node ID, and an MTRDevice without a delegate does not
    // subscribe, so its cache holds only what the tests inject.
    _device = [MTRDevice deviceWithNodeID:@(1) controller:_controller];
}

- (void)tearDown
{
    [_controller shutdown];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    [super tearDown];
}

- (MTRReadParams *)paramsAssumingUnknownAttributesReportable:(BOOL)reportable
{
    MTRReadParams * params = [[MTRReadParams alloc] init];
    params.assumeUnknownAttributesReportable = reportable;
    return params;
}

- (void)reportClusterRevision:(NSUInteger)revision forClusterPath:(MTRClusterPath *)clusterPath
{
    [_device unitTestInjectAttributeReport:@[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:clusterPath.endpoint
                                                                  clusterID:clusterPath.cluster
                                                                attributeID:@(MTRAttributeIDTypeGlobalAttributeClusterRevisionID)],
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(revision),
        },
    } ]
                          fromSubscription:YES];

    // The injected report is handled on the Matter queue and then the device queue.
    [_controller syncRunOnWorkQueue:^{
    } error:nil];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
}

- (void)testExistingDeviceForNodeIDDoesNotCreateDevice
{
    XCTAssertEqual([_controller _existingDeviceForNodeID:@(1)], _device);

    XCTAssertNil([_controller _existingDeviceForNodeID:@(2)]);
    // Asking did not create an MTRDevice for node 2.
    XCTAssertEqual(_controller.devices.count, 1);
}

- (void)testChangesOmittedAttributeNeedsReadThrough
{
    MTRAttributePath * utcTime = [MTRAttributePath attributePathWithEndpointID:@(0)
                                                                     clusterID:@(MTRClusterIDTypeTimeSynchronizationID)
                                                                   attributeID:@(MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID)];
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:utcTime params:nil]);
    // assumeUnknownAttributesReportable only applies to attributes that are not in the spec.
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:utcTime params:[self paramsAssumingUnknownAttributesReportable:YES]]);
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:utcTime params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

- (void)testChangesOmittedQualityComesFromClusterXML
{
    // The cluster XML gives these attributes the Changes Omitted quality.
    NSArray<MTRAttributePath *> * paths = @[
        [MTRAttributePath attributePathWithEndpointID:@(0)
                                            clusterID:@(MTRClusterIDTypeGroupKeyManagementID)
                                          attributeID:@(MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID)],
        [MTRAttributePath attributePathWithEndpointID:@(0)
                                            clusterID:@(MTRClusterIDTypeSoftwareDiagnosticsID)
                                          attributeID:@(MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID)],
        [MTRAttributePath attributePathWithEndpointID:@(0)
                                            clusterID:@(MTRClusterIDTypeICDManagementID)
                                          attributeID:@(MTRAttributeIDTypeClusterICDManagementAttributeICDCounterID)],
    ];
    for (MTRAttributePath * path in paths) {
        XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:path params:nil], @"%@", path);
    }
}

- (void)testFormerlyChangesOmittedAttributeNeedsReadThroughOnOlderClusterRevisions
{
    MTRClusterPath * powerSource = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypePowerSourceID)];
    MTRAttributePath * batPercentRemaining = [MTRAttributePath attributePathWithEndpointID:powerSource.endpoint
                                                                                 clusterID:powerSource.cluster
                                                                               attributeID:@(MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID)];

    // Power Source revision 3 replaced the C quality on BatPercentRemaining with Q.
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:batPercentRemaining params:nil]);

    [self reportClusterRevision:2 forClusterPath:powerSource];
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:batPercentRemaining params:nil]);

    [self reportClusterRevision:3 forClusterPath:powerSource];
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:batPercentRemaining params:nil]);

    // The revision is per cluster instance: another endpoint has not reported one.
    MTRAttributePath * otherBatPercentRemaining = [MTRAttributePath attributePathWithEndpointID:@(2)
                                                                                      clusterID:powerSource.cluster
                                                                                    attributeID:batPercentRemaining.attribute];
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:otherBatPercentRemaining params:nil]);
}

- (void)testReportableAttributeDoesNotNeedReadThrough
{
    MTRAttributePath * onOff = [MTRAttributePath attributePathWithEndpointID:@(1)
                                                                   clusterID:@(MTRClusterIDTypeOnOffID)
                                                                 attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)];
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:onOff params:nil]);
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:onOff params:[self paramsAssumingUnknownAttributesReportable:YES]]);
    // assumeUnknownAttributesReportable only applies to attributes that are not in the spec.
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:onOff params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

- (void)testUnknownAttributeNeedsReadThroughOnlyWhenParamsSayItIsNotReportable
{
    MTRAttributePath * vendorAttribute = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(0xFFF1FC99) attributeID:@(0x0001)];
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:vendorAttribute params:nil]);
    // assumeUnknownAttributesReportable defaults to YES.
    XCTAssertFalse([_device _attributeReadNeedsReadThroughForPath:vendorAttribute params:[[MTRReadParams alloc] init]]);
    XCTAssertTrue([_device _attributeReadNeedsReadThroughForPath:vendorAttribute params:[self paramsAssumingUnknownAttributesReportable:NO]]);
}

@end
