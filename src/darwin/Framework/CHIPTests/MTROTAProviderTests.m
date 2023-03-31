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

// system dependencies
#import <XCTest/XCTest.h>

// Set the following to 1 in order to run individual test case manually.
#define MANUAL_INDIVIDUAL_TEST 0

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId1 = 0x12341234;
static const uint64_t kDeviceId2 = 0x12341235;
// NOTE: These onboarding payloads are for the chip-ota-requestor-app, not chip-all-clusters-app
static NSString * kOnboardingPayload1 = @"MT:-24J0SO527K10648G00"; // Discriminator: 1111
static NSString * kOnboardingPayload2 = @"MT:-24J0AFN00L10648G00"; // Discriminator: 1112

static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kOTAProviderEndpointId = 0;

static MTRDevice * sConnectedDevice1;
static MTRDevice * sConnectedDevice2;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

@interface MTROTAProviderTestControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, readonly) XCTestExpectation * expectation;
@property (nonatomic, readonly) NSNumber * commissioneeNodeID;
@end

@implementation MTROTAProviderTestControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation commissioneeNodeID:(NSNumber *)nodeID
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _commissioneeNodeID = nodeID;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    XCTAssertEqual(error.code, 0);

    NSError * commissionError = nil;
    [sController commissionNodeWithID:self.commissioneeNodeID
                  commissioningParams:[[MTRCommissioningParameters alloc] init]
                                error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for onCommissioningComplete
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

@end

typedef void (^QueryImageCompletion)(
    MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error);
typedef void (^ApplyUpdateRequestCompletion)(
    MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error);
typedef void (^BlockQueryCompletion)(NSData * _Nullable data, BOOL isEOF);

typedef void (^QueryImageHandler)(NSNumber * nodeID, MTRDeviceController * controller,
    MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion);
typedef void (^ApplyUpdateRequestHandler)(NSNumber * nodeID, MTRDeviceController * controller,
    MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * params, ApplyUpdateRequestCompletion completion);
typedef void (^NotifyUpdateAppliedHandler)(NSNumber * nodeID, MTRDeviceController * controller,
    MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams * params, MTRStatusCompletion completion);
typedef void (^BDXTransferBeginHandler)(NSNumber * nodeID, MTRDeviceController * controller, NSString * fileDesignator,
    NSNumber * offset, MTRStatusCompletion completion);
typedef void (^BDXQueryHandler)(NSNumber * nodeID, MTRDeviceController * controller, NSNumber * blockSize, NSNumber * blockIndex,
    NSNumber * bytesToSkip, BlockQueryCompletion completion);
typedef void (^BDXTransferEndHandler)(NSNumber * nodeID, MTRDeviceController * controller, NSError * _Nullable error);

@interface MTROTAProviderDelegateImpl : NSObject <MTROTAProviderDelegate>
@property (nonatomic, nullable) QueryImageHandler queryImageHandler;
@property (nonatomic, nullable) ApplyUpdateRequestHandler applyUpdateRequestHandler;
@property (nonatomic, nullable) NotifyUpdateAppliedHandler notifyUpdateAppliedHandler;
@property (nonatomic, nullable) BDXTransferBeginHandler transferBeginHandler;
@property (nonatomic, nullable) BDXQueryHandler blockQueryHandler;
@property (nonatomic, nullable) BDXTransferEndHandler transferEndHandler;
@end

@implementation MTROTAProviderDelegateImpl
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(QueryImageCompletion)completion
{
    XCTAssertEqual(controller, sController);

    if (self.queryImageHandler) {
        self.queryImageHandler(nodeID, controller, params, completion);
    } else {
        [self respondNotAvailableWithCompletion:completion];
    }
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(ApplyUpdateRequestCompletion)completion
{
    XCTAssertEqual(controller, sController);

    if (self.applyUpdateRequestHandler) {
        self.applyUpdateRequestHandler(nodeID, controller, params, completion);
    } else {
        [self respondWithErrorToApplyUpdateRequestWithCompletion:completion];
    }
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
{
    XCTAssertEqual(controller, sController);

    if (self.notifyUpdateAppliedHandler) {
        self.notifyUpdateAppliedHandler(nodeID, controller, params, completion);
    } else {
        [self respondErrorWithCompletion:completion];
    }
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                                    completion:(MTRStatusCompletion)completion
{
    XCTAssertEqual(controller, sController);

    if (self.transferBeginHandler) {
        self.transferBeginHandler(nodeID, controller, fileDesignator, offset, completion);
    } else {
        [self respondErrorWithCompletion:completion];
    }
}

- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
                     completion:(BlockQueryCompletion)completion
{
    XCTAssertEqual(controller, sController);

    if (self.blockQueryHandler) {
        self.blockQueryHandler(nodeID, controller, blockSize, blockIndex, bytesToSkip, completion);
    } else {
        completion(nil, YES);
    }
}

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                       error:(NSError * _Nullable)error
{
    if (self.transferEndHandler) {
        self.transferEndHandler(nodeID, controller, error);
    }
}

- (void)respondNotAvailableWithCompletion:(QueryImageCompletion)completion
{
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable);
    completion(responseParams, nil);
}

- (void)respondBusyWithDelay:(NSNumber *)delay completion:(QueryImageCompletion)completion
{
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderOTAQueryStatusBusy);
    responseParams.delayedActionTime = delay;
    completion(responseParams, nil);
}

- (void)respondWithErrorToApplyUpdateRequestWithCompletion:(ApplyUpdateRequestCompletion)completion
{
    [self respondErrorWithCompletion:^(NSError * _Nullable error) {
        completion(nil, error);
    }];
}

- (void)respondErrorWithCompletion:(MTRStatusCompletion)completion
{
    [self respondErrorWithCode:MTRErrorCodeGeneralError completion:completion];
}

- (void)respondErrorWithCode:(MTRErrorCode)code completion:(MTRStatusCompletion)completion
{
    [self respondError:[NSError errorWithDomain:MTRErrorDomain code:code userInfo:nil] completion:completion];
}

- (void)respondError:(NSError *)error completion:(MTRStatusCompletion)completion
{
    completion(error);
}

- (void)respondSuccess:(MTRStatusCompletion)completion
{
    completion(nil);
}

@end

static MTROTAProviderDelegateImpl * sOTAProviderDelegate;

@interface MTROTAProviderTests : XCTestCase
@end

@implementation MTROTAProviderTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)tearDown
{
#if MANUAL_INDIVIDUAL_TEST
    [self shutdownStack];
#endif
    [super tearDown];
}

- (MTRDevice *)commissionDeviceWithPayload:(NSString *)payloadString nodeID:(NSNumber *)nodeID
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:[NSString stringWithFormat:@"Commissioning Complete for %@", nodeID]];
    __auto_type * deviceControllerDelegate = [[MTROTAProviderTestControllerDelegate alloc] initWithExpectation:expectation
                                                                                            commissioneeNodeID:nodeID];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [sController setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payloadString error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [sController setupCommissioningSessionWithPayload:payload newNodeID:nodeID error:&error];
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];

    return [MTRDevice deviceWithNodeID:nodeID controller:sController];
}

- (void)initStack
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    sOTAProviderDelegate = [[MTROTAProviderDelegateImpl alloc] init];

    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);
    factoryParams.otaProviderDelegate = sOTAProviderDelegate;
    factoryParams.shouldStartServer = YES;

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    sConnectedDevice1 = [self commissionDeviceWithPayload:kOnboardingPayload1 nodeID:@(kDeviceId1)];
    sConnectedDevice2 = [self commissionDeviceWithPayload:kOnboardingPayload2 nodeID:@(kDeviceId2)];
}

- (void)shutdownStack
{
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test000_SetUp
{
    [self initStack];
}
#endif

- (XCTestExpectation *)announceProviderToDevice:(MTRDevice *)device
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * responseExpectation =
        [self expectationWithDescription:[NSString stringWithFormat:@"AnnounceOTAProvider to %@ succeeded", device]];

    __auto_type * params = [[MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams alloc] init];
    params.providerNodeID = [sController controllerNodeID];
    params.vendorID = @(kTestVendorId);
    params.announcementReason = @(MTROTASoftwareUpdateRequestorOTAAnnouncementReasonSimpleAnnouncement);
    params.endpoint = @(kOTAProviderEndpointId);

    __auto_type * cluster = [[MTRClusterOTASoftwareUpdateRequestor alloc] initWithDevice:device endpointID:@(0) queue:queue];
    [cluster announceOTAProviderWithParams:params
                            expectedValues:nil
                     expectedValueInterval:nil
                                completion:^(NSError * _Nullable error) {
                                    XCTAssertNil(error);
                                    [responseExpectation fulfill];
                                }];

    return responseExpectation;
}

- (void)test001_ReceiveOTAQuery
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
#endif

    __auto_type * device = sConnectedDevice1;

    XCTestExpectation * queryExpectation = [self expectationWithDescription:@"handleQueryImageForNodeID called"];
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondNotAvailableWithCompletion:completion];
        [queryExpectation fulfill];
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    [self waitForExpectations:@[ queryExpectation, announceResponseExpectation ] timeout:kTimeoutInSeconds];

    sOTAProviderDelegate.queryImageHandler = nil;
}

- (void)test002_ReceiveTwoQueriesExplicitBusy
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
#endif

    // TODO: This test fails so far because the OTA requestor is following the
    // spec and clamping the delay to a minimum of 120 seconds.  We don't really
    // want to spend 2 minutes waiting in this test, so just disable it for
    // now.  See https://github.com/project-chip/connectedhomeip/issues/25922
    // for a proposal to address this.
#if 0
    __auto_type * device = sConnectedDevice1;

    XCTestExpectation * queryExpectation1 = [self expectationWithDescription:@"handleQueryImageForNodeID called first time"];
    XCTestExpectation * queryExpectation2 = [self expectationWithDescription:@"handleQueryImageForNodeID called second time"];
    const uint16_t busyDelay = 1; // Second

    __block QueryImageHandler handleSecondQuery;
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, MTROTASoftwareUpdateProviderClusterQueryImageParams * params,
                                               QueryImageCompletion completion) {
        sOTAProviderDelegate.queryImageHandler = handleSecondQuery;
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondBusyWithDelay:@(busyDelay) completion:completion];
        [queryExpectation1 fulfill];
    };

    handleSecondQuery = ^(NSNumber * nodeID, MTRDeviceController * controller, MTROTASoftwareUpdateProviderClusterQueryImageParams * params,
                                               QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondNotAvailableWithCompletion:completion];
        [queryExpectation2 fulfill];
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our queries in order.  Give it a bit more time, because
    // there will be a delay between the two queries.
    [self waitForExpectations:@[ queryExpectation1, queryExpectation2 ] timeout:(kTimeoutInSeconds+busyDelay) enforceOrder:YES];

    [self waitForExpectations:@[ announceResponseExpectation ] timeout:kTimeoutInSeconds];

    sOTAProviderDelegate.queryImageHandler = nil;
#endif
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    __auto_type * device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId1) controller:sController];
    ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);

    device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId2) controller:sController];
    ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);
    [self shutdownStack];
}
#endif

@end
