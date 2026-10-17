/**
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

static const uint16_t kTimeoutInSeconds = 3;

@interface MTRTestAttributePathSubclass : MTRAttributePath
@end

@implementation MTRTestAttributePathSubclass
@end

@protocol MTRTestAttributeReportXPCProtocol
- (oneway void)receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport;
@end

@interface MTRTestAttributeReportXPCServer : NSObject <NSXPCListenerDelegate, MTRTestAttributeReportXPCProtocol>
@property (atomic, copy, nullable) void (^onReport)(NSArray<NSDictionary<NSString *, id> *> * attributeReport);
@end

@implementation MTRTestAttributeReportXPCServer

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    __auto_type * interface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRTestAttributeReportXPCProtocol)];
    [interface setClasses:[NSSet setWithArray:@[ NSArray.class, NSDictionary.class, NSString.class, NSNumber.class, MTRAttributePath.class ]]
              forSelector:@selector(receivedAttributeReport:)
            argumentIndex:0
                  ofReply:NO];
    newConnection.exportedInterface = interface;
    newConnection.exportedObject = self;
    [newConnection activate];
    return YES;
}

- (oneway void)receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    self.onReport(attributeReport);
}

@end

static uint64_t MTRTestWorstChurnCallNs(NSUInteger calls, NSUInteger * nextAttribute)
{
    uint64_t worstNs = 0;
    for (NSUInteger i = 0; i < calls; i++) {
        @autoreleasepool {
            NSNumber * attribute = @((*nextAttribute)++);
            uint64_t start = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
            MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(8) clusterID:@(0xFFF1FC0A) attributeID:attribute];
            uint64_t elapsed = clock_gettime_nsec_np(CLOCK_UPTIME_RAW) - start;
            (void) path;
            worstNs = MAX(worstNs, elapsed);
        }
    }
    return worstNs;
}

static void MTRTestWarmUpChurn(NSUInteger calls, NSUInteger * nextAttribute)
{
    (void) MTRTestWorstChurnCallNs(calls, nextAttribute);
}

@interface MTRClusterPathSharingTests : XCTestCase
@end

@implementation MTRClusterPathSharingTests

- (void)test029_PathsBehavior
{
    __auto_type * commandPath1 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(3)];
    __auto_type * commandPath2 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(4)];
    __auto_type * commandPath3 = [MTRCommandPath commandPathWithEndpointID:@(2) clusterID:@(2) commandID:@(3)];
    __auto_type * commandPath4 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(1) commandID:@(3)];
    __auto_type * commandPath5 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(3)];

    __auto_type * eventPath1 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(3)];
    __auto_type * eventPath2 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(4)];
    __auto_type * eventPath3 = [MTREventPath eventPathWithEndpointID:@(2) clusterID:@(2) eventID:@(3)];
    __auto_type * eventPath4 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(1) eventID:@(3)];
    __auto_type * eventPath5 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(3)];

    __auto_type * attributePath1 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(3)];
    __auto_type * attributePath2 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(4)];
    __auto_type * attributePath3 = [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(2) attributeID:@(3)];
    __auto_type * attributePath4 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(3)];
    __auto_type * attributePath5 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(3)];

    __auto_type * clusterPath1 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)];
    __auto_type * clusterPath2 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(3)];
    __auto_type * clusterPath3 = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(2)];
    __auto_type * clusterPath4 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)];

    XCTAssertIdentical(commandPath1, commandPath5);
    XCTAssertIdentical(eventPath1, eventPath5);
    XCTAssertIdentical(attributePath1, attributePath5);
    XCTAssertIdentical(clusterPath1, clusterPath4);
    XCTAssertIdentical([attributePath1 copy], attributePath1);
    for (MTRClusterPath * path in @[ commandPath1, eventPath1, attributePath1, clusterPath1 ]) {
        NSData * archive = [NSKeyedArchiver archivedDataWithRootObject:path requiringSecureCoding:YES error:nil];
        XCTAssertIdentical([NSKeyedUnarchiver unarchivedObjectOfClass:path.class fromData:archive error:nil], path);
    }

    // Command paths
    XCTAssertTrue([commandPath1 isEqual:commandPath5]);
    XCTAssertEqualObjects(commandPath1, commandPath5);

    XCTAssertFalse([commandPath1 isEqual:commandPath2]);
    XCTAssertNotEqualObjects(commandPath1, commandPath2);

    XCTAssertFalse([commandPath1 isEqual:commandPath3]);
    XCTAssertNotEqualObjects(commandPath1, commandPath3);

    XCTAssertFalse([commandPath1 isEqual:commandPath4]);
    XCTAssertNotEqualObjects(commandPath1, commandPath4);

    // Event paths
    XCTAssertTrue([eventPath1 isEqual:eventPath5]);
    XCTAssertEqualObjects(eventPath1, eventPath5);

    XCTAssertFalse([eventPath1 isEqual:eventPath2]);
    XCTAssertNotEqualObjects(eventPath1, eventPath2);

    XCTAssertFalse([eventPath1 isEqual:eventPath3]);
    XCTAssertNotEqualObjects(eventPath1, eventPath3);

    XCTAssertFalse([eventPath1 isEqual:eventPath4]);
    XCTAssertNotEqualObjects(eventPath1, eventPath4);

    // Attribute paths
    XCTAssertTrue([attributePath1 isEqual:attributePath5]);
    XCTAssertEqualObjects(attributePath1, attributePath5);

    XCTAssertFalse([attributePath1 isEqual:attributePath2]);
    XCTAssertNotEqualObjects(attributePath1, attributePath2);

    XCTAssertFalse([attributePath1 isEqual:attributePath3]);
    XCTAssertNotEqualObjects(attributePath1, attributePath3);

    XCTAssertFalse([attributePath1 isEqual:attributePath4]);
    XCTAssertNotEqualObjects(attributePath1, attributePath4);

    // Clusters
    XCTAssertTrue([clusterPath1 isEqual:clusterPath4]);
    XCTAssertEqualObjects(clusterPath1, clusterPath4);

    XCTAssertFalse([clusterPath1 isEqual:clusterPath2]);
    XCTAssertNotEqualObjects(clusterPath1, clusterPath2);

    XCTAssertFalse([clusterPath1 isEqual:clusterPath3]);
    XCTAssertNotEqualObjects(clusterPath1, clusterPath3);

    // Mix
    XCTAssertFalse([commandPath1 isEqual:eventPath1]);
    XCTAssertFalse([eventPath1 isEqual:commandPath1]);

    XCTAssertFalse([commandPath1 isEqual:attributePath1]);
    XCTAssertFalse([attributePath1 isEqual:commandPath1]);

    XCTAssertFalse([attributePath1 isEqual:eventPath1]);
    XCTAssertFalse([eventPath1 isEqual:attributePath1]);
    XCTAssertFalse([clusterPath1 isEqual:attributePath1]);
    XCTAssertFalse([clusterPath1 isEqual:eventPath1]);
}

- (void)test029a_SharedPathsFollowObjectLifetime
{
    __weak MTRAttributePath * weakPath;
    @autoreleasepool {
        MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(4) clusterID:@(0xFFF1FC02) attributeID:@(0)];
        weakPath = path;
        XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@(4) clusterID:@(0xFFF1FC02) attributeID:@(0)], path);
        XCTAssertNotIdentical([MTREventPath eventPathWithEndpointID:@(4) clusterID:@(0xFFF1FC02) eventID:@(0)], (id) path);
    }
    XCTAssertNil(weakPath);

    MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(4) clusterID:@(0xFFF1FC02) attributeID:@(1)];
    NSKeyedArchiver * archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding:YES];
    [archiver setClassName:NSStringFromClass(MTRTestAttributePathSubclass.class) forClass:MTRAttributePath.class];
    [archiver encodeObject:path forKey:NSKeyedArchiveRootObjectKey];
    MTRAttributePath * decoded1 = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRTestAttributePathSubclass.class fromData:archiver.encodedData error:nil];
    MTRAttributePath * decoded2 = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRTestAttributePathSubclass.class fromData:archiver.encodedData error:nil];
    XCTAssertEqual(decoded1.class, MTRTestAttributePathSubclass.class);
    XCTAssertEqualObjects(decoded1, decoded2);
    XCTAssertNotIdentical(decoded1, decoded2);
}

- (void)test029b_SharedPathsAcrossThreads
{
    const size_t kWorkers = 16;
    const NSUInteger kKeys = 256;
    NSMutableArray<NSArray<MTRAttributePath *> *> * tables = [NSMutableArray array];
    __block NSUInteger mismatches = 0;
    dispatch_group_t group = dispatch_group_create();
    for (size_t worker = 0; worker < kWorkers; worker++) {
        dispatch_group_async(group, dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
            NSMutableArray<MTRAttributePath *> * table = [NSMutableArray arrayWithCapacity:kKeys];
            NSUInteger workerMismatches = 0;
            for (NSUInteger i = 0; i < 20000; i++) {
                @autoreleasepool {
                    NSUInteger key = (i * 7919 + worker * 131) % kKeys;
                    MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(key % 16) clusterID:@(0xFFF1FC06) attributeID:@(key / 16)];
                    workerMismatches += ![path.endpoint isEqual:@(key % 16)] || ![path.attribute isEqual:@(key / 16)];
                }
            }
            for (NSUInteger key = 0; key < kKeys; key++) {
                [table addObject:[MTRAttributePath attributePathWithEndpointID:@(key % 16) clusterID:@(0xFFF1FC06) attributeID:@(key / 16)]];
            }
            @synchronized(tables) {
                [tables addObject:table];
                mismatches += workerMismatches;
            }
        });
    }
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    XCTAssertEqual(mismatches, 0);
    XCTAssertEqual(tables.count, kWorkers);
    for (NSArray<MTRAttributePath *> * table in tables) {
        for (NSUInteger key = 0; key < kKeys; key++) {
            XCTAssertIdentical(table[key], tables.firstObject[key]);
        }
    }
}

- (void)test029c_HeldPathSurvivesChurn
{
    MTRAttributePath * held = [MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0xFFF1FC07) attributeID:@(0)];
    __weak MTRAttributePath * weakChurned;
    for (NSUInteger i = 1; i <= 50000; i++) {
        @autoreleasepool {
            MTRAttributePath * churned = [MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0xFFF1FC07) attributeID:@(i)];
            weakChurned = churned;
        }
    }
    XCTAssertNil(weakChurned);
    XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@(5) clusterID:@(0xFFF1FC07) attributeID:@(0)], held);
}

- (void)test029e_RepeatedPathTablesShareObjects
{
    const NSUInteger kPaths = 1000;
    const NSUInteger kBuilds = 10;
    NSMutableArray<MTRAttributePath *> * tables = [NSMutableArray arrayWithCapacity:kPaths * kBuilds];
    for (NSUInteger build = 0; build < kBuilds; build++) {
        for (NSUInteger i = 0; i < kPaths; i++) {
            [tables addObject:[MTRAttributePath attributePathWithEndpointID:@(i % 20) clusterID:@(0xFFF1FC08) attributeID:@(i / 20)]];
        }
    }
    NSHashTable * distinct = [NSHashTable hashTableWithOptions:NSPointerFunctionsObjectPointerPersonality];
    for (MTRAttributePath * path in tables) {
        [distinct addObject:path];
    }
    XCTAssertEqual(distinct.count, kPaths);
}

- (void)test029f_XPCDecodedPathIsShared
{
    MTRAttributePath * held = [MTRAttributePath attributePathWithEndpointID:@(6) clusterID:@(0xFFF1FC09) attributeID:@(0)];
    XCTestExpectation * expectation = [self expectationWithDescription:@"report received over XPC"];
    __auto_type * server = [[MTRTestAttributeReportXPCServer alloc] init];
    server.onReport = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertEqual(attributeReport.count, 1);
        XCTAssertIdentical(attributeReport.firstObject[MTRAttributePathKey], held);
        [expectation fulfill];
    };
    NSXPCListener * listener = [NSXPCListener anonymousListener];
    listener.delegate = server;
    [listener resume];
    __auto_type * connection = [[NSXPCConnection alloc] initWithListenerEndpoint:listener.endpoint];
    connection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRTestAttributeReportXPCProtocol)];
    [connection activate];
    [[connection remoteObjectProxy] receivedAttributeReport:@[ @{ MTRAttributePathKey : held, MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(1) } } ]];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
    [connection invalidate];
    [listener invalidate];
}

- (void)test029g_ChurnCallCostIndependentOfLiveCount
{
    const NSUInteger kSmallLive = 1000;
    const NSUInteger kLargeLive = 100 * kSmallLive;
    const NSUInteger kCallsExceedingLargeLive = kLargeLive + kLargeLive / 10;
    const NSUInteger kRounds = 5;
    const uint64_t kMaxWorstCallRatio = 25;
    NSMutableArray<MTRAttributePath *> * live = [NSMutableArray arrayWithCapacity:kLargeLive];
    NSMutableArray<NSNumber *> * smallWorst = [NSMutableArray array];
    NSMutableArray<NSNumber *> * largeWorst = [NSMutableArray array];
    NSUInteger nextAttribute = 0;
    for (NSUInteger round = 0; round < kRounds; round++) {
        @autoreleasepool {
            if (live.count > kSmallLive) {
                [live removeObjectsInRange:NSMakeRange(kSmallLive, live.count - kSmallLive)];
            }
            while (live.count < kSmallLive) {
                [live addObject:[MTRAttributePath attributePathWithEndpointID:@(7) clusterID:@(0xFFF1FC0A) attributeID:@(live.count)]];
            }
        }
        MTRTestWarmUpChurn(kCallsExceedingLargeLive, &nextAttribute);
        [smallWorst addObject:@(MTRTestWorstChurnCallNs(kCallsExceedingLargeLive, &nextAttribute))];
        @autoreleasepool {
            while (live.count < kLargeLive) {
                [live addObject:[MTRAttributePath attributePathWithEndpointID:@(7) clusterID:@(0xFFF1FC0A) attributeID:@(live.count)]];
            }
        }
        [largeWorst addObject:@(MTRTestWorstChurnCallNs(kCallsExceedingLargeLive, &nextAttribute))];
    }
    uint64_t smallWorstMax = [[smallWorst valueForKeyPath:@"@max.self"] unsignedLongLongValue];
    uint64_t largeWorstMin = [[largeWorst valueForKeyPath:@"@min.self"] unsignedLongLongValue];
    NSLog(@"Worst call per round, %lu live: %@ ns; %lu live: %@ ns", (unsigned long) kSmallLive, smallWorst, (unsigned long) kLargeLive, largeWorst);
    XCTAssertLessThan(largeWorstMin, kMaxWorstCallRatio * smallWorstMax);
    XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@(7) clusterID:@(0xFFF1FC0A) attributeID:@(0)], live.firstObject);
}

- (void)test029h_SharedPathIdentityUnderChurn
{
    const size_t kWorkers = 8;
    MTRAttributePath * held = [MTRAttributePath attributePathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) attributeID:@(0)];
    __block NSUInteger mismatches = 0;
    dispatch_group_t group = dispatch_group_create();
    for (size_t worker = 0; worker < kWorkers; worker++) {
        dispatch_group_async(group, dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
            NSUInteger workerMismatches = 0;
            for (NSUInteger i = 0; i < 200000; i++) {
                @autoreleasepool {
                    if (worker % 4 == 0) {
                        (void) [MTRAttributePath attributePathWithEndpointID:@(10 + worker) clusterID:@(0xFFF1FC0B) attributeID:@(i)];
                    } else if (worker % 4 == 1) {
                        workerMismatches += [MTRAttributePath attributePathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) attributeID:@(0)] != held;
                    } else {
                        NSNumber * attribute = @(1 + i % 4);
                        MTRAttributePath * first = [MTRAttributePath attributePathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) attributeID:attribute];
                        MTREventPath * event = [MTREventPath eventPathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) eventID:attribute];
                        workerMismatches += [MTRAttributePath attributePathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) attributeID:attribute] != first;
                        workerMismatches += [MTREventPath eventPathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) eventID:attribute] != event;
                        workerMismatches += [first copy] != first;
                    }
                }
            }
            @synchronized(held) {
                mismatches += workerMismatches;
            }
        });
    }
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    XCTAssertEqual(mismatches, 0);
    XCTAssertIdentical([MTRAttributePath attributePathWithEndpointID:@(9) clusterID:@(0xFFF1FC0B) attributeID:@(0)], held);
}

@end
