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
static char kCallbackQueueKey;

@interface MTRAsyncReadRecord : NSObject
@property (atomic) NSUInteger callCount;
@property (atomic) BOOL everyCallOnGivenQueue;
@property (atomic, copy) NSArray<NSDictionary<NSString *, id> *> * values;
@property (atomic) NSError * error;
@property (atomic) NSUInteger readThroughsAtCompletion;
@end

@implementation MTRAsyncReadRecord
- (instancetype)init
{
    if (self = [super init]) {
        _everyCallOnGivenQueue = YES;
    }
    return self;
}
@end

@interface MTRDeviceAsyncReadOutcomeTests : MTRTestCase
@end

@implementation MTRDeviceAsyncReadOutcomeTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRDeviceTestDelegateWithSubscriptionSetupOverride * _delegate;
    dispatch_queue_t _callbackQueue;
    NSMutableArray<NSArray<MTRAttributeRequestPath *> *> * _readThroughs;
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

    _device = [MTRDevice deviceWithNodeID:@(1) controller:_controller];

    _callbackQueue = dispatch_queue_create("MTRDeviceAsyncReadOutcomeTests.callbackQueue", DISPATCH_QUEUE_SERIAL);
    dispatch_queue_set_specific(_callbackQueue, &kCallbackQueueKey, &kCallbackQueueKey, NULL);
    _readThroughs = [NSMutableArray array];
}

- (void)tearDown
{
    [_controller shutdown];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeTest;
    [super tearDown];
}

#pragma mark - Helpers

- (MTRAttributePath *)onOffPath
{
    return [MTRAttributePath attributePathWithEndpointID:@(1)
                                               clusterID:@(MTRClusterIDTypeOnOffID)
                                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)];
}

- (MTRAttributePath *)currentHeapUsedPath
{
    return [MTRAttributePath attributePathWithEndpointID:@(0)
                                               clusterID:@(MTRClusterIDTypeSoftwareDiagnosticsID)
                                             attributeID:@(MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID)];
}

- (MTRAttributeRequestPath *)requestPathFor:(MTRAttributePath *)path
{
    return [MTRAttributeRequestPath requestPathWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute];
}

- (NSDictionary<NSString *, id> *)booleanValue:(BOOL)value
{
    return @{ MTRTypeKey : MTRBooleanValueType, MTRValueKey : @(value) };
}

- (void)useDelegateWithSubscriptionAbleToReport:(BOOL)ableToReport
{
    _delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    _delegate.skipSetupSubscription = YES;
    _delegate.subscriptionAbleToReportOverride = @(ableToReport);
    [_device addDelegate:_delegate queue:dispatch_get_main_queue()];
}

- (void)inject:(NSDictionary<NSString *, id> *)dataValue forPath:(MTRAttributePath *)path
{
    [_device unitTestInjectAttributeReport:@[ @{ MTRAttributePathKey : path, MTRDataKey : dataValue } ] fromSubscription:YES];
    [_controller syncRunOnWorkQueue:^{
    } error:nil];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
}

- (void)expect:(NSDictionary<NSString *, id> *)dataValue forPath:(MTRAttributePath *)path
{
    uint64_t expectedValueID = 0;
    [_device setExpectedValues:@[ @{ MTRAttributePathKey : path, MTRDataKey : dataValue } ]
         expectedValueInterval:@(60 * 1000)
               expectedValueID:&expectedValueID];
}

- (NSDictionary<NSString *, id> *)syncRead:(MTRAttributePath *)path
{
    return [_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
}

- (NSUInteger)readThroughCount
{
    @synchronized(_readThroughs) {
        return _readThroughs.count;
    }
}

- (NSArray<MTRAttributeRequestPath *> *)readThroughPathsAtIndex:(NSUInteger)index
{
    @synchronized(_readThroughs) {
        return _readThroughs[index];
    }
}

- (void)answerReadThroughsWith:(void (^)(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion))answer
{
    NSMutableArray * readThroughs = _readThroughs;
    _delegate.onReadThrough = ^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        @synchronized(readThroughs) {
            [readThroughs addObject:paths];
        }
        answer(paths, completion);
    };
}

- (void)answerReadThroughsLaterWithValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)values error:(NSError * _Nullable)error
{
    [self answerReadThroughsWith:^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        dispatch_async(dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
            completion(values, error);
        });
    }];
}

- (MTRAsyncReadRecord *)readAsync:(MTRAttributePath *)path params:(MTRReadParams * _Nullable)params fulfilling:(XCTestExpectation *)completed
{
    completed.assertForOverFulfill = YES;
    MTRAsyncReadRecord * record = [[MTRAsyncReadRecord alloc] init];
    __weak typeof(self) weakSelf = self;
    [_device readAttributeWithEndpointID:path.endpoint
                               clusterID:path.cluster
                             attributeID:path.attribute
                                  params:params
                                   queue:_callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  record.callCount = record.callCount + 1;
                                  if (dispatch_get_specific(&kCallbackQueueKey) != &kCallbackQueueKey) {
                                      record.everyCallOnGivenQueue = NO;
                                  }
                                  record.values = values;
                                  record.error = error;
                                  record.readThroughsAtCompletion = [weakSelf readThroughCount];
                                  [completed fulfill];
                              }];
    return record;
}

- (void)assertCalledOnceOnGivenQueue:(MTRAsyncReadRecord *)record
{
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    dispatch_sync(_callbackQueue, ^ {});
    XCTAssertEqual(record.callCount, 1, @"completion must run exactly once");
    XCTAssertTrue(record.everyCallOnGivenQueue, @"completion must run on the queue the caller gave");
}

- (void)assertRecord:(MTRAsyncReadRecord *)record hasValue:(NSDictionary<NSString *, id> *)dataValue forPath:(MTRAttributePath *)path
{
    XCTAssertNil(record.error);
    XCTAssertEqual(record.values.count, 1);
    XCTAssertEqualObjects(record.values.firstObject[MTRAttributePathKey], path);
    XCTAssertEqualObjects(record.values.firstObject[MTRDataKey], dataValue);
    XCTAssertNil(record.values.firstObject[MTRErrorKey]);
}

#pragma mark - No read-through needed

- (void)testCachedValueCompletesOnceWithSyncReadValueAndNoReadThrough
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    [self answerReadThroughsLaterWithValues:@[] error:nil];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    [self assertRecord:record hasValue:[self syncRead:[self onOffPath]] forPath:[self onOffPath]];
    XCTAssertEqualObjects(record.values.firstObject[MTRDataKey], [self booleanValue:YES]);
    XCTAssertEqual([self readThroughCount], 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
}

- (void)testPendingExpectedValueWinsOverCachedValueWithoutReadThrough
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    [self answerReadThroughsLaterWithValues:@[] error:nil];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];
    [self expect:[self booleanValue:NO] forPath:[self onOffPath]];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    [self assertRecord:record hasValue:[self booleanValue:NO] forPath:[self onOffPath]];
    XCTAssertEqualObjects(record.values.firstObject[MTRDataKey], [self syncRead:[self onOffPath]]);
    XCTAssertEqual([self readThroughCount], 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
}

#pragma mark - Read-through needed

- (void)testFetchedValueArrivesOnlyAfterReadThroughCompletes
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];

    XCTestExpectation * readThroughStarted = [self expectationWithDescription:@"read-through started"];
    __block MTRDeviceResponseHandler heldCompletion;
    [self answerReadThroughsWith:^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        heldCompletion = completion;
        [readThroughStarted fulfill];
    }];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ readThroughStarted ] timeout:5];

    [_device unitTestSyncRunOnDeviceQueue:^ {}];
    dispatch_sync(_callbackQueue, ^ {});
    XCTAssertEqual(record.callCount, 0, @"completion ran while its read-through was still outstanding");
    XCTAssertTrue([[self readThroughPathsAtIndex:0] containsObject:[self requestPathFor:[self onOffPath]]]);

    heldCompletion(@[ @{ MTRAttributePathKey : [self onOffPath], MTRDataKey : [self booleanValue:NO] } ], nil);
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    [self assertRecord:record hasValue:[self booleanValue:NO] forPath:[self onOffPath]];
    XCTAssertEqual([self readThroughCount], 1);
}

- (void)testFetchWithExpectedValuePendingCompletesWithExpectedValue
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self expect:[self booleanValue:YES] forPath:[self onOffPath]];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : [self onOffPath], MTRDataKey : [self booleanValue:NO] } ] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1, @"completion must come after the read-through");
    [self assertRecord:record hasValue:[self booleanValue:YES] forPath:[self onOffPath]];
}

- (void)testChangesOmittedAttributeIsFetchedEvenWhenSubscriptionCanReport
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    NSDictionary * cached = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(1000) };
    NSDictionary * fetched = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(2000) };
    [self inject:cached forPath:[self currentHeapUsedPath]];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : [self currentHeapUsedPath], MTRDataKey : fetched } ] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self currentHeapUsedPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1);
    [self assertRecord:record hasValue:fetched forPath:[self currentHeapUsedPath]];
}

- (void)testUnknownAttributeNotAssumedReportableIsFetched
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    MTRAttributePath * vendorPath = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(0xFFF1FC99) attributeID:@(0x0001)];
    [self inject:[self booleanValue:YES] forPath:vendorPath];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : vendorPath, MTRDataKey : [self booleanValue:NO] } ] error:nil];

    MTRReadParams * params = [[MTRReadParams alloc] init];
    params.assumeUnknownAttributesReportable = NO;
    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:vendorPath params:params fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1);
    [self assertRecord:record hasValue:[self booleanValue:NO] forPath:vendorPath];
}

- (void)testEmptyCacheWithSubscriptionAbleToReportFetchesAndCompletesWithFetchedValue
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : [self onOffPath], MTRDataKey : [self booleanValue:YES] } ] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1, @"an empty cache must be filled by a read-through before completing");
    [self assertRecord:record hasValue:[self booleanValue:YES] forPath:[self onOffPath]];
}

#pragma mark - Read-through outcomes

- (void)testPerPathErrorCompletesWithErrorEntryAndNoTopLevelError
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];
    NSError * pathError = [NSError errorWithDomain:MTRInteractionErrorDomain code:MTRInteractionErrorCodeUnsupportedAttribute userInfo:nil];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : [self onOffPath], MTRErrorKey : pathError } ] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertNil(record.error, @"a per-path error belongs in values, not in error");
    XCTAssertEqual(record.values.count, 1);
    XCTAssertEqualObjects(record.values.firstObject[MTRAttributePathKey], [self onOffPath]);
    XCTAssertNil(record.values.firstObject[MTRDataKey], @"a per-path error must not come with a data value");
    NSError * reportedError = record.values.firstObject[MTRErrorKey];
    XCTAssertEqualObjects(reportedError.domain, MTRInteractionErrorDomain);
    XCTAssertEqual(reportedError.code, MTRInteractionErrorCodeUnsupportedAttribute);
    XCTAssertEqual([self readThroughCount], 1);
}

- (void)testWholeFetchFailureCompletesWithErrorOnlyAfterThirdTry
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];
    NSError * fetchError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeTimeout userInfo:nil];
    [self answerReadThroughsLaterWithValues:nil error:fetchError];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 3, @"completion must wait for the first try and both retries");
    XCTAssertEqual([self readThroughCount], 3);
    XCTAssertNil(record.values, @"a failed read must not return the cached value");
    XCTAssertEqualObjects(record.error.domain, MTRErrorDomain);
    XCTAssertEqual(record.error.code, MTRErrorCodeTimeout);
}

- (void)testNoEntryForPathWithoutAnyValueCompletesWithSchemaMismatch
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self answerReadThroughsLaterWithValues:@[] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1);
    XCTAssertNil(record.values);
    XCTAssertEqualObjects(record.error.domain, MTRErrorDomain);
    XCTAssertEqual(record.error.code, MTRErrorCodeSchemaMismatch);
}

- (void)testNoEntryForPathWithCachedValueCompletesWithCachedValue
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];
    MTRAttributePath * otherPath = [MTRAttributePath attributePathWithEndpointID:@(1)
                                                                       clusterID:@(MTRClusterIDTypeOnOffID)
                                                                     attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID)];
    [self answerReadThroughsLaterWithValues:@[ @{ MTRAttributePathKey : otherPath, MTRDataKey : [self booleanValue:NO] } ] error:nil];

    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertEqual(record.readThroughsAtCompletion, 1);
    [self assertRecord:record hasValue:[self booleanValue:YES] forPath:[self onOffPath]];
}

#pragma mark - Suspended

- (void)testSuspendedWithEmptyCacheFailsWithInvalidStateAndNoReadThrough
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self answerReadThroughsLaterWithValues:@[] error:nil];
    [_controller suspend];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    XCTestExpectation * completed = [self expectationWithDescription:@"read completed"];
    MTRAsyncReadRecord * record = [self readAsync:[self onOffPath] params:nil fulfilling:completed];
    [self waitForExpectations:@[ completed ] timeout:5];
    [self assertCalledOnceOnGivenQueue:record];

    XCTAssertNil(record.values);
    XCTAssertEqualObjects(record.error.domain, MTRErrorDomain);
    XCTAssertEqual(record.error.code, MTRErrorCodeInvalidState);
    XCTAssertEqual([self readThroughCount], 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
}

#pragma mark - Synchronous read guards

- (void)testSyncReadReturnsPendingExpectedValueOverCachedValue
{
    [self useDelegateWithSubscriptionAbleToReport:YES];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];
    XCTAssertEqualObjects([self syncRead:[self onOffPath]], [self booleanValue:YES]);

    [self expect:[self booleanValue:NO] forPath:[self onOffPath]];
    XCTAssertEqualObjects([self syncRead:[self onOffPath]], [self booleanValue:NO]);
}

- (void)testSyncReadNeedingReadThroughReturnsCachedValueAndQueuesOneRead
{
    [self useDelegateWithSubscriptionAbleToReport:NO];
    [self inject:[self booleanValue:YES] forPath:[self onOffPath]];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    XCTAssertEqualObjects([self syncRead:[self onOffPath]], [self booleanValue:YES]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 1);

    XCTAssertEqualObjects([self syncRead:[self onOffPath]], [self booleanValue:YES]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 1, @"a second read of the same path must join the queued read");
}

@end
