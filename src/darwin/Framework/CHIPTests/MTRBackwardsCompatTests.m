/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

// module headers
#import <Matter/Matter.h>

#import "MTRErrorTestUtils.h"
#import "MTRTestKeys.h"
#import "MTRTestResetCommissioneeHelper.h"
#import "MTRTestStorage.h"

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

// Fixture: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kCASESetupTimeoutInSeconds = 30;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static MTRBaseDevice * sConnectedDevice;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

static MTRBaseDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(sConnectedDevice);
    return sConnectedDevice;
}

@interface MTRBackwardsCompatTestPairingDelegate : NSObject <MTRDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTRBackwardsCompatTestPairingDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)onPairingComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    NSError * commissionError = nil;
    XCTAssertTrue([sController commissionDevice:kDeviceId commissioningParams:[[MTRCommissioningParameters alloc] init] error:&commissionError]);
    XCTAssertNil(commissionError);

    // Keep waiting for onCommissioningComplete
}

- (void)onCommissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface MTRBackwardsCompatTests : XCTestCase
@end

@implementation MTRBackwardsCompatTests

+ (void)setUp
{
    [super setUp];

    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);
    XCTAssertTrue([factory startup:factoryParams]);

    XCTAssertNotNil(sTestKeys = [[MTRTestKeys alloc] init]);

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:sTestKeys fabricId:1 ipk:sTestKeys.ipk];
    params.vendorId = @(kTestVendorId);

    MTRDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * pairing = [[MTRBackwardsCompatTestPairingDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    [controller setPairingDelegate:pairing queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue([controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error]);
    XCTAssertNil(error);

    XCTAssertEqual([XCTWaiter waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds], XCTWaiterResultCompleted);

    __block XCTestExpectation * connectionExpectation = [[XCTestExpectation alloc] initWithDescription:@"CASE established"];
    [controller getBaseDevice:kDeviceId
                        queue:dispatch_get_main_queue()
            completionHandler:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                XCTAssertEqual(error.code, 0);
                [connectionExpectation fulfill];
                sConnectedDevice = device;
                connectionExpectation = nil;
            }];
    XCTAssertEqual([XCTWaiter waitForExpectations:@[ connectionExpectation ] timeout:kCASESetupTimeoutInSeconds], XCTWaiterResultCompleted);
}

+ (void)tearDown
{
    ResetCommissionee(GetConnectedDevice(), dispatch_get_main_queue(), nil, kTimeoutInSeconds);

    [sController shutdown];
    XCTAssertFalse([sController isRunning]);
    [[MTRControllerFactory sharedInstance] shutdown];

    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

#define CHECK_RETURN_TYPE(sig, type)                                       \
    do {                                                                   \
        XCTAssertNotNil(sig);                                              \
        XCTAssertTrue(strcmp([sig methodReturnType], @encode(type)) == 0); \
    } while (0)

/**
 * Arguments 0 and 1 are the implicit self and _cmd arguments; the real arguments begin at index 2.
 */
#define CHECK_ARGUMENT(sig, index, type)                                                     \
    do {                                                                                     \
        XCTAssertTrue(strcmp([sig getArgumentTypeAtIndex:(index) + 2], @encode(type)) == 0); \
    } while (0)

#define CHECK_READONLY_PROPERTY(instance, propName, type)                                          \
    do {                                                                                           \
        NSMethodSignature * signature = [instance methodSignatureForSelector:@selector(propName)]; \
        CHECK_RETURN_TYPE(signature, type);                                                        \
        /* Check that getting the property directly compiles too */                                \
        (void) instance.propName;                                                                  \
    } while (0)

#define CHECK_PROPERTY(instance, propName, setterName, type)                                          \
    do {                                                                                              \
        CHECK_READONLY_PROPERTY(instance, propName, type);                                            \
        NSMethodSignature * signature = [instance methodSignatureForSelector:@selector(setterName:)]; \
        CHECK_RETURN_TYPE(signature, void);                                                           \
        CHECK_ARGUMENT(signature, 0, type);                                                           \
    } while (0)

/**
 * Basic tests that all the selectors we expect are in place.  This does some
 * limited checking of the argument types (enough to check for boxed numbers vs
 * primitives, say, but not enough to distinguish various object types).
 *
 * Also missing: checks for whether init/new are available or not
 * (NS_UNAVAILABLE), since the selector setup is the same in both cases.
 */

- (void)test001_MTRAsyncCallbackWorkQueue
{
    NSMethodSignature * sig;

    sig = [MTRAsyncCallbackWorkQueue instanceMethodSignatureForSelector:@selector(initWithContext:queue:)];
    CHECK_RETURN_TYPE(sig, id);
    CHECK_ARGUMENT(sig, 0, id);
    CHECK_ARGUMENT(sig, 1, dispatch_queue_t);

    sig = [MTRAsyncCallbackWorkQueue instanceMethodSignatureForSelector:@selector(invalidate)];
    CHECK_RETURN_TYPE(sig, void);

    sig = [MTRAsyncCallbackWorkQueue instanceMethodSignatureForSelector:@selector(enqueueWorkItem:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, id);
}

- (void)test002_MTRAsyncCallbackQueueWorkItem
{
    NSMethodSignature * sig;

    sig = [MTRAsyncCallbackQueueWorkItem instanceMethodSignatureForSelector:@selector(initWithQueue:)];
    CHECK_RETURN_TYPE(sig, id);
    CHECK_ARGUMENT(sig, 0, dispatch_queue_t);

    __auto_type * item = [[MTRAsyncCallbackQueueWorkItem alloc] initWithQueue:dispatch_get_main_queue()];

    CHECK_PROPERTY(item, readyHandler, setReadyHandler, MTRAsyncCallbackReadyHandler);
    CHECK_PROPERTY(item, cancelHandler, setCancelHandler, dispatch_block_t);

    sig = [MTRAsyncCallbackQueueWorkItem instanceMethodSignatureForSelector:@selector(endWork)];
    CHECK_RETURN_TYPE(sig, void);

    sig = [MTRAsyncCallbackQueueWorkItem instanceMethodSignatureForSelector:@selector(retryWork)];
    CHECK_RETURN_TYPE(sig, void);
}

- (void)test003_MTRAttestationInfo
{
    __auto_type * info = [[AttestationInfo alloc] init];
    CHECK_PROPERTY(info, challenge, setChallenge, NSData *);
    CHECK_PROPERTY(info, nonce, setNonce, NSData *);
    CHECK_PROPERTY(info, elements, setElements, NSData *);
    CHECK_PROPERTY(info, elementsSignature, setElementsSignature, NSData *);
    CHECK_PROPERTY(info, dac, setDac, NSData *);
    CHECK_PROPERTY(info, pai, setPai, NSData *);
    CHECK_PROPERTY(info, certificationDeclaration, setCertificationDeclaration, NSData *);
    CHECK_PROPERTY(info, firmwareInfo, setFirmwareInfo, NSData *);

    NSMethodSignature * sig;

    sig = [AttestationInfo instanceMethodSignatureForSelector:@selector
                           (initWithChallenge:nonce:elements:elementsSignature:dac:pai:certificationDeclaration:firmwareInfo:)];
    CHECK_RETURN_TYPE(sig, id);
    CHECK_ARGUMENT(sig, 0, NSData *);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, NSData *);
    CHECK_ARGUMENT(sig, 3, NSData *);
    CHECK_ARGUMENT(sig, 4, NSData *);
    CHECK_ARGUMENT(sig, 5, NSData *);
    CHECK_ARGUMENT(sig, 6, NSData *);
    CHECK_ARGUMENT(sig, 7, NSData *);
}

- (void)test004_MTRAttributeCacheContainer
{
    NSMethodSignature * sig;

    sig = [MTRAttributeCacheContainer
        instanceMethodSignatureForSelector:@selector(readAttributeWithEndpointId:clusterId:attributeId:clientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 4, void (^)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error));
}

- (void)test005_MTRBaseClusters
{
    // TODO: Need to think a bit about how to test this exhaustively, but in the
    // meantime just test a representative cluster with commands and attributes
    // on it, including commands that have no fields and writable attributes.
    NSMethodSignature * sig;

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(initWithDevice:endpoint:queue:)];
    CHECK_RETURN_TYPE(sig, id);
    CHECK_ARGUMENT(sig, 0, MTRBaseDevice *);
    CHECK_ARGUMENT(sig, 1, uint16_t);
    CHECK_ARGUMENT(sig, 2, dispatch_queue_t);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(offWithParams:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, MTROnOffClusterOffParams *);
    CHECK_ARGUMENT(sig, 1, StatusCompletion);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(offWithCompletionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, StatusCompletion);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(offWithEffectWithParams:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, MTROnOffClusterOffWithEffectParams *);
    CHECK_ARGUMENT(sig, 1, StatusCompletion);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(readAttributeOnTimeWithCompletionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, void (^)(NSNumber * _Nullable value, NSError * _Nullable error));

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(writeAttributeOnTimeWithValue:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, StatusCompletion);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector(writeAttributeOnTimeWithValue:
                                                                                                   params:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, MTRWriteParams *);
    CHECK_ARGUMENT(sig, 2, StatusCompletion);

    sig = [MTRBaseClusterOnOff instanceMethodSignatureForSelector:@selector
                               (subscribeAttributeOnTimeWithMinInterval:maxInterval:params:subscriptionEstablished:reportHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, MTRSubscribeParams *);
    CHECK_ARGUMENT(sig, 3, SubscriptionEstablishedHandler);
    CHECK_ARGUMENT(sig, 4, void (^)(NSNumber * _Nullable value, NSError * _Nullable error));

    sig = [MTRBaseClusterOnOff
        methodSignatureForSelector:@selector(readAttributeOnTimeWithAttributeCache:endpoint:queue:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, MTRAttributeCacheContainer *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 3, void (^)(NSNumber * _Nullable value, NSError * _Nullable error));
}

- (void)test006_MTRBaseDevice
{
    NSMethodSignature * sig;

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (subscribeWithQueue:
                                 minInterval:maxInterval:params:cacheContainer:attributeReportHandler:eventReportHandler
                                            :errorHandler:subscriptionEstablished:resubscriptionScheduled:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 1, uint16_t);
    CHECK_ARGUMENT(sig, 2, uint16_t);
    CHECK_ARGUMENT(sig, 3, MTRSubscribeParams *);
    CHECK_ARGUMENT(sig, 4, MTRAttributeCacheContainer *);
    CHECK_ARGUMENT(sig, 5, MTRDeviceReportHandler);
    CHECK_ARGUMENT(sig, 6, MTRDeviceReportHandler);
    CHECK_ARGUMENT(sig, 7, MTRDeviceErrorHandler);
    CHECK_ARGUMENT(sig, 8, dispatch_block_t);
    CHECK_ARGUMENT(sig, 9, MTRDeviceResubscriptionScheduledHandler);

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (readAttributeWithEndpointId:clusterId:attributeId:params:clientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, MTRReadParams *);
    CHECK_ARGUMENT(sig, 4, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 5, MTRDeviceResponseHandler);

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (writeAttributeWithEndpointId:clusterId:attributeId:value:timedWriteTimeout:clientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, id);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 6, MTRDeviceResponseHandler);

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (invokeCommandWithEndpointId:
                                            clusterId:commandId:commandFields:timedInvokeTimeout:clientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, id);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 6, MTRDeviceResponseHandler);

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (subscribeAttributeWithEndpointId:
                                                 clusterId:attributeId:minInterval:maxInterval:params:clientQueue:reportHandler
                                                          :subscriptionEstablished:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, NSNumber *);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, MTRSubscribeParams *);
    CHECK_ARGUMENT(sig, 6, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 7, MTRDeviceResponseHandler);
    CHECK_ARGUMENT(sig, 8, void (^)(void));

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector(deregisterReportHandlersWithClientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 1, void (^)(void));

    sig = [MTRBaseDevice instanceMethodSignatureForSelector:@selector
                         (openCommissioningWindowWithSetupPasscode:discriminator:duration:queue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 4, MTRDeviceOpenCommissioningWindowHandler);

    // TODO add tests that actually invoking the old methods on an MTRBaseDevice calls the new methods.
}

- (void)test007_MTRAttributePath
{
    XCTAssertTrue([MTRAttributePath conformsToProtocol:@protocol(NSCopying)]);

    __auto_type * path = [MTRAttributePath attributePathWithEndpointId:@(0) clusterId:@(0) attributeId:@(0)];

    CHECK_READONLY_PROPERTY(path, endpoint, NSNumber *);
    CHECK_READONLY_PROPERTY(path, cluster, NSNumber *);
    CHECK_READONLY_PROPERTY(path, attribute, NSNumber *);

    NSMethodSignature * sig;

    sig = [MTRAttributePath methodSignatureForSelector:@selector(attributePathWithEndpointId:clusterId:attributeId:)];
    CHECK_RETURN_TYPE(sig, MTRAttributePath *);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
}

- (void)test008_MTREventPath
{
    __auto_type * path = [MTREventPath eventPathWithEndpointId:@(0) clusterId:@(0) eventId:@(0)];

    CHECK_READONLY_PROPERTY(path, endpoint, NSNumber *);
    CHECK_READONLY_PROPERTY(path, cluster, NSNumber *);
    CHECK_READONLY_PROPERTY(path, event, NSNumber *);

    NSMethodSignature * sig;

    sig = [MTREventPath methodSignatureForSelector:@selector(eventPathWithEndpointId:clusterId:eventId:)];
    CHECK_RETURN_TYPE(sig, MTREventPath *);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
}

- (void)test009_MTRCommandPath
{
    __auto_type * path = [MTRCommandPath commandPathWithEndpointId:@(0) clusterId:@(0) commandId:@(0)];

    CHECK_READONLY_PROPERTY(path, endpoint, NSNumber *);
    CHECK_READONLY_PROPERTY(path, cluster, NSNumber *);
    CHECK_READONLY_PROPERTY(path, command, NSNumber *);

    NSMethodSignature * sig;

    sig = [MTRCommandPath methodSignatureForSelector:@selector(commandPathWithEndpointId:clusterId:commandId:)];
    CHECK_RETURN_TYPE(sig, MTRCommandPath *);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
}

- (void)test010_MTRAttributeReport
{
    __auto_type * report = [[MTRAttributeReport alloc] init];

    CHECK_READONLY_PROPERTY(report, path, MTRAttributePath *);
    CHECK_READONLY_PROPERTY(report, value, id);
    CHECK_READONLY_PROPERTY(report, error, NSError *);
}

- (void)test011_MTREventReport
{
    __auto_type * report = [[MTREventReport alloc] init];

    CHECK_READONLY_PROPERTY(report, path, MTREventPath *);
    CHECK_READONLY_PROPERTY(report, eventNumber, NSNumber *);
    CHECK_READONLY_PROPERTY(report, priority, NSNumber *);
    CHECK_READONLY_PROPERTY(report, timestamp, NSNumber *);
    CHECK_READONLY_PROPERTY(report, value, id);
    CHECK_READONLY_PROPERTY(report, error, NSError *);
}

- (void)test012_MTRCSRInfo
{
    __auto_type * info = [[CSRInfo alloc] init];

    CHECK_PROPERTY(info, nonce, setNonce, NSData *);
    CHECK_PROPERTY(info, elements, setElements, NSData *);
    CHECK_PROPERTY(info, elementsSignature, setElementsSignature, NSData *);
    CHECK_PROPERTY(info, csr, setCsr, NSData *);

    NSMethodSignature * sig;

    sig = [CSRInfo instanceMethodSignatureForSelector:@selector(initWithNonce:elements:elementsSignature:csr:)];
    CHECK_RETURN_TYPE(sig, CSRInfo *);
    CHECK_ARGUMENT(sig, 0, NSData *);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, NSData *);
    CHECK_ARGUMENT(sig, 3, NSData *);
}

- (void)test013_MTRCertificates
{
    NSMethodSignature * sig;

    sig = [MTRCertificates methodSignatureForSelector:@selector(generateRootCertificate:issuerId:fabricId:error:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, NSError * __autoreleasing *);

    sig = [MTRCertificates methodSignatureForSelector:@selector
                           (generateIntermediateCertificate:rootCertificate:intermediatePublicKey:issuerId:fabricId:error:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, SecKeyRef);
    CHECK_ARGUMENT(sig, 3, NSNumber *);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, NSError * __autoreleasing *);

    sig = [MTRCertificates methodSignatureForSelector:@selector
                           (generateOperationalCertificate:
                                        signingCertificate:operationalPublicKey:fabricId:nodeId:caseAuthenticatedTags:error:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, SecKeyRef);
    CHECK_ARGUMENT(sig, 3, NSNumber *);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, NSArray<NSNumber *> *);
    CHECK_ARGUMENT(sig, 6, NSError * __autoreleasing *);

    sig = [MTRCertificates methodSignatureForSelector:@selector(keypair:matchesCertificate:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSData *);

    sig = [MTRCertificates methodSignatureForSelector:@selector(isCertificate:equalTo:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, NSData *);
    CHECK_ARGUMENT(sig, 1, NSData *);

    sig = [MTRCertificates methodSignatureForSelector:@selector(generateCertificateSigningRequest:error:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);

    sig = [MTRCertificates methodSignatureForSelector:@selector(convertX509Certificate:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, NSData *);
}

- (void)test014_MTRWriteParams
{
    XCTAssertTrue([MTRWriteParams conformsToProtocol:@protocol(NSCopying)]);

    __auto_type * params = [[MTRWriteParams alloc] init];

    CHECK_PROPERTY(params, timedWriteTimeout, setTimedWriteTimeout, NSNumber *);
    CHECK_PROPERTY(params, dataVersion, setDataVersion, NSNumber *);
}

- (void)test015_MTRReadParams
{
    XCTAssertTrue([MTRReadParams conformsToProtocol:@protocol(NSCopying)]);

    __auto_type * params = [[MTRReadParams alloc] init];

    CHECK_PROPERTY(params, fabricFiltered, setFabricFiltered, NSNumber *);
}

- (void)test016_MTRSubscribeParams
{
    XCTAssertTrue([MTRSubscribeParams conformsToProtocol:@protocol(NSCopying)]);
    XCTAssertTrue([MTRSubscribeParams isSubclassOfClass:[MTRReadParams class]]);

    __auto_type * params = [[MTRSubscribeParams alloc] init];

    CHECK_PROPERTY(params, keepPreviousSubscriptions, setKeepPreviousSubscriptions, NSNumber *);
    CHECK_PROPERTY(params, autoResubscribe, setAutoResubscribe, NSNumber *);
}

- (void)test017_MTRClusterConstants
{
    // TODO: Figure out what and how to test here.
}

- (void)test018_MTRClusters
{
    // TODO: Need to think a bit about how to test this exhaustively, but in the
    // meantime just test a representative cluster with commands and attributes
    // on it, including commands that have no fields and writable attributes.
    NSMethodSignature * sig;

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(initWithDevice:endpoint:queue:)];
    CHECK_RETURN_TYPE(sig, MTRClusterOnOff *);
    CHECK_ARGUMENT(sig, 0, MTRDevice *);
    CHECK_ARGUMENT(sig, 1, uint16_t);
    CHECK_ARGUMENT(sig, 2, dispatch_queue_t);

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(offWithParams:
                                                                       expectedValues:expectedValueInterval:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, MTROnOffClusterOffParams *);
    CHECK_ARGUMENT(sig, 1, NSArray *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, StatusCompletion);

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(offWithExpectedValues:
                                                                        expectedValueInterval:completionHandler:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSArray *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, StatusCompletion);

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(readAttributeOnTimeWithParams:)];
    CHECK_RETURN_TYPE(sig, NSDictionary *);
    CHECK_ARGUMENT(sig, 0, MTRReadParams *);

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(writeAttributeOnTimeWithValue:expectedValueInterval:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSDictionary *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);

    sig = [MTRClusterOnOff instanceMethodSignatureForSelector:@selector(writeAttributeOnTimeWithValue:
                                                                                expectedValueInterval:params:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSDictionary *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, MTRWriteParams *);
}

- (void)test019_MTRCommandPayloadsObjc
{
    // TODO: Figure out what and how to test here.
}

- (void)test020_MTRCommissioningParameters
{
    __auto_type * params = [[MTRCommissioningParameters alloc] init];

    CHECK_PROPERTY(params, CSRNonce, setCSRNonce, NSData *);
    CHECK_PROPERTY(params, attestationNonce, setAttestationNonce, NSData *);
    CHECK_PROPERTY(params, wifiSSID, setWifiSSID, NSData *);
    CHECK_PROPERTY(params, wifiCredentials, setWifiCredentials, NSData *);
    CHECK_PROPERTY(params, threadOperationalDataset, setThreadOperationalDataset, NSData *);
    CHECK_PROPERTY(params, deviceAttestationDelegate, setDeviceAttestationDelegate, id<MTRDeviceAttestationDelegate>);
    CHECK_PROPERTY(params, failSafeExpiryTimeoutSecs, setFailSafeExpiryTimeoutSecs, NSNumber *);
}

- (void)test021_MTRControllerFactoryParams
{
    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * params = [[MTRControllerFactoryParams alloc] initWithStorage:storage];
    CHECK_READONLY_PROPERTY(params, storageDelegate, id<MTRPersistentStorageDelegate>);
    CHECK_PROPERTY(params, otaProviderDelegate, setOtaProviderDelegate, id<MTROTAProviderDelegate>);
    CHECK_PROPERTY(params, paaCerts, setPaaCerts, NSArray *);
    CHECK_PROPERTY(params, cdCerts, setCdCerts, NSArray *);
    CHECK_PROPERTY(params, port, setPort, NSNumber *);
    CHECK_PROPERTY(params, startServer, setStartServer, BOOL);

    NSMethodSignature * sig;

    sig = [MTRControllerFactoryParams instanceMethodSignatureForSelector:@selector(initWithStorage:)];
    CHECK_RETURN_TYPE(sig, MTRControllerFactoryParams *);
    CHECK_ARGUMENT(sig, 0, id<MTRPersistentStorageDelegate>);
}

- (void)test022_MTRControllerFactory
{
    __auto_type * instance = [MTRControllerFactory sharedInstance];
    CHECK_READONLY_PROPERTY(instance, isRunning, BOOL);

    NSMethodSignature * sig;

    sig = [MTRControllerFactory methodSignatureForSelector:@selector(sharedInstance)];
    CHECK_RETURN_TYPE(sig, MTRControllerFactory *);

    sig = [MTRControllerFactory instanceMethodSignatureForSelector:@selector(startup:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, MTRControllerFactoryParams *);

    sig = [MTRControllerFactory instanceMethodSignatureForSelector:@selector(shutdown)];
    CHECK_RETURN_TYPE(sig, void);

    sig = [MTRControllerFactory instanceMethodSignatureForSelector:@selector(startControllerOnExistingFabric:)];
    CHECK_RETURN_TYPE(sig, MTRDeviceController *);
    CHECK_ARGUMENT(sig, 0, MTRDeviceControllerStartupParams *);

    sig = [MTRControllerFactory instanceMethodSignatureForSelector:@selector(startControllerOnNewFabric:)];
    CHECK_RETURN_TYPE(sig, MTRDeviceController *);
    CHECK_ARGUMENT(sig, 0, MTRDeviceControllerStartupParams *);
}

- (void)test023_MTRDevice
{
    NSMethodSignature * sig;

    sig = [MTRDevice methodSignatureForSelector:@selector(deviceWithNodeID:deviceController:)];
    CHECK_RETURN_TYPE(sig, MTRDevice *);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, MTRDeviceController *);

    __auto_type * instance = [MTRDevice deviceWithNodeID:1 deviceController:sController];

    CHECK_READONLY_PROPERTY(instance, state, MTRDeviceState);

    sig = [MTRDevice instanceMethodSignatureForSelector:@selector(setDelegate:queue:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, id<MTRDeviceDelegate>);
    CHECK_ARGUMENT(sig, 1, dispatch_queue_t);

    sig = [MTRDevice instanceMethodSignatureForSelector:@selector(readAttributeWithEndpointID:clusterID:attributeID:params:)];
    CHECK_RETURN_TYPE(sig, NSDictionary *);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, MTRReadParams *);

    sig = [MTRDevice instanceMethodSignatureForSelector:@selector
                     (writeAttributeWithEndpointID:clusterID:attributeID:value:expectedValueInterval:timedWriteTimeout:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, id);
    CHECK_ARGUMENT(sig, 4, NSNumber *);
    CHECK_ARGUMENT(sig, 5, NSNumber *);

    sig = [MTRDevice instanceMethodSignatureForSelector:@selector
                     (invokeCommandWithEndpointID:
                                        clusterID:commandID:commandFields:expectedValues:expectedValueInterval:timedInvokeTimeout
                                                 :clientQueue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, id);
    CHECK_ARGUMENT(sig, 4, NSArray *);
    CHECK_ARGUMENT(sig, 5, NSNumber *);
    CHECK_ARGUMENT(sig, 6, NSNumber *);
    CHECK_ARGUMENT(sig, 7, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 8, MTRDeviceResponseHandler);

    sig = [MTRDevice instanceMethodSignatureForSelector:@selector
                     (openCommissioningWindowWithSetupPasscode:discriminator:duration:queue:completion:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSNumber *);
    CHECK_ARGUMENT(sig, 3, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 4, MTRDeviceOpenCommissioningWindowHandler);

    // TODO add tests that actually invoking the old methods on an MTRBaseDevice calls the new methods.
}

- (void)test024_MTRDeviceDelegate
{
    // TODO: Figure out how to test this. conformsToProtocol does not do what we
    // want, since we want to check whether declaring a specific set of
    // selectors would conform to the protocol.
}

- (void)test025_MTRDeviceAttestationDeviceInfo
{
    // Cheating by not initializing the object....
    __auto_type * info = [MTRDeviceAttestationDeviceInfo alloc];
    CHECK_READONLY_PROPERTY(info, dacCertificate, NSData *);
    CHECK_READONLY_PROPERTY(info, dacPAICertificate, NSData *);
    CHECK_READONLY_PROPERTY(info, certificateDeclaration, NSData *);
}

- (void)test026_MTRDeviceAttestationDelegate
{
    // TODO: Figure out how to test this.
}

- (void)test027_MTRDeviceControllerXPC
{
    NSMethodSignature * sig;

    sig = [MTRDeviceController methodSignatureForSelector:@selector(sharedControllerWithId:xpcConnectBlock:)];
    CHECK_RETURN_TYPE(sig, MTRDeviceController *);
    CHECK_ARGUMENT(sig, 0, id<NSCopying>);
    CHECK_ARGUMENT(sig, 1, NSXPCConnection * (^)(void) );

    sig = [MTRDeviceController methodSignatureForSelector:@selector(encodeXPCResponseValues:)];
    CHECK_RETURN_TYPE(sig, NSArray *);
    CHECK_ARGUMENT(sig, 0, NSArray *);

    sig = [MTRDeviceController methodSignatureForSelector:@selector(decodeXPCResponseValues:)];
    CHECK_RETURN_TYPE(sig, NSArray *);
    CHECK_ARGUMENT(sig, 0, NSArray *);

    sig = [MTRDeviceController methodSignatureForSelector:@selector(encodeXPCReadParams:)];
    CHECK_RETURN_TYPE(sig, NSArray *);
    CHECK_ARGUMENT(sig, 0, MTRReadParams *);

    sig = [MTRDeviceController methodSignatureForSelector:@selector(decodeXPCReadParams:)];
    CHECK_RETURN_TYPE(sig, MTRReadParams *);
    CHECK_ARGUMENT(sig, 0, NSArray *);

    sig = [MTRDeviceController methodSignatureForSelector:@selector(encodeXPCSubscribeParams:)];
    CHECK_RETURN_TYPE(sig, NSArray *);
    CHECK_ARGUMENT(sig, 0, MTRSubscribeParams *);

    sig = [MTRDeviceController methodSignatureForSelector:@selector(decodeXPCSubscribeParams:)];
    CHECK_RETURN_TYPE(sig, MTRSubscribeParams *);
    CHECK_ARGUMENT(sig, 0, NSArray *);
}

- (void)test028_MTRDeviceControllerServerProtocol
{
    // TODO: Figure out how to test this.
}

- (void)test029_MTRDeviceControllerClientProtocol
{
    // TODO: Figure out how to test this.
}

- (void)test030_MTRDeviceController
{
    CHECK_READONLY_PROPERTY(sController, isRunning, BOOL);
    CHECK_READONLY_PROPERTY(sController, controllerNodeId, NSNumber *);

    NSMethodSignature * sig;

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(setupCommissioningSessionWithPayload:newNodeID:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);
    CHECK_ARGUMENT(sig, 2, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(pairDevice:discriminator:setupPINCode:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, uint16_t);
    CHECK_ARGUMENT(sig, 2, uint32_t);
    CHECK_ARGUMENT(sig, 3, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(pairDevice:address:port:setupPINCode:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSString *);
    CHECK_ARGUMENT(sig, 2, uint16_t);
    CHECK_ARGUMENT(sig, 3, uint32_t);
    CHECK_ARGUMENT(sig, 4, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(pairDevice:onboardingPayload:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSString *);
    CHECK_ARGUMENT(sig, 2, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(commissionDevice:commissioningParams:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, MTRCommissioningParameters *);
    CHECK_ARGUMENT(sig, 2, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(continueCommissioningDevice:
                                                                               ignoreAttestationFailure:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, void *);
    CHECK_ARGUMENT(sig, 1, BOOL);
    CHECK_ARGUMENT(sig, 2, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(stopDevicePairing:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(getDeviceBeingCommissioned:error:)];
    CHECK_RETURN_TYPE(sig, MTRBaseDevice *);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(getBaseDevice:queue:completionHandler:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, dispatch_queue_t);
    CHECK_ARGUMENT(sig, 2, MTRDeviceConnectionCallback);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(openPairingWindow:duration:error:)];
    CHECK_RETURN_TYPE(sig, BOOL);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSUInteger);
    CHECK_ARGUMENT(sig, 2, NSError * __autoreleasing *);

    sig = [MTRDeviceController
        instanceMethodSignatureForSelector:@selector(openPairingWindowWithPIN:duration:discriminator:setupPIN:error:)];
    CHECK_RETURN_TYPE(sig, NSString *);
    CHECK_ARGUMENT(sig, 0, uint64_t);
    CHECK_ARGUMENT(sig, 1, NSUInteger);
    CHECK_ARGUMENT(sig, 2, NSUInteger);
    CHECK_ARGUMENT(sig, 3, NSUInteger);
    CHECK_ARGUMENT(sig, 4, NSError * __autoreleasing *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(setPairingDelegate:queue:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, id<MTRDevicePairingDelegate>);
    CHECK_ARGUMENT(sig, 1, dispatch_queue_t);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(setNocChainIssuer:queue:)];
    CHECK_RETURN_TYPE(sig, void);
    CHECK_ARGUMENT(sig, 0, id<MTRNOCChainIssuer>);
    CHECK_ARGUMENT(sig, 1, dispatch_queue_t);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(fetchAttestationChallengeForDeviceId:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, uint64_t);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(computePaseVerifier:iterations:salt:)];
    CHECK_RETURN_TYPE(sig, NSData *);
    CHECK_ARGUMENT(sig, 0, uint32_t);
    CHECK_ARGUMENT(sig, 1, uint32_t);
    CHECK_ARGUMENT(sig, 2, NSData *);

    sig = [MTRDeviceController instanceMethodSignatureForSelector:@selector(shutdown)];
    CHECK_RETURN_TYPE(sig, void);
}

- (void)test031_MTRDeviceControllerStartupParams
{
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:sTestKeys fabricId:1 ipk:sTestKeys.ipk];
    CHECK_READONLY_PROPERTY(params, nocSigner, id<MTRKeypair>);
    CHECK_READONLY_PROPERTY(params, fabricId, uint64_t);
    CHECK_READONLY_PROPERTY(params, ipk, NSData *);
    CHECK_PROPERTY(params, vendorId, setVendorId, NSNumber *);
    CHECK_PROPERTY(params, nodeId, setNodeId, NSNumber *);
    CHECK_PROPERTY(params, rootCertificate, setRootCertificate, NSData *);
    CHECK_PROPERTY(params, intermediateCertificate, setIntermediateCertificate, NSData *);
    CHECK_READONLY_PROPERTY(params, operationalCertificate, NSData *);
    CHECK_PROPERTY(params, operationalKeypair, setOperationalKeypair, id<MTRKeypair>);

    NSMethodSignature * sig;

    sig = [MTRDeviceControllerStartupParams instanceMethodSignatureForSelector:@selector(initWithSigningKeypair:fabricId:ipk:)];
    CHECK_RETURN_TYPE(sig, MTRDeviceControllerStartupParams *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, uint64_t);
    CHECK_ARGUMENT(sig, 2, NSData *);

    sig = [MTRDeviceControllerStartupParams
        instanceMethodSignatureForSelector:@selector(initWithOperationalKeypair:
                                                         operationalCertificate:intermediateCertificate:rootCertificate:ipk:)];
    CHECK_RETURN_TYPE(sig, MTRDeviceControllerStartupParams *);
    CHECK_ARGUMENT(sig, 0, id<MTRKeypair>);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, NSData *);
    CHECK_ARGUMENT(sig, 3, NSData *);
    CHECK_ARGUMENT(sig, 4, NSData *);
}

- (void)test032_MTRDevicePairingDelegate
{
    // TODO: Figure out how to test this.
}

- (void)test033_MTRError
{
    // TODO: Figure out how to test this.
}

- (void)test034_MTRKeypair
{
    // TODO: Figure out how to test this.
}

- (void)test035_MTRManualSetupPayloadParser
{
    NSMethodSignature * sig;

    sig = [MTRManualSetupPayloadParser instanceMethodSignatureForSelector:@selector(initWithDecimalStringRepresentation:)];
    CHECK_RETURN_TYPE(sig, MTRManualSetupPayloadParser *);
    CHECK_ARGUMENT(sig, 0, NSString *);

    sig = [MTRManualSetupPayloadParser instanceMethodSignatureForSelector:@selector(populatePayload:)];
    CHECK_RETURN_TYPE(sig, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 0, NSError * __autoreleasing *);
}

- (void)test036_MTRQRCodeSetupPayloadParser
{
    NSMethodSignature * sig;

    sig = [MTRQRCodeSetupPayloadParser instanceMethodSignatureForSelector:@selector(initWithBase38Representation:)];
    CHECK_RETURN_TYPE(sig, MTRQRCodeSetupPayloadParser *);
    CHECK_ARGUMENT(sig, 0, NSString *);

    sig = [MTRQRCodeSetupPayloadParser instanceMethodSignatureForSelector:@selector(populatePayload:)];
    CHECK_RETURN_TYPE(sig, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 0, NSError * __autoreleasing *);
}

- (void)test037_MTROnboardingPayloadParser
{
    NSMethodSignature * sig;

    sig = [MTROnboardingPayloadParser methodSignatureForSelector:@selector(setupPayloadForOnboardingPayload:error:)];
    CHECK_RETURN_TYPE(sig, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 0, NSString *);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);
}

- (void)test038_MTRNOCChainIssuer
{
    // TODO: Figure out how to test this.
}

- (void)test039_MTROTAHeader
{
    __auto_type * header = [[MTROTAHeader alloc] init];
    CHECK_PROPERTY(header, vendorID, setVendorID, NSNumber *);
    CHECK_PROPERTY(header, productID, setProductID, NSNumber *);
    CHECK_PROPERTY(header, payloadSize, setPayloadSize, NSNumber *);
    CHECK_PROPERTY(header, softwareVersion, setSoftwareVersion, NSNumber *);
    CHECK_PROPERTY(header, softwareVersionString, setSoftwareVersionString, NSString *);
    CHECK_PROPERTY(header, releaseNotesURL, setReleaseNotesURL, NSString *);
    CHECK_PROPERTY(header, imageDigest, setImageDigest, NSData *);
    CHECK_PROPERTY(header, imageDigestType, setImageDigestType, MTROTAImageDigestType);
    CHECK_PROPERTY(header, minApplicableVersion, setMinApplicableVersion, NSNumber *);
    CHECK_PROPERTY(header, maxApplicableVersion, setMaxApplicableVersion, NSNumber *);
}

- (void)test040_MTROTAHeaderParser
{
    NSMethodSignature * sig;

    sig = [MTROTAHeaderParser methodSignatureForSelector:@selector(headerFromData:error:)];
    CHECK_RETURN_TYPE(sig, MTROTAHeader *);
    CHECK_ARGUMENT(sig, 0, NSData *);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);
}

- (void)test041_MTROTAProviderDelegate
{
    // TODO: Figure out how to test this.
}

- (void)test042_MTRPersistentStorageDelegate
{
    // TODO: Figure out how to test this.
}

- (void)test043_MTROptionalQRCodeInfo
{
    __auto_type * info = [[MTROptionalQRCodeInfo alloc] init];
    CHECK_PROPERTY(info, infoType, setInfoType, NSNumber *);
    CHECK_PROPERTY(info, tag, setTag, NSNumber *);
    CHECK_PROPERTY(info, integerValue, setIntegerValue, NSNumber *);
    CHECK_PROPERTY(info, stringValue, setStringValue, NSString *);
}

- (void)test044_MTRSetupPayload
{
    XCTAssertTrue([MTRSetupPayload conformsToProtocol:@protocol(NSSecureCoding)]);

    __auto_type * payload = [[MTRSetupPayload alloc] init];
    CHECK_PROPERTY(payload, version, setVersion, NSNumber *);
    CHECK_PROPERTY(payload, vendorID, setVendorID, NSNumber *);
    CHECK_PROPERTY(payload, productID, setProductID, NSNumber *);
    CHECK_PROPERTY(payload, commissioningFlow, setCommissioningFlow, MTRCommissioningFlow);
    CHECK_PROPERTY(payload, rendezvousInformation, setRendezvousInformation, NSNumber *);
    CHECK_PROPERTY(payload, discriminator, setDiscriminator, NSNumber *);
    CHECK_PROPERTY(payload, hasShortDiscriminator, setHasShortDiscriminator, BOOL);
    CHECK_PROPERTY(payload, setUpPINCode, setSetUpPINCode, NSNumber *);
    CHECK_PROPERTY(payload, serialNumber, setSerialNumber, NSString *);

    NSMethodSignature * sig;

    sig = [MTRSetupPayload instanceMethodSignatureForSelector:@selector(getAllOptionalVendorData:)];
    CHECK_RETURN_TYPE(sig, NSArray *);
    CHECK_ARGUMENT(sig, 0, NSError * __autoreleasing *);

    sig = [MTRSetupPayload methodSignatureForSelector:@selector(generateRandomPIN)];
    CHECK_RETURN_TYPE(sig, NSUInteger);

    sig = [MTRSetupPayload methodSignatureForSelector:@selector(generateRandomSetupPasscode)];
    CHECK_RETURN_TYPE(sig, NSNumber *);

    sig = [MTRSetupPayload methodSignatureForSelector:@selector(setupPayloadWithOnboardingPayload:error:)];
    CHECK_RETURN_TYPE(sig, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 0, NSString *);
    CHECK_ARGUMENT(sig, 1, NSError * __autoreleasing *);

    sig = [MTRSetupPayload instanceMethodSignatureForSelector:@selector(initWithSetupPasscode:discriminator:)];
    CHECK_RETURN_TYPE(sig, MTRSetupPayload *);
    CHECK_ARGUMENT(sig, 0, NSNumber *);
    CHECK_ARGUMENT(sig, 1, NSNumber *);

    sig = [MTRSetupPayload instanceMethodSignatureForSelector:@selector(manualEntryCode)];
    CHECK_RETURN_TYPE(sig, NSString *);

    sig = [MTRSetupPayload instanceMethodSignatureForSelector:@selector(qrCodeString:)];
    CHECK_RETURN_TYPE(sig, NSString *);
    CHECK_ARGUMENT(sig, 0, NSError * __autoreleasing *);
}

- (void)test045_MTRStructsObjc
{
    // TODO: Figure out what and how to test here.
}

- (void)test046_MTRThreadOperationalDataset
{
    // Cheating by not initing the object.
    __auto_type * dataset = [MTRThreadOperationalDataset alloc];
    CHECK_READONLY_PROPERTY(dataset, networkName, NSString *);
    CHECK_READONLY_PROPERTY(dataset, extendedPANID, NSData *);
    CHECK_READONLY_PROPERTY(dataset, masterKey, NSData *);
    CHECK_READONLY_PROPERTY(dataset, PSKc, NSData *);
    CHECK_PROPERTY(dataset, channel, setChannel, uint16_t);
    CHECK_READONLY_PROPERTY(dataset, panID, NSData *);

    NSMethodSignature * sig;

    sig = [MTRThreadOperationalDataset
        instanceMethodSignatureForSelector:@selector(initWithNetworkName:extendedPANID:masterKey:PSKc:channel:panID:)];
    CHECK_RETURN_TYPE(sig, MTRThreadOperationalDataset *);
    CHECK_ARGUMENT(sig, 0, NSString *);
    CHECK_ARGUMENT(sig, 1, NSData *);
    CHECK_ARGUMENT(sig, 2, NSData *);
    CHECK_ARGUMENT(sig, 3, NSData *);
    CHECK_ARGUMENT(sig, 4, uint16_t);
    CHECK_ARGUMENT(sig, 5, NSData *);

    sig = [MTRThreadOperationalDataset instanceMethodSignatureForSelector:@selector(initWithData:)];
    CHECK_RETURN_TYPE(sig, MTRThreadOperationalDataset *);
    CHECK_ARGUMENT(sig, 0, NSData *);

    sig = [MTRThreadOperationalDataset instanceMethodSignatureForSelector:@selector(data)];
    CHECK_RETURN_TYPE(sig, NSData *);
}

- (void)test047_MTRGroupKeyManagementClusterKeySetReadAllIndicesParams
{
    __auto_type * params = [[MTRGroupKeyManagementClusterKeySetReadAllIndicesParams alloc] init];
    CHECK_PROPERTY(params, groupKeySetIDs, setGroupKeySetIDs, NSArray *);

    params.groupKeySetIDs = @[ @(16) ];
    XCTAssertEqualObjects(params.groupKeySetIDs, @[ @(16) ]);
}

- (void)test048_MTRModeSelectClusterSemanticTagStruct
{
    __auto_type * obj = [[MTRModeSelectClusterSemanticTagStruct alloc] init];
    CHECK_PROPERTY(obj, mfgCode, setMfgCode, NSNumber *);
    CHECK_PROPERTY(obj, value, setValue, NSNumber *);
}

@end
