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

#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestResponseValues.h"

static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId1 = 0x12344321;

@interface MTRDeviceReportFilterTests : MTRTestCase
@end

@implementation MTRDeviceReportFilterTests {
    BOOL _localTestStorageEnabledBeforeTest;
}

- (void)setUp
{
    [super setUp];
    _localTestStorageEnabledBeforeTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;
}

- (void)tearDown
{
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

- (MTRDeviceTestDelegateWithSubscriptionSetupOverride *)addDelegateToDevice:(MTRDevice *)device
                                               interestedPathsForAttributes:(NSArray *)interestedPathsForAttributes
                                                   interestedPathsForEvents:(NSArray *)interestedPathsForEvents
                                                             attributePaths:(NSMutableArray<MTRAttributePath *> *)attributePaths
                                                                 eventPaths:(NSMutableArray<MTREventPath *> *)eventPaths
{
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * value in data) {
            [attributePaths addObject:value[MTRAttributePathKey]];
        }
    };
    delegate.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * value in data) {
            [eventPaths addObject:value[MTREventPathKey]];
        }
    };
    XCTestExpectation * gotReportEnd = [self expectationWithDescription:@"Report end"];
    delegate.onReportEnd = ^{
        [gotReportEnd fulfill];
    };
    [device addDelegate:delegate queue:dispatch_get_main_queue() interestedPathsForAttributes:interestedPathsForAttributes interestedPathsForEvents:interestedPathsForEvents];
    return delegate;
}

- (void)test001_AttributePathMatchesItsEndpointClusterAndAttribute
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    NSMutableArray<MTRAttributePath *> * attributePaths = [NSMutableArray array];
    __auto_type * interestedPath = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(0)];
    __auto_type * delegate = [self addDelegateToDevice:device interestedPathsForAttributes:@[ interestedPath ] interestedPathsForEvents:@[] attributePaths:attributePaths eventPaths:[NSMutableArray array]];

    [device unitTestInjectAttributeReport:@[
        TestAttributeResponseValue(@(1), @(6), @(0), 1),
        TestAttributeResponseValue(@(2), @(6), @(0), 1),
        TestAttributeResponseValue(@(1), @(6), @(1), 1),
        TestAttributeResponseValue(@(1), @(8), @(0), 1),
    ]
                         fromSubscription:YES];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    XCTAssertEqualObjects(attributePaths, @[ interestedPath ]);
    [device removeDelegate:delegate];
}

- (void)test002_EventPathMatchesItsEndpointClusterAndEvent
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    NSMutableArray<MTREventPath *> * eventPaths = [NSMutableArray array];
    __auto_type * interestedPath = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(59) eventID:@(1)];
    __auto_type * delegate = [self addDelegateToDevice:device interestedPathsForAttributes:@[] interestedPathsForEvents:@[ interestedPath ] attributePaths:[NSMutableArray array] eventPaths:eventPaths];

    [device unitTestInjectEventReport:@[
        TestEventResponseValue(@(1), @(59), @(1)),
        TestEventResponseValue(@(2), @(59), @(1)),
        TestEventResponseValue(@(1), @(59), @(2)),
        TestEventResponseValue(@(1), @(6), @(1)),
    ]];
    // Event reports have no report end; the empty attribute report that follows them on the device queue supplies one.
    [device unitTestInjectAttributeReport:@[] fromSubscription:YES];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    XCTAssertEqualObjects(eventPaths, @[ interestedPath ]);
    [device removeDelegate:delegate];
}

- (void)test003_ClusterPathMatchesItsEndpointAndCluster
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    NSMutableArray<MTRAttributePath *> * attributePaths = [NSMutableArray array];
    NSMutableArray<MTREventPath *> * eventPaths = [NSMutableArray array];
    __auto_type * delegate = [self addDelegateToDevice:device
                          interestedPathsForAttributes:@[ [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(6)] ]
                              interestedPathsForEvents:@[ [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(59)] ]
                                        attributePaths:attributePaths
                                            eventPaths:eventPaths];

    [device unitTestInjectEventReport:@[
        TestEventResponseValue(@(1), @(59), @(1)),
        TestEventResponseValue(@(1), @(59), @(2)),
        TestEventResponseValue(@(2), @(59), @(1)),
        TestEventResponseValue(@(1), @(6), @(1)),
    ]];
    [device unitTestInjectAttributeReport:@[
        TestAttributeResponseValue(@(1), @(6), @(0), 1),
        TestAttributeResponseValue(@(1), @(6), @(1), 1),
        TestAttributeResponseValue(@(2), @(6), @(0), 1),
        TestAttributeResponseValue(@(1), @(8), @(0), 1),
    ]
                         fromSubscription:YES];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    NSArray * expectedAttributePaths = @[
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(0)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(1)],
    ];
    NSArray * expectedEventPaths = @[
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(59) eventID:@(1)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(59) eventID:@(2)],
    ];
    XCTAssertEqualObjects(attributePaths, expectedAttributePaths);
    XCTAssertEqualObjects(eventPaths, expectedEventPaths);
    [device removeDelegate:delegate];
}

- (void)test004_MixedInterestedPathsEachMatchByTheirOwnFields
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    NSMutableArray<MTRAttributePath *> * attributePaths = [NSMutableArray array];
    __auto_type * delegate = [self addDelegateToDevice:device
                          interestedPathsForAttributes:@[
                              [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(6)],
                              [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(6) attributeID:@(0)],
                          ]
                              interestedPathsForEvents:@[]
                                        attributePaths:attributePaths
                                            eventPaths:[NSMutableArray array]];

    [device unitTestInjectAttributeReport:@[
        TestAttributeResponseValue(@(1), @(6), @(3), 1),
        TestAttributeResponseValue(@(2), @(6), @(0), 1),
        TestAttributeResponseValue(@(2), @(6), @(1), 1),
        TestAttributeResponseValue(@(3), @(6), @(0), 1),
    ]
                         fromSubscription:YES];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    NSArray * expectedAttributePaths = @[
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(3)],
        [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(6) attributeID:@(0)],
    ];
    XCTAssertEqualObjects(attributePaths, expectedAttributePaths);
    [device removeDelegate:delegate];
}

// Tests that an interested path of a subclass of a path class matches by its fields, like the path class itself.
- (void)test005_InterestedPathSubclassesMatchByTheirOwnFields
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    NSMutableArray<MTRAttributePath *> * attributePaths = [NSMutableArray array];
    NSMutableArray<MTREventPath *> * eventPaths = [NSMutableArray array];
    id interestedAttributePath = MTRInterestedPathsTestPathAsSubclass([MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(0)], [MTRInterestedPathsTestAttributePath class]);
    id interestedClusterPath = MTRInterestedPathsTestPathAsSubclass([MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(8)], [MTRInterestedPathsTestClusterPath class]);
    id interestedEventPath = MTRInterestedPathsTestPathAsSubclass([MTREventPath eventPathWithEndpointID:@(1) clusterID:@(59) eventID:@(1)], [MTRInterestedPathsTestEventPath class]);
    XCTAssertTrue([interestedAttributePath isMemberOfClass:[MTRInterestedPathsTestAttributePath class]]);
    XCTAssertTrue([interestedClusterPath isMemberOfClass:[MTRInterestedPathsTestClusterPath class]]);
    XCTAssertTrue([interestedEventPath isMemberOfClass:[MTRInterestedPathsTestEventPath class]]);
    __auto_type * delegate = [self addDelegateToDevice:device
                          interestedPathsForAttributes:@[ interestedAttributePath, interestedClusterPath ]
                              interestedPathsForEvents:@[ interestedEventPath ]
                                        attributePaths:attributePaths
                                            eventPaths:eventPaths];

    [device unitTestInjectEventReport:@[
        TestEventResponseValue(@(1), @(59), @(1)),
        TestEventResponseValue(@(1), @(59), @(2)),
        TestEventResponseValue(@(2), @(59), @(1)),
    ]];
    [device unitTestInjectAttributeReport:@[
        TestAttributeResponseValue(@(1), @(6), @(0), 1),
        TestAttributeResponseValue(@(1), @(6), @(1), 1),
        TestAttributeResponseValue(@(2), @(6), @(0), 1),
        TestAttributeResponseValue(@(1), @(8), @(0), 1),
        TestAttributeResponseValue(@(2), @(8), @(0), 1),
    ]
                         fromSubscription:YES];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    NSArray * expectedAttributePaths = @[
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(6) attributeID:@(0)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(8) attributeID:@(0)],
    ];
    XCTAssertEqualObjects(attributePaths, expectedAttributePaths);
    XCTAssertEqualObjects(eventPaths, @[ [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(59) eventID:@(1)] ]);
    [device removeDelegate:delegate];
}
@end
