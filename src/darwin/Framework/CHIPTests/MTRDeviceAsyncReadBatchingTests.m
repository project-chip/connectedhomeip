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
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static const NSUInteger kMaxPathsPerReadThrough = 9;
static const NSTimeInterval kWaitTimeout = 2;
static char kCallerQueueKey;

@interface MTRAsyncReadBatchingOutcome : NSObject
@property (nonatomic, readonly) MTRAttributePath * path;
@property (atomic) NSUInteger callCount;
@property (atomic, copy, nullable) NSArray<NSDictionary<NSString *, id> *> * values;
@property (atomic, strong, nullable) NSError * error;
@property (atomic) BOOL pathWasAnsweredBeforeCall;
@property (atomic) intptr_t queueTag;
@end

@implementation MTRAsyncReadBatchingOutcome
- (instancetype)initWithPath:(MTRAttributePath *)path
{
    if (self = [super init]) {
        _path = path;
    }
    return self;
}
@end

@interface MTRAsyncReadBatchingReadThrough : NSObject
@property (nonatomic, copy) NSArray<MTRAttributePath *> * paths;
@property (nonatomic, copy) MTRDeviceResponseHandler completion;
@end

@implementation MTRAsyncReadBatchingReadThrough
@end

@interface MTRDeviceAsyncReadBatchingTests : MTRTestCase
@end

@implementation MTRDeviceAsyncReadBatchingTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRDeviceTestDelegateWithSubscriptionSetupOverride * _delegate;
    BOOL _localTestStorageEnabledBeforeTest;
    dispatch_queue_t _callerQueue;
    NSMutableArray<MTRAsyncReadBatchingReadThrough *> * _readThroughs;
    NSMutableSet<MTRAttributePath *> * _answeredPaths;
    NSUInteger _readThroughWaiterCount;
    XCTestExpectation * _readThroughWaiter;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];

    _localTestStorageEnabledBeforeTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:[[MTRTestStorage alloc] init]];
    factoryParams.port = @(kLocalPort);
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    _controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(_controller);

    _device = [MTRDevice deviceWithNodeID:@(1) controller:_controller];

    _callerQueue = dispatch_queue_create("MTRDeviceAsyncReadBatchingTests.caller", DISPATCH_QUEUE_SERIAL);
    _readThroughs = [NSMutableArray array];
    _answeredPaths = [NSMutableSet set];

    // With no subscription every read needs a read-through, and the hook holds each one until the test answers it.
    _delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    _delegate.skipSetupSubscription = YES;
    mtr_weakify(self);
    _delegate.onReadThrough = ^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        mtr_strongify(self);
        if (!self) {
            return;
        }
        [self recordReadThroughOfPaths:paths completion:completion];
    };
    [_device addDelegate:_delegate queue:dispatch_get_main_queue()];
}

- (void)tearDown
{
    _delegate.onReadThrough = nil;
    [_controller shutdown];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    @synchronized(_readThroughs) {
        [_readThroughs removeAllObjects];
        _readThroughWaiter = nil;
    }
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

#pragma mark - Helpers

- (MTRAttributePath *)pathOnEndpoint:(NSUInteger)endpoint
{
    return [MTRAttributePath attributePathWithEndpointID:@(endpoint)
                                               clusterID:@(MTRClusterIDTypeOnOffID)
                                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)];
}

- (NSArray<MTRAttributePath *> *)pathsOnEndpointsFrom:(NSUInteger)first count:(NSUInteger)count
{
    NSMutableArray<MTRAttributePath *> * paths = [NSMutableArray array];
    for (NSUInteger endpoint = first; endpoint < first + count; endpoint++) {
        [paths addObject:[self pathOnEndpoint:endpoint]];
    }
    return paths;
}

- (NSNumber *)valueForPath:(MTRAttributePath *)path
{
    return @(100 + path.endpoint.unsignedIntegerValue);
}

- (NSDictionary<NSString *, id> *)dataReportForPath:(MTRAttributePath *)path
{
    return @{
        MTRAttributePathKey : path,
        MTRDataKey : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : [self valueForPath:path],
        },
    };
}

- (void)recordReadThroughOfPaths:(NSArray<MTRAttributeRequestPath *> *)requestPaths completion:(MTRDeviceResponseHandler)completion
{
    NSMutableArray<MTRAttributePath *> * paths = [NSMutableArray array];
    for (MTRAttributeRequestPath * requestPath in requestPaths) {
        [paths addObject:[MTRAttributePath attributePathWithEndpointID:requestPath.endpoint
                                                             clusterID:requestPath.cluster
                                                           attributeID:requestPath.attribute]];
    }
    __auto_type * readThrough = [[MTRAsyncReadBatchingReadThrough alloc] init];
    readThrough.paths = paths;
    readThrough.completion = completion;

    XCTestExpectation * waiterToFulfill;
    @synchronized(_readThroughs) {
        [_readThroughs addObject:readThrough];
        if (_readThroughWaiter && _readThroughs.count >= _readThroughWaiterCount) {
            waiterToFulfill = _readThroughWaiter;
            _readThroughWaiter = nil;
        }
    }
    [waiterToFulfill fulfill];
}

- (NSArray<MTRAsyncReadBatchingReadThrough *> *)readThroughsSnapshot
{
    @synchronized(_readThroughs) {
        return [_readThroughs copy];
    }
}

- (BOOL)waitForReadThroughCount:(NSUInteger)count
{
    __auto_type * started = [[XCTestExpectation alloc] initWithDescription:[NSString stringWithFormat:@"read-through %lu started", (unsigned long) count]];
    @synchronized(_readThroughs) {
        if (_readThroughs.count >= count) {
            return YES;
        }
        _readThroughWaiterCount = count;
        _readThroughWaiter = started;
    }
    return [XCTWaiter waitForExpectations:@[ started ] timeout:kWaitTimeout] == XCTWaiterResultCompleted;
}

- (void)answerReadThroughAtIndex:(NSUInteger)index withValues:(NSArray<NSDictionary<NSString *, id> *> *)values
{
    __auto_type * readThroughs = [self readThroughsSnapshot];
    if (index >= readThroughs.count) {
        return;
    }
    MTRAsyncReadBatchingReadThrough * readThrough = readThroughs[index];
    @synchronized(_answeredPaths) {
        [_answeredPaths addObjectsFromArray:readThrough.paths];
    }
    readThrough.completion(values, nil);
}

- (void)answerReadThroughAtIndexWithData:(NSUInteger)index
{
    __auto_type * readThroughs = [self readThroughsSnapshot];
    if (index >= readThroughs.count) {
        return;
    }
    NSMutableArray * values = [NSMutableArray array];
    for (MTRAttributePath * path in readThroughs[index].paths) {
        [values addObject:[self dataReportForPath:path]];
    }
    [self answerReadThroughAtIndex:index withValues:values];
}

// Answers each read-through with data as it starts, until `count` have been answered or none starts.
- (void)answerReadThroughsWithDataUpToCount:(NSUInteger)count
{
    for (NSUInteger index = 0; index < count; index++) {
        if (![self waitForReadThroughCount:index + 1]) {
            return;
        }
        [self answerReadThroughAtIndexWithData:index];
    }
}

- (MTRAsyncReadBatchingOutcome *)readAsync:(MTRAttributePath *)path queue:(dispatch_queue_t)queue done:(XCTestExpectation *)done
{
    __auto_type * outcome = [[MTRAsyncReadBatchingOutcome alloc] initWithPath:path];
    NSMutableSet<MTRAttributePath *> * answeredPaths = _answeredPaths;
    [_device readAttributeWithEndpointID:path.endpoint
                               clusterID:path.cluster
                             attributeID:path.attribute
                                  params:nil
                                   queue:queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  @synchronized(answeredPaths) {
                                      outcome.pathWasAnsweredBeforeCall = [answeredPaths containsObject:path];
                                  }
                                  outcome.queueTag = (intptr_t) dispatch_get_specific(&kCallerQueueKey);
                                  outcome.values = values;
                                  outcome.error = error;
                                  outcome.callCount += 1;
                                  [done fulfill];
                              }];
    return outcome;
}

- (MTRAsyncReadBatchingOutcome *)readAsync:(MTRAttributePath *)path done:(XCTestExpectation *)done
{
    return [self readAsync:path queue:_callerQueue done:done];
}

- (XCTestExpectation *)expectationForCompletions:(NSUInteger)count
{
    __auto_type * done = [[XCTestExpectation alloc] initWithDescription:@"async reads completed"];
    done.expectedFulfillmentCount = count;
    done.assertForOverFulfill = NO;
    return done;
}

- (void)waitForCompletions:(XCTestExpectation *)done
{
    XCTWaiterResult result = [XCTWaiter waitForExpectations:@[ done ] timeout:kWaitTimeout];
    XCTAssertEqual(result, XCTWaiterResultCompleted, @"fewer than %lu completions arrived", (unsigned long) done.expectedFulfillmentCount);
    // A second call for the same read would already be on the device queue or the caller's queue.
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    dispatch_sync(_callerQueue, ^ {});
}

- (void)assertOutcome:(MTRAsyncReadBatchingOutcome *)outcome carriesOnlyItsOwnValue:(NSString *)label
{
    XCTAssertEqual(outcome.callCount, 1, @"%@ %@ completion ran %lu times", label, outcome.path, (unsigned long) outcome.callCount);
    XCTAssertNil(outcome.error, @"%@ %@ completion got an error", label, outcome.path);
    XCTAssertEqual(outcome.values.count, 1, @"%@ %@ completion got %lu entries", label, outcome.path, (unsigned long) outcome.values.count);
    XCTAssertEqualObjects(outcome.values.firstObject[MTRAttributePathKey], outcome.path, @"%@ completion got another path's entry", label);
    XCTAssertEqualObjects(outcome.values.firstObject[MTRDataKey][MTRValueKey], [self valueForPath:outcome.path], @"%@ %@ completion got another path's value", label, outcome.path);
}

- (NSArray<NSNumber *> *)batchSizes
{
    NSMutableArray<NSNumber *> * sizes = [NSMutableArray array];
    for (MTRAsyncReadBatchingReadThrough * readThrough in [self readThroughsSnapshot]) {
        [sizes addObject:@(readThrough.paths.count)];
    }
    return sizes;
}

#pragma mark - Batching

- (void)testTenAsyncReadsBehindAHeldReadThroughProduceThreeReadThroughs
{
    __auto_type * done = [self expectationForCompletions:11];
    MTRAsyncReadBatchingOutcome * held = [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];

    NSMutableArray<MTRAsyncReadBatchingOutcome *> * queued = [NSMutableArray array];
    for (MTRAttributePath * path in [self pathsOnEndpointsFrom:2 count:10]) {
        [queued addObject:[self readAsync:path done:done]];
    }

    [self answerReadThroughsWithDataUpToCount:4];
    [self waitForCompletions:done];

    XCTAssertEqual([self readThroughsSnapshot].count, 3, @"10 reads behind a held read-through take 3 read-throughs in all, got batches %@", [self batchSizes]);
    XCTAssertEqualObjects([self batchSizes], (@[ @1, @9, @1 ]));
    [self assertOutcome:held carriesOnlyItsOwnValue:@"held read"];
    for (MTRAsyncReadBatchingOutcome * outcome in queued) {
        [self assertOutcome:outcome carriesOnlyItsOwnValue:@"queued read"];
    }
}

- (void)testNoReadThroughCarriesMoreThanNinePathsAndEachPathIsReadOnce
{
    __auto_type * done = [self expectationForCompletions:21];
    NSMutableArray<MTRAsyncReadBatchingOutcome *> * outcomes = [NSMutableArray array];
    [outcomes addObject:[self readAsync:[self pathOnEndpoint:1] done:done]];
    [self waitForReadThroughCount:1];
    for (MTRAttributePath * path in [self pathsOnEndpointsFrom:2 count:20]) {
        [outcomes addObject:[self readAsync:path done:done]];
    }

    [self answerReadThroughsWithDataUpToCount:6];
    [self waitForCompletions:done];

    NSCountedSet<MTRAttributePath *> * readPaths = [NSCountedSet set];
    for (MTRAsyncReadBatchingReadThrough * readThrough in [self readThroughsSnapshot]) {
        XCTAssertLessThanOrEqual(readThrough.paths.count, kMaxPathsPerReadThrough, @"a read-through carried %lu paths", (unsigned long) readThrough.paths.count);
        [readPaths addObjectsFromArray:readThrough.paths];
    }
    for (MTRAsyncReadBatchingOutcome * outcome in outcomes) {
        XCTAssertEqual([readPaths countForObject:outcome.path], 1, @"%@ was read %lu times, batches %@", outcome.path, (unsigned long) [readPaths countForObject:outcome.path], [self batchSizes]);
    }
    XCTAssertEqualObjects([self batchSizes], (@[ @1, @9, @9, @2 ]));
    for (MTRAsyncReadBatchingOutcome * outcome in outcomes) {
        [self assertOutcome:outcome carriesOnlyItsOwnValue:@"read"];
    }
}

#pragma mark - Per-path results inside a merged batch

- (void)testEachReadInAMergedBatchGetsItsOwnPathsResultWhateverTheResponseOrder
{
    __auto_type * done = [self expectationForCompletions:10];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];
    NSMutableArray<MTRAsyncReadBatchingOutcome *> * queued = [NSMutableArray array];
    for (MTRAttributePath * path in [self pathsOnEndpointsFrom:2 count:9]) {
        [queued addObject:[self readAsync:path done:done]];
    }

    [self answerReadThroughAtIndexWithData:0];
    if ([self waitForReadThroughCount:2]) {
        NSMutableArray * reversed = [NSMutableArray array];
        for (MTRAttributePath * path in [self readThroughsSnapshot][1].paths.reverseObjectEnumerator) {
            [reversed addObject:[self dataReportForPath:path]];
        }
        [self answerReadThroughAtIndex:1 withValues:reversed];
    }
    [self waitForCompletions:done];

    XCTAssertEqualObjects([self batchSizes], (@[ @1, @9 ]), @"the 9 queued reads did not ride one read-through");
    for (MTRAsyncReadBatchingOutcome * outcome in queued) {
        [self assertOutcome:outcome carriesOnlyItsOwnValue:@"batched read"];
    }
}

- (void)testMergedBatchRoutesDataToOnePathAndAPerPathErrorToAnother
{
    MTRAttributePath * dataPath = [self pathOnEndpoint:2];
    MTRAttributePath * errorPath = [self pathOnEndpoint:3];
    NSError * pathError = [NSError errorWithDomain:MTRInteractionErrorDomain code:MTRInteractionErrorCodeUnsupportedAttribute userInfo:nil];

    __auto_type * done = [self expectationForCompletions:3];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];
    MTRAsyncReadBatchingOutcome * dataRead = [self readAsync:dataPath done:done];
    MTRAsyncReadBatchingOutcome * errorRead = [self readAsync:errorPath done:done];

    [self answerReadThroughAtIndexWithData:0];
    if ([self waitForReadThroughCount:2]) {
        [self answerReadThroughAtIndex:1
                            withValues:@[
                                [self dataReportForPath:dataPath],
                                @{ MTRAttributePathKey : errorPath, MTRErrorKey : pathError },
                            ]];
    }
    [self waitForCompletions:done];

    __auto_type * readThroughs = [self readThroughsSnapshot];
    XCTAssertEqual(readThroughs.count, 2, @"the two queued reads did not ride one read-through, batches %@", [self batchSizes]);
    XCTAssertEqualObjects([NSSet setWithArray:readThroughs[1].paths], ([NSSet setWithObjects:dataPath, errorPath, nil]));

    [self assertOutcome:dataRead carriesOnlyItsOwnValue:@"data read"];

    XCTAssertEqual(errorRead.callCount, 1);
    XCTAssertNil(errorRead.error);
    XCTAssertEqual(errorRead.values.count, 1, @"the per-path error read got %@", errorRead.values);
    XCTAssertEqualObjects(errorRead.values.firstObject[MTRAttributePathKey], errorPath);
    XCTAssertNil(errorRead.values.firstObject[MTRDataKey], @"the per-path error read also got data");
    NSError * routedError = errorRead.values.firstObject[MTRErrorKey];
    XCTAssertEqualObjects(routedError.domain, MTRInteractionErrorDomain);
    XCTAssertEqual(routedError.code, MTRInteractionErrorCodeUnsupportedAttribute);
}

- (void)testTwoReadsOfOnePathBehindAHeldReadThroughEachCompleteOnce
{
    MTRAttributePath * path = [self pathOnEndpoint:2];
    __auto_type * done = [self expectationForCompletions:3];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];
    MTRAsyncReadBatchingOutcome * first = [self readAsync:path done:done];
    MTRAsyncReadBatchingOutcome * second = [self readAsync:path done:done];

    [self answerReadThroughsWithDataUpToCount:3];
    [self waitForCompletions:done];

    XCTAssertEqual([self readThroughsSnapshot].count, 2, @"two reads of one path behind a held read-through take one more read-through, got batches %@", [self batchSizes]);
    [self assertOutcome:first carriesOnlyItsOwnValue:@"first waiter"];
    [self assertOutcome:second carriesOnlyItsOwnValue:@"second waiter"];
}

- (void)testRepeatedPathJoinsAFullBatchInsteadOfTakingAnotherReadThrough
{
    NSArray<MTRAttributePath *> * paths = [self pathsOnEndpointsFrom:2 count:9];
    __auto_type * done = [self expectationForCompletions:11];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];
    NSMutableArray<MTRAsyncReadBatchingOutcome *> * outcomes = [NSMutableArray array];
    for (MTRAttributePath * path in paths) {
        [outcomes addObject:[self readAsync:path done:done]];
    }
    MTRAsyncReadBatchingOutcome * repeated = [self readAsync:paths.lastObject done:done];

    [self answerReadThroughsWithDataUpToCount:3];
    [self waitForCompletions:done];

    XCTAssertEqualObjects([self batchSizes], (@[ @1, @9 ]), @"a path already in a full batch must not take another read-through");
    [self assertOutcome:outcomes.lastObject carriesOnlyItsOwnValue:@"first reader of the repeated path"];
    [self assertOutcome:repeated carriesOnlyItsOwnValue:@"second reader of the repeated path"];
}

#pragma mark - Sequencing

- (void)testHeldReadCompletesWithoutWaitingForTheReadThroughQueuedBehindIt
{
    __auto_type * heldDone = [self expectationForCompletions:1];
    __auto_type * queuedDone = [self expectationForCompletions:1];
    MTRAsyncReadBatchingOutcome * held = [self readAsync:[self pathOnEndpoint:1] done:heldDone];
    [self waitForReadThroughCount:1];
    MTRAsyncReadBatchingOutcome * queued = [self readAsync:[self pathOnEndpoint:2] done:queuedDone];

    [self answerReadThroughAtIndexWithData:0];
    [self waitForCompletions:heldDone];
    BOOL secondReadThroughStarted = [self waitForReadThroughCount:2];

    XCTAssertTrue(held.pathWasAnsweredBeforeCall, @"the held read completed before its read-through was answered");
    XCTAssertEqual(queued.callCount, 0, @"the queued read completed before the read-through it rides was answered");
    XCTAssertTrue(secondReadThroughStarted, @"the queued read's read-through did not start after the held one finished");
    [self assertOutcome:held carriesOnlyItsOwnValue:@"held read"];

    [self answerReadThroughAtIndexWithData:1];
    [self waitForCompletions:queuedDone];
    [self assertOutcome:queued carriesOnlyItsOwnValue:@"queued read"];
}

- (void)testBatchedReadsCompleteOnlyAfterTheReadThroughTheyRideIsAnswered
{
    __auto_type * done = [self expectationForCompletions:4];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];
    NSMutableArray<MTRAsyncReadBatchingOutcome *> * queued = [NSMutableArray array];
    for (MTRAttributePath * path in [self pathsOnEndpointsFrom:2 count:3]) {
        [queued addObject:[self readAsync:path done:done]];
    }

    [self answerReadThroughsWithDataUpToCount:3];
    [self waitForCompletions:done];

    for (MTRAsyncReadBatchingOutcome * outcome in queued) {
        XCTAssertEqual(outcome.callCount, 1, @"%@ completion ran %lu times", outcome.path, (unsigned long) outcome.callCount);
        XCTAssertTrue(outcome.pathWasAnsweredBeforeCall, @"%@ completed before the read-through it rides was answered", outcome.path);
    }
    XCTAssertEqualObjects([self batchSizes], (@[ @1, @3 ]));
}

- (void)testEachReadInAMergedBatchCompletesOnItsOwnCallersQueue
{
    const NSUInteger callerCount = 3;
    __auto_type * done = [self expectationForCompletions:callerCount + 1];
    [self readAsync:[self pathOnEndpoint:1] done:done];
    [self waitForReadThroughCount:1];

    NSMutableArray<dispatch_queue_t> * queues = [NSMutableArray array];
    NSMutableArray<MTRAsyncReadBatchingOutcome *> * queued = [NSMutableArray array];
    for (NSUInteger i = 0; i < callerCount; i++) {
        dispatch_queue_t queue = dispatch_queue_create("MTRDeviceAsyncReadBatchingTests.caller-n", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_set_specific(queue, &kCallerQueueKey, (void *) (intptr_t) (i + 1), NULL);
        [queues addObject:queue];
        [queued addObject:[self readAsync:[self pathOnEndpoint:2 + i] queue:queue done:done]];
    }

    [self answerReadThroughsWithDataUpToCount:3];
    [self waitForCompletions:done];
    for (dispatch_queue_t queue in queues) {
        dispatch_sync(queue, ^ {});
    }

    for (NSUInteger i = 0; i < callerCount; i++) {
        MTRAsyncReadBatchingOutcome * outcome = queued[i];
        XCTAssertEqual(outcome.callCount, 1, @"caller %lu completion ran %lu times", (unsigned long) i, (unsigned long) outcome.callCount);
        XCTAssertTrue(outcome.pathWasAnsweredBeforeCall, @"caller %lu completed before the read-through it rides was answered", (unsigned long) i);
        XCTAssertEqual(outcome.queueTag, (intptr_t) (i + 1), @"caller %lu completion ran on another caller's queue", (unsigned long) i);
    }
    XCTAssertEqualObjects([self batchSizes], (@[ @1, @3 ]));
}

#pragma mark - Synchronous read guards

- (MTRAsyncWorkCompletionBlock)holdDeviceWorkQueue
{
    __auto_type * blockerRunning = [self expectationWithDescription:@"blocking item running"];
    __block MTRAsyncWorkCompletionBlock releaseBlocker;
    __auto_type * blocker = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0)];
    blocker.readyHandler = ^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        releaseBlocker = completion;
        [blockerRunning fulfill];
    };
    [_device.asyncWorkQueue enqueueWorkItem:blocker description:@"blocking item"];
    [self waitForExpectations:@[ blockerRunning ] timeout:kWaitTimeout];
    return releaseBlocker;
}

- (void)syncRead:(MTRAttributePath *)path
{
    [_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
}

- (void)testSyncReadsQueuedBehindAHeldItemMergeIntoItemsOfAtMostNinePaths
{
    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    MTRAsyncWorkCompletionBlock releaseBlocker = [self holdDeviceWorkQueue];
    for (MTRAttributePath * path in [self pathsOnEndpointsFrom:1 count:11]) {
        [self syncRead:path];
    }
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 12);

    releaseBlocker(MTRAsyncWorkComplete);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 3, @"11 queued reads did not merge into one item of 9 paths plus 2 items");
}

- (void)testSyncReadOfAnAlreadyQueuedPathQueuesNothing
{
    MTRAsyncWorkCompletionBlock releaseBlocker = [self holdDeviceWorkQueue];
    MTRAttributePath * firstPath = [self pathOnEndpoint:1];
    MTRAttributePath * secondPath = [self pathOnEndpoint:2];
    [self syncRead:firstPath];
    [self syncRead:secondPath];
    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;

    [self syncRead:secondPath];
    [self syncRead:firstPath];
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
    (void) releaseBlocker;
}

@end
