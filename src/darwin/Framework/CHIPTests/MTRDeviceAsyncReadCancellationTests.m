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
static char kCallerQueueKey;

@interface MTRCancellationTestReadCall : NSObject
@property (nonatomic, readonly) MTRAttributePath * path;
@property (nonatomic, readonly) dispatch_queue_t queue;
@property (nonatomic, readonly) XCTestExpectation * completed;
@property (nonatomic, readonly) XCTestExpectation * completedAgain;
@property (atomic) BOOL mayComplete;
@property (atomic) NSUInteger callCount;
@property (atomic) BOOL calledOnItsQueue;
@property (atomic) BOOL completedEarly;
@property (atomic, nullable) NSArray<NSDictionary<NSString *, id> *> * values;
@property (atomic, nullable) NSError * error;
@end

@implementation MTRCancellationTestReadCall
- (instancetype)initWithPath:(MTRAttributePath *)path queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _path = path;
        _queue = queue;
        _completed = [[XCTestExpectation alloc] initWithDescription:[NSString stringWithFormat:@"read of %@ completed", path]];
        _completed.assertForOverFulfill = YES;
        _completedAgain = [[XCTestExpectation alloc] initWithDescription:[NSString stringWithFormat:@"read of %@ completed a second time", path]];
        _completedAgain.inverted = YES;
    }
    return self;
}
@end

@interface MTRCancellationTestReadThroughHolder : NSObject
@property (nonatomic, readonly) NSMutableArray<NSArray<MTRAttributeRequestPath *> *> * paths;
@property (nonatomic, readonly) NSMutableArray<MTRDeviceResponseHandler> * completions;
@property (nonatomic, readonly) NSArray<XCTestExpectation *> * started;
@end

@implementation MTRCancellationTestReadThroughHolder
- (instancetype)init
{
    if (self = [super init]) {
        _paths = [NSMutableArray array];
        _completions = [NSMutableArray array];
        NSMutableArray<XCTestExpectation *> * started = [NSMutableArray array];
        for (NSUInteger i = 0; i < 4; i++) {
            [started addObject:[[XCTestExpectation alloc] initWithDescription:[NSString stringWithFormat:@"read-through %lu held", (unsigned long) i]]];
        }
        _started = started;
    }
    return self;
}

- (NSUInteger)count
{
    @synchronized(self) {
        return _completions.count;
    }
}

- (MTRDeviceResponseHandler)takeCompletionAtIndex:(NSUInteger)index
{
    @synchronized(self) {
        MTRDeviceResponseHandler completion = _completions[index];
        _completions[index] = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        };
        return completion;
    }
}

- (void)dropCompletions
{
    @synchronized(self) {
        [_completions removeAllObjects];
    }
}
@end

@interface MTRDeviceAsyncReadCancellationTests : MTRTestCase
@end

@implementation MTRDeviceAsyncReadCancellationTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRDeviceTestDelegate * _delegate;
    MTRCancellationTestReadThroughHolder * _readThroughs;
    BOOL _localTestStorageEnabledBeforeTest;
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

    // No device has this node ID, and an MTRDevice without a delegate does not
    // subscribe, so its cache holds only what the tests inject.
    _device = [MTRDevice deviceWithNodeID:@(1) controller:_controller];
}

- (void)tearDown
{
    [_controller shutdown];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

#pragma mark - Helpers

- (MTRAttributePath *)onOffPathOnEndpoint:(NSUInteger)endpoint
{
    return [MTRAttributePath attributePathWithEndpointID:@(endpoint)
                                               clusterID:@(MTRClusterIDTypeOnOffID)
                                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)];
}

- (NSArray<NSDictionary<NSString *, id> *> *)onOffValuesForPaths:(NSArray<MTRAttributeRequestPath *> *)paths
{
    NSMutableArray<NSDictionary<NSString *, id> *> * values = [NSMutableArray array];
    for (MTRAttributeRequestPath * path in paths) {
        [values addObject:@{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute],
            MTRDataKey : @ {
                MTRTypeKey : MTRBooleanValueType,
                MTRValueKey : @(YES),
            },
        }];
    }
    return values;
}

- (dispatch_queue_t)callerQueueNamed:(NSString *)name
{
    dispatch_queue_t queue = dispatch_queue_create(name.UTF8String, DISPATCH_QUEUE_SERIAL);
    dispatch_queue_set_specific(queue, &kCallerQueueKey, (__bridge void *) queue, NULL);
    return queue;
}

- (void)holdReadThroughsOfDevice:(MTRDevice *)device
{
    MTRCancellationTestReadThroughHolder * readThroughs = [[MTRCancellationTestReadThroughHolder alloc] init];
    _readThroughs = readThroughs;

    MTRDeviceTestDelegateWithSubscriptionSetupOverride * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    delegate.onReadThrough = ^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        NSUInteger index;
        @synchronized(readThroughs) {
            [readThroughs.paths addObject:paths];
            [readThroughs.completions addObject:completion];
            index = readThroughs.completions.count - 1;
        }
        if (index < readThroughs.started.count) {
            [readThroughs.started[index] fulfill];
        }
    };
    _delegate = delegate;
    [device addDelegate:delegate queue:dispatch_get_main_queue()];
}

- (MTRCancellationTestReadCall *)readPath:(MTRAttributePath *)path ofDevice:(MTRDevice *)device onQueue:(dispatch_queue_t)queue
{
    MTRCancellationTestReadCall * call = [[MTRCancellationTestReadCall alloc] initWithPath:path queue:queue];
    [device readAttributeWithEndpointID:path.endpoint
                              clusterID:path.cluster
                            attributeID:path.attribute
                                 params:nil
                                  queue:queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 NSUInteger callCount;
                                 @synchronized(call) {
                                     callCount = ++call.callCount;
                                 }
                                 if (callCount > 1) {
                                     [call.completedAgain fulfill];
                                     return;
                                 }
                                 call.completedEarly = !call.mayComplete;
                                 call.calledOnItsQueue = dispatch_get_specific(&kCallerQueueKey) == (__bridge void *) call.queue;
                                 call.values = values;
                                 call.error = error;
                                 [call.completed fulfill];
                             }];
    return call;
}

- (void)waitForReadThrough:(NSUInteger)index heldFor:(MTRCancellationTestReadCall *)call
{
    XCTWaiterResult result = [XCTWaiter waitForExpectations:@[ _readThroughs.started[index] ] timeout:5];
    XCTAssertEqual(result, XCTWaiterResultCompleted, @"read-through %lu was never handed to the test while the read of %@ waited; that read completed %lu time(s), error %@",
        (unsigned long) index, call.path, (unsigned long) call.callCount, call.error);
}

- (void)allowToComplete:(NSArray<MTRCancellationTestReadCall *> *)calls
{
    for (MTRCancellationTestReadCall * call in calls) {
        call.mayComplete = YES;
    }
}

- (void)waitForCompletionOf:(NSArray<MTRCancellationTestReadCall *> *)calls
{
    [self waitForExpectations:[calls valueForKey:@"completed"] timeout:5];
    for (MTRCancellationTestReadCall * call in calls) {
        XCTAssertFalse(call.completedEarly, @"read of %@ completed before it was cancelled, released or answered: error %@", call.path, call.error);
    }
}

- (void)assertNoSecondCompletionOf:(NSArray<MTRCancellationTestReadCall *> *)calls device:(MTRDevice * _Nullable)device
{
    for (NSUInteger pass = 0; pass < 2; pass++) {
        [device unitTestSyncRunOnDeviceQueue:^ {}];
        for (MTRCancellationTestReadCall * call in calls) {
            dispatch_sync(call.queue, ^ {});
        }
    }
    [self waitForExpectations:[calls valueForKey:@"completedAgain"] timeout:0.5];
}

- (void)assertCancelledOnItsQueue:(MTRCancellationTestReadCall *)call
{
    XCTAssertEqual(call.callCount, 1, @"read of %@ was not called back exactly once", call.path);
    XCTAssertEqualObjects(call.error.domain, MTRErrorDomain, @"read of %@: %@", call.path, call.error);
    XCTAssertEqual(call.error.code, MTRErrorCodeCancelled, @"read of %@: %@", call.path, call.error);
    XCTAssertTrue(call.calledOnItsQueue, @"read of %@ was not called back on its own queue", call.path);
}

#pragma mark - Invalidation and shutdown

- (void)testInvalidateCancelsEveryWaiterOfAMergedReadThroughAndTheItemBehindItOnce
{
    [self holdReadThroughsOfDevice:_device];
    NSArray<dispatch_queue_t> * queues = @[ [self callerQueueNamed:@"caller.0"], [self callerQueueNamed:@"caller.1"], [self callerQueueNamed:@"caller.2"] ];

    MTRCancellationTestReadCall * first = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:queues[0]];
    [self waitForReadThrough:0 heldFor:first];

    NSMutableArray<MTRCancellationTestReadCall *> * waiting = [NSMutableArray array];
    for (NSUInteger endpoint = 2; endpoint <= 11; endpoint++) {
        [waiting addObject:[self readPath:[self onOffPathOnEndpoint:endpoint] ofDevice:_device onQueue:queues[endpoint % queues.count]]];
    }

    [self allowToComplete:@[ first ]];
    [_readThroughs takeCompletionAtIndex:0]([self onOffValuesForPaths:_readThroughs.paths[0]], nil);
    [self waitForCompletionOf:@[ first ]];
    XCTAssertNil(first.error);

    [self waitForReadThrough:1 heldFor:waiting.firstObject];
    XCTAssertEqual(_readThroughs.paths[1].count, 9);

    [self allowToComplete:waiting];
    [_device invalidate];
    [self waitForCompletionOf:waiting];

    [self assertNoSecondCompletionOf:[waiting arrayByAddingObject:first] device:_device];
    for (MTRCancellationTestReadCall * call in waiting) {
        [self assertCancelledOnItsQueue:call];
    }
    XCTAssertEqual(first.callCount, 1);
    XCTAssertEqual(_readThroughs.count, 2);
}

- (void)testControllerShutdownCancelsTheHeldWaiterAndTheQueuedWaitersOnce
{
    [self holdReadThroughsOfDevice:_device];

    MTRCancellationTestReadCall * held = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.held"]];
    [self waitForReadThrough:0 heldFor:held];
    MTRCancellationTestReadCall * queued = [self readPath:[self onOffPathOnEndpoint:2] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.queued"]];
    MTRCancellationTestReadCall * queuedSecond = [self readPath:[self onOffPathOnEndpoint:3] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.queued.2"]];
    NSArray<MTRCancellationTestReadCall *> * calls = @[ held, queued, queuedSecond ];

    [self allowToComplete:calls];
    [_controller shutdown];
    [self waitForCompletionOf:calls];

    [self assertNoSecondCompletionOf:calls device:_device];
    for (MTRCancellationTestReadCall * call in calls) {
        [self assertCancelledOnItsQueue:call];
    }
}

- (void)testInvalidateBeforeALateReadThroughResultCallsBackOnceWithCancelled
{
    [self holdReadThroughsOfDevice:_device];

    MTRCancellationTestReadCall * call = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller"]];
    [self waitForReadThrough:0 heldFor:call];

    [self allowToComplete:@[ call ]];
    [_device invalidate];
    [_readThroughs takeCompletionAtIndex:0]([self onOffValuesForPaths:_readThroughs.paths[0]], nil);
    [self waitForCompletionOf:@[ call ]];

    [self assertNoSecondCompletionOf:@[ call ] device:_device];
    [self assertCancelledOnItsQueue:call];
}

- (void)testLateFailureAfterInvalidateIsNotRetriedAndCallsBackOnceWithCancelled
{
    [self holdReadThroughsOfDevice:_device];

    MTRCancellationTestReadCall * call = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller"]];
    [self waitForReadThrough:0 heldFor:call];

    [self allowToComplete:@[ call ]];
    [_device invalidate];
    [_readThroughs takeCompletionAtIndex:0](nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil]);
    [self waitForCompletionOf:@[ call ]];

    [self assertNoSecondCompletionOf:@[ call ] device:_device];
    [self assertCancelledOnItsQueue:call];
    XCTAssertEqual(_readThroughs.count, 1);
}

- (void)testLateResultRacingInvalidateCallsBackOnceWithEitherTheValueOrCancelled
{
    [self holdReadThroughsOfDevice:_device];

    MTRCancellationTestReadCall * call = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller"]];
    [self waitForReadThrough:0 heldFor:call];

    [self allowToComplete:@[ call ]];
    [_readThroughs takeCompletionAtIndex:0]([self onOffValuesForPaths:_readThroughs.paths[0]], nil);
    [_device invalidate];
    [self waitForCompletionOf:@[ call ]];

    [self assertNoSecondCompletionOf:@[ call ] device:_device];
    XCTAssertEqual(call.callCount, 1);
    XCTAssertTrue(call.calledOnItsQueue);
    if (call.error == nil) {
        XCTAssertEqual(call.values.count, 1);
        XCTAssertEqualObjects(call.values.firstObject[MTRAttributePathKey], call.path);
        XCTAssertNotNil(call.values.firstObject[MTRDataKey], @"%@", call.values);
    } else {
        XCTAssertEqualObjects(call.error.domain, MTRErrorDomain, @"%@", call.error);
        XCTAssertEqual(call.error.code, MTRErrorCodeCancelled, @"%@", call.error);
        XCTAssertNil(call.values);
    }
}

- (void)testAsyncReadIssuedAfterInvalidateWhileAReadThroughIsHeldCallsBackOnceWithCancelled
{
    [self holdReadThroughsOfDevice:_device];

    MTRCancellationTestReadCall * held = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.held"]];
    [self waitForReadThrough:0 heldFor:held];

    [self allowToComplete:@[ held ]];
    [_device invalidate];
    MTRCancellationTestReadCall * late = [self readPath:[self onOffPathOnEndpoint:2] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.late"]];
    [self allowToComplete:@[ late ]];
    [self waitForCompletionOf:@[ held, late ]];

    [self assertNoSecondCompletionOf:@[ held, late ] device:_device];
    [self assertCancelledOnItsQueue:held];
    [self assertCancelledOnItsQueue:late];
    XCTAssertEqual(_readThroughs.count, 1);
}

- (void)testAsyncReadIssuedAfterInvalidateWithNothingRunningCallsBackOnceWithCancelledWithoutAReadThrough
{
    [self holdReadThroughsOfDevice:_device];

    [_device invalidate];
    MTRCancellationTestReadCall * late = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:_device onQueue:[self callerQueueNamed:@"caller.late"]];
    [self allowToComplete:@[ late ]];
    [self waitForCompletionOf:@[ late ]];

    [self assertNoSecondCompletionOf:@[ late ] device:_device];
    [self assertCancelledOnItsQueue:late];
    XCTAssertEqual(_readThroughs.count, 0);
}

#pragma mark - Releasing the device

- (void)testReleasingDeviceWhileReadThroughIsHeldCancelsWaiterOnceWhenTheResultArrivesLate
{
    __weak MTRDevice * weakDevice;
    MTRCancellationTestReadCall * call;
    XCTestExpectation * deviceDeallocated = [[XCTestExpectation alloc] initWithDescription:@"device deallocated"];
    @autoreleasepool {
        MTRDevice * device = [MTRDevice deviceWithNodeID:@(2) controller:_controller];
        weakDevice = device;
        [self holdReadThroughsOfDevice:device];
        _delegate.onSubscriptionCallbackDelete = ^{
            [deviceDeallocated fulfill];
        };

        call = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:device onQueue:[self callerQueueNamed:@"caller"]];
        [self waitForReadThrough:0 heldFor:call];
        [self allowToComplete:@[ call ]];
    }
    [self waitForExpectations:@[ deviceDeallocated ] timeout:5];
    XCTAssertNil(weakDevice);

    @autoreleasepool {
        [_readThroughs takeCompletionAtIndex:0]([self onOffValuesForPaths:_readThroughs.paths[0]], nil);
        [_readThroughs dropCompletions];
    }
    [self waitForCompletionOf:@[ call ]];

    [self assertNoSecondCompletionOf:@[ call ] device:nil];
    [self assertCancelledOnItsQueue:call];
}

- (void)testReleasingDeviceWithReadsQueuedCancelsEachOnItsOwnQueueOnce
{
    __weak MTRDevice * weakDevice;
    NSArray<MTRCancellationTestReadCall *> * calls;
    @autoreleasepool {
        MTRDevice * device = [MTRDevice deviceWithNodeID:@(2) controller:_controller];
        weakDevice = device;
        [self holdReadThroughsOfDevice:device];

        MTRCancellationTestReadCall * held = [self readPath:[self onOffPathOnEndpoint:1] ofDevice:device onQueue:[self callerQueueNamed:@"caller.held"]];
        [self waitForReadThrough:0 heldFor:held];
        calls = @[
            held,
            [self readPath:[self onOffPathOnEndpoint:2] ofDevice:device onQueue:[self callerQueueNamed:@"caller.queued.0"]],
            [self readPath:[self onOffPathOnEndpoint:3] ofDevice:device onQueue:[self callerQueueNamed:@"caller.queued.1"]],
        ];
        [self allowToComplete:calls];
    }
    // The held read-through never answers; dropping it leaves nothing but the callers' waits.
    @autoreleasepool {
        [_readThroughs dropCompletions];
    }
    [self waitForCompletionOf:calls];
    XCTAssertNil(weakDevice);

    [self assertNoSecondCompletionOf:calls device:nil];
    for (MTRCancellationTestReadCall * call in calls) {
        [self assertCancelledOnItsQueue:call];
    }
}

#pragma mark - Existing behaviour the tests above rest on

- (void)testDeviceWithSyncReadThroughQueuedIsDeallocatedWhenReleased
{
    __weak MTRDevice * weakDevice;
    XCTestExpectation * deviceDeallocated = [[XCTestExpectation alloc] initWithDescription:@"device deallocated"];
    @autoreleasepool {
        MTRDevice * device = [MTRDevice deviceWithNodeID:@(3) controller:_controller];
        weakDevice = device;
        MTRDeviceTestDelegateWithSubscriptionSetupOverride * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
        delegate.skipSetupSubscription = YES;
        delegate.onSubscriptionCallbackDelete = ^{
            [deviceDeallocated fulfill];
        };
        _delegate = delegate;
        [device addDelegate:delegate queue:dispatch_get_main_queue()];

        for (NSUInteger endpoint = 1; endpoint <= 2; endpoint++) {
            MTRAttributePath * path = [self onOffPathOnEndpoint:endpoint];
            [device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
        }
        XCTAssertEqual(device.asyncWorkQueue.itemCount, 2);
    }
    [self waitForExpectations:@[ deviceDeallocated ] timeout:5];
    XCTAssertNil(weakDevice);
}

- (void)testControllerShutdownEmptiesTheWorkQueueOfADeviceWithSyncReadThroughsQueued
{
    for (NSUInteger endpoint = 1; endpoint <= 2; endpoint++) {
        MTRAttributePath * path = [self onOffPathOnEndpoint:endpoint];
        [_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
    }
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, 2);

    [_controller shutdown];
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, 0);
}

@end
