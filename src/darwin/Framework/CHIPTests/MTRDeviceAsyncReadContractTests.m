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
static const NSTimeInterval kReadTimeout = 5;
static const uint8_t kFabricIndexFieldTag = 0xFE;
static char kCompletionQueueKey;

@interface MTRAsyncReadContractResult : NSObject
@property (nonatomic, copy, nullable) NSArray<NSDictionary<NSString *, id> *> * values;
@property (nonatomic, nullable) NSError * error;
@property (nonatomic) BOOL ranOnGivenQueue;
@property (nonatomic) BOOL callHadReturned;
@end

@implementation MTRAsyncReadContractResult
@end

@interface MTRDeviceAsyncReadContractTests : MTRTestCase
@end

@implementation MTRDeviceAsyncReadContractTests {
    MTRDeviceController * _controller;
    MTRDevice * _device;
    MTRDeviceTestDelegate * _delegate;
    BOOL _localTestStorageEnabledBeforeTest;
    dispatch_queue_t _completionQueue;
    NSMutableArray<NSArray<MTRAttributeRequestPath *> *> * _readThroughs;
    NSDictionary<MTRAttributePath *, NSDictionary<NSString *, id> *> * _readThroughAnswers;
    dispatch_queue_t _readThroughAnswerQueue;
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

    _completionQueue = dispatch_queue_create("MTRDeviceAsyncReadContractTests.completion", DISPATCH_QUEUE_SERIAL);
    dispatch_queue_set_specific(_completionQueue, &kCompletionQueueKey, &kCompletionQueueKey, NULL);
    _readThroughs = [NSMutableArray array];
    _readThroughAnswers = @{};
    _readThroughAnswerQueue = dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
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

- (MTRAttributePath *)onTimePath
{
    return [MTRAttributePath attributePathWithEndpointID:@(1)
                                               clusterID:@(MTRClusterIDTypeOnOffID)
                                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)];
}

- (MTRAttributePath *)vendorAttributePath
{
    return [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(0xFFF1FC99) attributeID:@(0x0001)];
}

- (MTRAttributePath *)utcTimePath
{
    return [MTRAttributePath attributePathWithEndpointID:@(0)
                                               clusterID:@(MTRClusterIDTypeTimeSynchronizationID)
                                             attributeID:@(MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID)];
}

- (NSDictionary<NSString *, id> *)booleanValue:(BOOL)value
{
    return @{ MTRTypeKey : MTRBooleanValueType, MTRValueKey : @(value) };
}

- (NSDictionary<NSString *, id> *)unsignedValue:(unsigned long long)value
{
    return @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(value) };
}

- (MTRReadParams *)paramsAssumingUnknownAttributesReportable:(BOOL)reportable
{
    MTRReadParams * params = [[MTRReadParams alloc] init];
    params.assumeUnknownAttributesReportable = reportable;
    return params;
}

- (MTRReadParams *)paramsFilteringByFabric:(BOOL)filterByFabric
{
    MTRReadParams * params = [[MTRReadParams alloc] init];
    params.filterByFabric = filterByFabric;
    return params;
}

- (void)injectValue:(NSDictionary<NSString *, id> *)value forPath:(MTRAttributePath *)path fromSubscription:(BOOL)fromSubscription
{
    [_device unitTestInjectAttributeReport:@[ @{ MTRAttributePathKey : path, MTRDataKey : value } ] fromSubscription:fromSubscription];
    [_controller syncRunOnWorkQueue:^{
    } error:nil];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];
}

- (void)addReadThroughDelegateWithSubscriptionAbleToReport:(BOOL)ableToReport
{
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    delegate.subscriptionAbleToReportOverride = @(ableToReport);

    __weak __typeof__(self) weakSelf = self;
    delegate.onReadThrough = ^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        __typeof__(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        NSMutableArray<NSDictionary<NSString *, id> *> * values = [NSMutableArray array];
        @synchronized(strongSelf->_readThroughs) {
            [strongSelf->_readThroughs addObject:[paths copy]];
            for (MTRAttributeRequestPath * requestPath in paths) {
                MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:requestPath.endpoint
                                                                              clusterID:requestPath.cluster
                                                                            attributeID:requestPath.attribute];
                NSDictionary<NSString *, id> * answer = strongSelf->_readThroughAnswers[path];
                if (answer != nil) {
                    [values addObject:@{ MTRAttributePathKey : path, MTRDataKey : answer }];
                }
            }
        }
        dispatch_async(strongSelf->_readThroughAnswerQueue, ^{
            completion(values, nil);
        });
    };

    _delegate = delegate;
    [_device addDelegate:_delegate queue:dispatch_get_main_queue()];
}

- (NSArray<NSArray<MTRAttributeRequestPath *> *> *)readThroughs
{
    @synchronized(_readThroughs) {
        return [_readThroughs copy];
    }
}

- (MTRAttributeRequestPath *)requestPathForPath:(MTRAttributePath *)path
{
    return [MTRAttributeRequestPath requestPathWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute];
}

- (MTRAsyncReadContractResult *)readAsync:(MTRAttributePath *)path params:(MTRReadParams * _Nullable)params
{
    __auto_type * result = [[MTRAsyncReadContractResult alloc] init];
    XCTestExpectation * completed = [self expectationWithDescription:[NSString stringWithFormat:@"async read of %@ completed", path]];
    completed.assertForOverFulfill = YES;

    MTRDevice * device = _device;
    dispatch_queue_t queue = _completionQueue;
    __block BOOL callReturned = NO;
    // Issued from the completion queue itself, so an inline call sees callReturned still NO.
    dispatch_async(queue, ^{
        [device readAttributeWithEndpointID:path.endpoint
                                  clusterID:path.cluster
                                attributeID:path.attribute
                                     params:params
                                      queue:queue
                                 completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                     result.values = values;
                                     result.error = error;
                                     result.ranOnGivenQueue = dispatch_get_specific(&kCompletionQueueKey) == &kCompletionQueueKey;
                                     result.callHadReturned = callReturned;
                                     [completed fulfill];
                                 }];
        callReturned = YES;
    });
    [self waitForExpectations:@[ completed ] timeout:kReadTimeout];
    return result;
}

- (void)assertDeliveredAsynchronouslyOnGivenQueue:(MTRAsyncReadContractResult *)result
{
    XCTAssertTrue(result.ranOnGivenQueue, @"completion did not run on the queue passed to the read");
    XCTAssertTrue(result.callHadReturned, @"completion ran inline, before the read call returned");
}

- (NSDictionary<NSString *, id> *)assertSingleDataValueForPath:(MTRAttributePath *)path inResult:(MTRAsyncReadContractResult *)result
{
    XCTAssertNil(result.error, @"read of %@ failed: %@", path, result.error);
    XCTAssertEqual(result.values.count, 1, @"values should hold exactly one entry: %@", result.values);
    XCTAssertEqualObjects(result.values.firstObject[MTRAttributePathKey], path);
    XCTAssertNil(result.values.firstObject[MTRErrorKey]);
    NSDictionary<NSString *, id> * data = result.values.firstObject[MTRDataKey];
    XCTAssertNotNil(data);
    return data;
}

#pragma mark - Delivery

- (void)testCachedAnswerIsDeliveredAfterTheCallReturnsEvenWhenCalledFromTheCompletionQueue
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self booleanValue:YES] forPath:[self onOffPath] fromSubscription:YES];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    MTRAsyncReadContractResult * result = [self readAsync:[self onOffPath] params:nil];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    [self assertSingleDataValueForPath:[self onOffPath] inResult:result];
    XCTAssertEqual([self readThroughs].count, 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
}

- (void)testReadThroughAnswerIsDeliveredOnTheGivenQueueNotWhereTheDeviceAnswered
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:NO];
    _readThroughAnswers = @{ [self onOffPath] : [self booleanValue:YES] };
    _readThroughAnswerQueue = dispatch_get_main_queue();

    MTRAsyncReadContractResult * result = [self readAsync:[self onOffPath] params:nil];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    [self assertSingleDataValueForPath:[self onOffPath] inResult:result];
    XCTAssertEqual([self readThroughs].count, 1);
}

- (void)testEarlyErrorIsDeliveredAfterTheCallReturnsEvenWhenCalledFromTheCompletionQueue
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [_controller suspend];
    [_device unitTestSyncRunOnDeviceQueue:^ {}];

    MTRAsyncReadContractResult * result = [self readAsync:[self onOffPath] params:nil];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    XCTAssertNil(result.values);
    XCTAssertEqualObjects(result.error.domain, MTRErrorDomain);
    XCTAssertEqual(result.error.code, MTRErrorCodeInvalidState, @"unexpected error: %@", result.error);
}

#pragma mark - Response shape

- (void)testValuesHoldOnlyTheRequestedPathWhenTheReadThroughReturnsMore
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:NO];
    _readThroughAnswers = @{
        [self onOffPath] : [self booleanValue:YES],
        [self onTimePath] : [self unsignedValue:30],
    };
    MTRAttributeRequestPath * onTimeRequest = [self requestPathForPath:[self onTimePath]];
    __auto_type onReadThrough = _delegate.onReadThrough;
    _delegate.onReadThrough = ^(NSArray<MTRAttributeRequestPath *> * paths, MTRDeviceResponseHandler completion) {
        onReadThrough([paths arrayByAddingObject:onTimeRequest], completion);
    };

    MTRAsyncReadContractResult * result = [self readAsync:[self onOffPath] params:nil];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:[self onOffPath] inResult:result];
    XCTAssertEqualObjects(data[MTRValueKey], @(YES));
}

#pragma mark - params

- (void)testUnknownAttributeIsReadThroughWhenParamsSayItIsNotReportable
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self unsignedValue:7] forPath:[self vendorAttributePath] fromSubscription:YES];
    _readThroughAnswers = @{ [self vendorAttributePath] : [self unsignedValue:8] };

    MTRAsyncReadContractResult * result = [self readAsync:[self vendorAttributePath]
                                                   params:[self paramsAssumingUnknownAttributesReportable:NO]];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:[self vendorAttributePath] inResult:result];
    XCTAssertEqualObjects(data[MTRValueKey], @(8));
    NSArray<NSArray<MTRAttributeRequestPath *> *> * readThroughs = [self readThroughs];
    XCTAssertEqual(readThroughs.count, 1);
    XCTAssertTrue([readThroughs.firstObject containsObject:[self requestPathForPath:[self vendorAttributePath]]], @"%@", readThroughs);
}

- (void)testUnknownAttributeIsAnsweredFromCacheWhenParamsSayItIsReportable
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self unsignedValue:7] forPath:[self vendorAttributePath] fromSubscription:YES];
    _readThroughAnswers = @{ [self vendorAttributePath] : [self unsignedValue:8] };
    MTRReadParams * params = [self paramsAssumingUnknownAttributesReportable:YES];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    MTRAsyncReadContractResult * result = [self readAsync:[self vendorAttributePath] params:params];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:[self vendorAttributePath] inResult:result];
    XCTAssertEqual([self readThroughs].count, 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
    MTRAttributePath * path = [self vendorAttributePath];
    XCTAssertEqualObjects(data, [_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:params]);
}

- (void)testUnknownAttributeIsAnsweredFromCacheWithNilParams
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self unsignedValue:7] forPath:[self vendorAttributePath] fromSubscription:YES];
    _readThroughAnswers = @{ [self vendorAttributePath] : [self unsignedValue:8] };

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    MTRAsyncReadContractResult * result = [self readAsync:[self vendorAttributePath] params:nil];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:[self vendorAttributePath] inResult:result];
    XCTAssertEqual([self readThroughs].count, 0);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
    MTRAttributePath * path = [self vendorAttributePath];
    XCTAssertEqualObjects(data, [_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil]);
}

- (void)testChangesOmittedAttributeIsReadThroughEvenWithAReportingSubscription
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self unsignedValue:1000] forPath:[self utcTimePath] fromSubscription:NO];
    _readThroughAnswers = @{ [self utcTimePath] : [self unsignedValue:2000] };

    MTRAsyncReadContractResult * result = [self readAsync:[self utcTimePath] params:[self paramsAssumingUnknownAttributesReportable:YES]];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:[self utcTimePath] inResult:result];
    XCTAssertEqualObjects(data[MTRValueKey], @(2000));
    NSArray<NSArray<MTRAttributeRequestPath *> *> * readThroughs = [self readThroughs];
    XCTAssertEqual(readThroughs.count, 1);
    XCTAssertTrue([readThroughs.firstObject containsObject:[self requestPathForPath:[self utcTimePath]]], @"%@", readThroughs);
}

- (void)testFilterByFabricNeitherForcesNorSuppressesAReadThrough
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    [self injectValue:[self booleanValue:YES] forPath:[self onOffPath] fromSubscription:YES];
    [self injectValue:[self unsignedValue:1000] forPath:[self utcTimePath] fromSubscription:NO];
    _readThroughAnswers = @{ [self utcTimePath] : [self unsignedValue:2000] };

    for (NSNumber * filterByFabric in @[ @(YES), @(NO) ]) {
        MTRReadParams * params = [self paramsFilteringByFabric:filterByFabric.boolValue];

        NSUInteger readThroughCount = [self readThroughs].count;
        MTRAsyncReadContractResult * reportable = [self readAsync:[self onOffPath] params:params];
        [self assertDeliveredAsynchronouslyOnGivenQueue:reportable];
        [self assertSingleDataValueForPath:[self onOffPath] inResult:reportable];
        XCTAssertEqual([self readThroughs].count, readThroughCount, @"filterByFabric = %@ forced a read-through", filterByFabric);

        MTRAsyncReadContractResult * changesOmitted = [self readAsync:[self utcTimePath] params:params];
        [self assertDeliveredAsynchronouslyOnGivenQueue:changesOmitted];
        [self assertSingleDataValueForPath:[self utcTimePath] inResult:changesOmitted];
        XCTAssertEqual([self readThroughs].count, readThroughCount + 1, @"filterByFabric = %@ suppressed a read-through", filterByFabric);
    }
}

- (void)testFilterByFabricDoesNotFilterTheReturnedValue
{
    [self addReadThroughDelegateWithSubscriptionAbleToReport:YES];
    MTRAttributePath * fabrics = [MTRAttributePath attributePathWithEndpointID:@(0)
                                                                     clusterID:@(MTRClusterIDTypeOperationalCredentialsID)
                                                                   attributeID:@(MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID)];
    NSMutableArray * descriptors = [NSMutableArray array];
    for (NSNumber * fabricIndex in @[ @(1), @(2) ]) {
        [descriptors addObject:@{
            MTRDataKey : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[ @{
                    MTRContextTagKey : @(kFabricIndexFieldTag),
                    MTRDataKey : [self unsignedValue:fabricIndex.unsignedLongLongValue],
                } ],
            },
        }];
    }
    [self injectValue:@{ MTRTypeKey : MTRArrayValueType, MTRValueKey : descriptors } forPath:fabrics fromSubscription:YES];

    MTRAsyncReadContractResult * result = [self readAsync:fabrics params:[self paramsFilteringByFabric:YES]];

    [self assertDeliveredAsynchronouslyOnGivenQueue:result];
    NSDictionary<NSString *, id> * data = [self assertSingleDataValueForPath:fabrics inResult:result];
    XCTAssertEqualObjects(data, [_device readAttributeWithEndpointID:fabrics.endpoint clusterID:fabrics.cluster attributeID:fabrics.attribute params:nil]);
    XCTAssertEqual([data[MTRValueKey] count], 2, @"value was fabric-filtered: %@", data);
}

#pragma mark - Synchronous read guards

- (void)addDelegateWhoseSubscriptionNeverEstablishes
{
    _delegate = [[MTRDeviceTestDelegate alloc] init];
    [_device addDelegate:_delegate queue:dispatch_get_main_queue()];
}

- (void)testSyncReadOfUnknownAttributeReadsThroughOnlyWhenParamsSayItIsNotReportable
{
    [self addDelegateWhoseSubscriptionNeverEstablishes];
    NSDictionary<NSString *, id> * cached = [self unsignedValue:7];
    [self injectValue:cached forPath:[self vendorAttributePath] fromSubscription:YES];
    MTRAttributePath * path = [self vendorAttributePath];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    NSArray<MTRReadParams *> * reportableParams = @[
        [self paramsAssumingUnknownAttributesReportable:YES],
        [self paramsFilteringByFabric:YES],
        [self paramsFilteringByFabric:NO],
    ];
    XCTAssertEqualObjects([_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil][MTRValueKey], cached[MTRValueKey]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);
    for (MTRReadParams * params in reportableParams) {
        XCTAssertEqualObjects([_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:params][MTRValueKey], cached[MTRValueKey]);
        XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount, @"%@", params);
    }

    MTRReadParams * notReportable = [self paramsAssumingUnknownAttributesReportable:NO];
    XCTAssertEqualObjects([_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:notReportable][MTRValueKey], cached[MTRValueKey]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 1);
    // A second read of the same path finds the queued read and adds nothing.
    XCTAssertEqualObjects([_device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:notReportable][MTRValueKey], cached[MTRValueKey]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 1);
}

- (void)testSyncReadOfChangesOmittedAttributeReadsThroughWithAReportingSubscription
{
    [self addDelegateWhoseSubscriptionNeverEstablishes];
    [self injectValue:[self booleanValue:YES] forPath:[self onOffPath] fromSubscription:YES];
    NSDictionary<NSString *, id> * cached = [self unsignedValue:1000];
    [self injectValue:cached forPath:[self utcTimePath] fromSubscription:NO];

    NSUInteger itemCount = _device.asyncWorkQueue.itemCount;
    MTRAttributePath * onOff = [self onOffPath];
    XCTAssertEqualObjects([_device readAttributeWithEndpointID:onOff.endpoint clusterID:onOff.cluster attributeID:onOff.attribute params:nil][MTRValueKey], @(YES));
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount);

    MTRAttributePath * utcTime = [self utcTimePath];
    XCTAssertEqualObjects([_device readAttributeWithEndpointID:utcTime.endpoint clusterID:utcTime.cluster attributeID:utcTime.attribute params:nil][MTRValueKey], cached[MTRValueKey]);
    XCTAssertEqual(_device.asyncWorkQueue.itemCount, itemCount + 1);
}

@end
