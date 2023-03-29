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
static const uint64_t kDeviceId = 0x12341234;
// NOTE: This onboarding payload is for the chip-ota-requestor-app, not chip-all-clusters-app
static NSString * kOnboardingPayload = @"MT:-24J0SO527K10648G00";

static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kOTAProviderEndpointId = 0;

static MTRDevice * sConnectedDevice;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

@interface MTROTAProviderTestControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTROTAProviderTestControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    XCTAssertEqual(error.code, 0);

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId)
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

@interface MTROTAProviderDelegateImpl : NSObject <MTROTAProviderDelegate>
@property (nonatomic) XCTestExpectation * handleQueryImageExpectation;
@end

@implementation MTROTAProviderDelegateImpl
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion
{
    XCTAssertEqualObjects(nodeID, @(kDeviceId));
    XCTAssertEqual(controller, sController);
    // TODO: Anything we can test here about the params?

    // TODO: Make it possible to configure our responses.
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderOTAQueryStatusNotAvailable);
    completion(responseParams, nil);

    if (self.handleQueryImageExpectation != nil) {
        [self.handleQueryImageExpectation fulfill];
    }
}

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
{
    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
}

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
{
    completion([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
}

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                                    completion:(MTRStatusCompletion)completion
{
    completion([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
}

- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
                     completion:(void (^)(NSData * _Nullable data, BOOL isEOF))completion
{
    completion(nil, YES);
}

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                       error:(NSError * _Nullable)error
{
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

    XCTestExpectation * expectation = [self expectationWithDescription:@"Commissioning Complete"];
    __auto_type * deviceControllerDelegate = [[MTROTAProviderTestControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];

    sConnectedDevice = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:controller];
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

- (void)test001_ReceiveOTAQuery
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
#endif

    __auto_type * device = sConnectedDevice;
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * queryExpectation = [self expectationWithDescription:@"handleQueryImageForNodeID called"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"AnnounceOTAProvider succeeded"];
    sOTAProviderDelegate.handleQueryImageExpectation = queryExpectation;

    // Advertise ourselves as an OTA provider.
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

    [self waitForExpectations:@[ queryExpectation, responseExpectation ] timeout:kTimeoutInSeconds];
    sOTAProviderDelegate.handleQueryImageExpectation = nil;
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    __auto_type * device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);
    [self shutdownStack];
}
#endif

@end
