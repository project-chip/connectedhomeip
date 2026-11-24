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

#import <os/lock.h>

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
- (NSArray<NSDictionary<NSString *, id> *> *)_unitTestingReportAcrossEndpointCount:(NSUInteger)endpointCount dataVersion:(unsigned int)dataVersion
{
    NSMutableArray * report = [NSMutableArray array];
    for (NSUInteger endpoint = 1; endpoint <= endpointCount; ++endpoint) {
        for (NSUInteger attribute = 0; attribute < 2; ++attribute) {
            [report addObject:@{
                MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(endpoint) clusterID:@(MTRClusterIDTypeUnitTestingID) attributeID:@(attribute)],
                MTRDataKey : @ {
                    MTRDataVersionKey : @(dataVersion),
                    MTRTypeKey : MTRUnsignedIntegerValueType,
                    MTRValueKey : @(attribute),
                }
            }];
        }
    }
    return report;
}

- (void)test006_DataVersionUpdatesLogOneSummaryLinePerReport
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId1) controller:[self createControllerOnTestFabric]];
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    [device setDelegate:delegate queue:queue];

    const NSUInteger clusterCount = 5;
    NSString * deviceTag = [NSString stringWithFormat:@"node 0x%016llX", kDeviceId1];

    __block os_unfair_lock linesLock = OS_UNFAIR_LOCK_INIT;
    NSMutableArray<NSString *> * defaultLines = [NSMutableArray array];
    NSMutableArray<NSString *> * detailLines = [NSMutableArray array];
    MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if (![message containsString:deviceTag]) {
            return;
        }
        os_unfair_lock_lock(&linesLock);
        [(type == MTRLogTypeDetail ? detailLines : defaultLines) addObject:message];
        os_unfair_lock_unlock(&linesLock);
    });
    [self addTeardownBlock:^{
        MTRSetLogCallback(MTRLogTypeError, nil);
    }];

    NSArray<NSString *> * (^linesContaining)(NSArray<NSString *> *, NSString *) = ^(NSArray<NSString *> * lines, NSString * text) {
        os_unfair_lock_lock(&linesLock);
        NSArray * matching = [lines filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF CONTAINS %@", text]];
        os_unfair_lock_unlock(&linesLock);
        return matching;
    };
    void (^clearLines)(void) = ^{
        os_unfair_lock_lock(&linesLock);
        [defaultLines removeAllObjects];
        [detailLines removeAllObjects];
        os_unfair_lock_unlock(&linesLock);
    };

    XCTestExpectation * firstEnd = [self expectationWithDescription:@"first report end"];
    delegate.onReportEnd = ^{
        [firstEnd fulfill];
    };
    [device unitTestInjectAttributeReport:[self _unitTestingReportAcrossEndpointCount:clusterCount dataVersion:1] fromSubscription:YES];
    [self waitForExpectations:@[ firstEnd ] timeout:kTimeoutInSeconds];

    XCTAssertEqual(linesContaining(defaultLines, @" updated DataVersion for <MTRClusterPath").count, 0);
    // Until report end commits pendingDataVersion, each further attribute of a changed cluster
    // notes the version again, so this is one Detail line per attribute (10 here), not per cluster.
    XCTAssertGreaterThanOrEqual(linesContaining(detailLines, @" updated DataVersion for <MTRClusterPath").count, clusterCount);
    NSString * summaryText = [NSString stringWithFormat:@"data versions updated for %lu clusters", (unsigned long) clusterCount];
    XCTAssertEqual(linesContaining(defaultLines, summaryText).count, 1);

    clearLines();
    XCTestExpectation * unchangedEnd = [self expectationWithDescription:@"unchanged report end"];
    delegate.onReportEnd = ^{
        [unchangedEnd fulfill];
    };
    [device unitTestInjectAttributeReport:[self _unitTestingReportAcrossEndpointCount:clusterCount dataVersion:1] fromSubscription:YES];
    [self waitForExpectations:@[ unchangedEnd ] timeout:kTimeoutInSeconds];

    XCTAssertEqual(linesContaining(defaultLines, @"updated DataVersion").count, 0);
    XCTAssertEqual(linesContaining(defaultLines, @"data versions updated").count, 0);
}

@end
