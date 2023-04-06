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

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static const uint16_t kTimeoutWithUpdateInSeconds = 10;
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
        XCTFail(@"Unexpected attempt to query for an image");
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
        XCTFail(@"Unexpected attempt to apply an update");
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
        XCTFail(@"Unexpected update application");
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
        XCTFail(@"Unexpected attempt to begin BDX transfer");
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
        XCTFail(@"Unexpected attempt to get BDX block");
        completion(nil, YES);
    }
}

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                       error:(NSError * _Nullable)error
{
    if (self.transferEndHandler) {
        self.transferEndHandler(nodeID, controller, error);
    } else {
        XCTFail(@"Unexpected end of BDX transfer");
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

- (void)respondAvailableWithDelay:(NSNumber *)delay
                              uri:(NSString *)uri
                      updateToken:(NSData *)updateToken
                       completion:(QueryImageCompletion)completion
{
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderOTAQueryStatusUpdateAvailable);
    responseParams.delayedActionTime = delay;
    responseParams.imageURI = uri;
    // TODO: Figure out whether we need better
    // SoftwareVersion/SoftwareVersionString/UpdateToken bits.
    responseParams.softwareVersion = @(18);
    responseParams.softwareVersionString = @"18";
    responseParams.updateToken = updateToken;
    completion(responseParams, nil);
}

- (void)respondWithErrorToApplyUpdateRequestWithCompletion:(ApplyUpdateRequestCompletion)completion
{
    [self respondErrorWithCompletion:^(NSError * _Nullable error) {
        completion(nil, error);
    }];
}

- (void)respondWithDiscontinueToApplyUpdateRequestWithCompletion:(ApplyUpdateRequestCompletion)completion
{
    __auto_type * params = [[MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];
    params.action = @(MTROTASoftwareUpdateProviderOTAApplyUpdateActionDiscontinue);
    params.delayedActionTime = @(0);
    completion(params, nil);
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

- (NSData *)generateUpdateToken
{
    const size_t dataSize = 16;
    const size_t randomBytesAtOnce = sizeof(uint32_t);
    XCTAssertEqual(dataSize % randomBytesAtOnce, 0);
    NSMutableData * data = [NSMutableData dataWithCapacity:16];
    for (unsigned i = 0; i < dataSize / randomBytesAtOnce; ++i) {
        uint32_t randomBytes = arc4random();
        [data appendBytes:&randomBytes length:randomBytesAtOnce];
    }
    return [NSData dataWithData:data];
}

@end

static MTROTAProviderDelegateImpl * sOTAProviderDelegate;

@interface MTROTAProviderTests : XCTestCase
@end

static BOOL sStackInitRan = NO;
static BOOL sNeedsStackShutdown = YES;

@implementation MTROTAProviderTests

+ (void)tearDown
{
    // Global teardown, runs once
    if (sNeedsStackShutdown) {
        // We don't need to worry about ResetCommissionee.  If we get here,
        // we're running only one of our test methods (using
        // -only-testing:MatterTests/MTROTAProviderTests/testMethodName), since
        // we did not run test999_TearDown.
        [self shutdownStack];
    }
}

- (void)setUp
{
    // Per-test setup, runs before each test.
    [super setUp];
    [self setContinueAfterFailure:NO];

    if (sStackInitRan == NO) {
        [self initStack];
    }

    XCTAssertNil(sOTAProviderDelegate.queryImageHandler);
    XCTAssertNil(sOTAProviderDelegate.applyUpdateRequestHandler);
    XCTAssertNil(sOTAProviderDelegate.notifyUpdateAppliedHandler);
    XCTAssertNil(sOTAProviderDelegate.transferBeginHandler);
    XCTAssertNil(sOTAProviderDelegate.blockQueryHandler);
    XCTAssertNil(sOTAProviderDelegate.transferEndHandler);
}

- (void)tearDown
{
    // Per-test teardown, runs after each test.
    [super tearDown];

    // Reset all handlers so tests don't interfere with each other.
    sOTAProviderDelegate.queryImageHandler = nil;
    sOTAProviderDelegate.applyUpdateRequestHandler = nil;
    sOTAProviderDelegate.notifyUpdateAppliedHandler = nil;
    sOTAProviderDelegate.transferBeginHandler = nil;
    sOTAProviderDelegate.blockQueryHandler = nil;
    sOTAProviderDelegate.transferEndHandler = nil;
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
    sStackInitRan = YES;

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

+ (void)shutdownStack
{
    sNeedsStackShutdown = NO;

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

- (void)test000_SetUp
{
    // Nothing to do here; our setUp method handled this already.  This test
    // just exists to make the setup not look like it's happening inside other
    // tests.
}

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
    // Test that if we advertise ourselves as a provider we end up getting a
    // QueryImage callbacks that we can respond to.
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
    // Test that if we advertise ourselves as a provider and respond BUSY to
    // QueryImage callback, then we get a second QueryImage callback later on
    // that we can then respond to however we wish.
    __auto_type * device = sConnectedDevice1;

    XCTestExpectation * queryExpectation1 = [self expectationWithDescription:@"handleQueryImageForNodeID called first time"];
    XCTestExpectation * queryExpectation2 = [self expectationWithDescription:@"handleQueryImageForNodeID called second time"];
    const uint16_t busyDelay = 1; // 1 second

    __block QueryImageHandler handleSecondQuery;
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        sOTAProviderDelegate.queryImageHandler = handleSecondQuery;
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondBusyWithDelay:@(busyDelay) completion:completion];
        [queryExpectation1 fulfill];
    };

    handleSecondQuery = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondNotAvailableWithCompletion:completion];
        [queryExpectation2 fulfill];
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our queries in order.  Give it a bit more time, because
    // there will be a delay between the two queries.
    [self waitForExpectations:@[ queryExpectation1, queryExpectation2 ] timeout:(kTimeoutInSeconds + busyDelay) enforceOrder:YES];

    [self waitForExpectations:@[ announceResponseExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test003_ReceiveSecondQueryWhileHandlingBDX
{
    // In this test we do the following:
    //
    // 1) Advertise ourselves to device1.
    // 2) When device1 queries for an image, claim to have one.
    // 3) When device1 tries to start a bdx transfer, stall it and advertise to device2.
    // 4) When device2 queries for an image, claim to have one. Since we are
    //    in the middle of doing BDX with device1, this actually responds with Busy.
    // 5) Error out of the device1 transfer.
    // 6) Wait for device2 to query us again.
    __auto_type * device1 = sConnectedDevice1;
    __auto_type * device2 = sConnectedDevice2;

    __block XCTestExpectation * announceResponseExpectation2;
    XCTestExpectation * queryExpectation1 = [self expectationWithDescription:@"handleQueryImageForNodeID called first time"];
    XCTestExpectation * queryExpectation2 = [self expectationWithDescription:@"handleQueryImageForNodeID called second time"];
    XCTestExpectation * queryExpectation3 = [self expectationWithDescription:@"handleQueryImageForNodeID called third time"];

    const uint16_t busyDelay = 1; // 1 second
    NSString * fakeImageURI = @"No such image, really";

    __block QueryImageHandler handleThirdQuery;
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        [sOTAProviderDelegate respondAvailableWithDelay:@(0)
                                                    uri:fakeImageURI
                                            updateToken:[sOTAProviderDelegate generateUpdateToken]
                                             completion:completion];
        [queryExpectation1 fulfill];
    };
    sOTAProviderDelegate.transferBeginHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSString * fileDesignator,
        NSNumber * offset, MTRStatusCompletion outerCompletion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);

        // Don't actually respond until the second requestor has queried us for
        // an image.  We need to reset queryImageHandler here, so we can close
        // over outerCompletion.
        sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
            MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion innerCompletion) {
            sOTAProviderDelegate.queryImageHandler = handleThirdQuery;
            sOTAProviderDelegate.transferBeginHandler = nil;

            XCTAssertEqualObjects(nodeID, @(kDeviceId2));
            XCTAssertEqual(controller, sController);

            // We respond UpdateAvailable, but since we are in the middle of
            // handling OTA for device1 we expect the requestor to get Busy and
            // try again.
            [sOTAProviderDelegate respondAvailableWithDelay:@(busyDelay)
                                                        uri:fakeImageURI
                                                updateToken:[sOTAProviderDelegate generateUpdateToken]
                                                 completion:innerCompletion];
            [sOTAProviderDelegate respondErrorWithCompletion:outerCompletion];
            [queryExpectation2 fulfill];
        };

        announceResponseExpectation2 = [self announceProviderToDevice:device2];
    };

    handleThirdQuery = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId2));
        XCTAssertEqual(controller, sController);

        [sOTAProviderDelegate respondNotAvailableWithCompletion:completion];
        [queryExpectation3 fulfill];
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation1 = [self announceProviderToDevice:device1];

    // Make sure we get our queries in order.  Give it a bit more time, because
    // there will be a delay between the two queries.
    [self waitForExpectations:@[ queryExpectation1, queryExpectation2, queryExpectation3 ]
                      timeout:(kTimeoutInSeconds + busyDelay * 3)
                 enforceOrder:YES];

    [self waitForExpectations:@[ announceResponseExpectation1, announceResponseExpectation2 ] timeout:kTimeoutInSeconds];
}

- (void)test004_DoBDXTransferDenyUpdateRequest
{
    // In this test we do the following:
    //
    // 1) Create an actual image we can send to the device, with a valid header
    //    but garbage data.
    // 2) Advertise ourselves to device.
    // 3) When device queries for an image, claim to have one.
    // 4) When device tries to start a bdx transfer, respond with success.
    // 5) Send the data as the BDX transfer proceeds.
    // 6) When device invokes ApplyUpdateRequest, respond with Discontinue so
    //    that the update does not actually proceed.
    __auto_type * device = sConnectedDevice1;

    XCTestExpectation * queryExpectation = [self expectationWithDescription:@"handleQueryImageForNodeID called"];
    XCTestExpectation * bdxBeginExpectation = [self expectationWithDescription:@"handleBDXTransferSessionBeginForNodeID called"];
    XCTestExpectation * bdxQueryExpectation = [self expectationWithDescription:@"handleBDXQueryForNodeID called"];
    XCTestExpectation * bdxEndExpectation = [self expectationWithDescription:@"handleBDXTransferSessionEndForNodeID called"];
    XCTestExpectation * applyUpdateRequestExpectation =
        [self expectationWithDescription:@"handleApplyUpdateRequestForNodeID called"];

    NSData * updateToken = [sOTAProviderDelegate generateUpdateToken];

    // First, create an image.  Make it at least 4096 bytes long, so we get
    // multiple BDX blocks going.
    const size_t rawImageSize = 4112;
    NSData * rawImagePiece = [@"1234567890abcdef" dataUsingEncoding:NSUTF8StringEncoding];
    XCTAssertEqual(rawImageSize % rawImagePiece.length, 0);
    NSMutableData * fakeImage = [NSMutableData dataWithCapacity:rawImageSize];
    while (fakeImage.length < rawImageSize) {
        [fakeImage appendData:rawImagePiece];
    }
    NSString * rawImagePath = @"/tmp/test004-raw-image";
    NSString * imagePath = @"/tmp/test004-image";

    [[NSFileManager defaultManager] createFileAtPath:rawImagePath contents:fakeImage attributes:nil];

    // Find the right absolute path to our ota_image_tool.py script.  PWD should
    // point to our src/darwin/Framework, while the script is in
    // src/app/ota_image_tool.py.
    NSString * pwd = [[NSProcessInfo processInfo] environment][@"PWD"];
    NSString * imageToolPath = [NSString
        pathWithComponents:@[ [pwd substringToIndex:(pwd.length - @"darwin/Framework".length)], @"app", @"ota_image_tool.py" ]];

    NSTask * task = [[NSTask alloc] init];
    [task setLaunchPath:imageToolPath];
    [task setArguments:@[
        @"create", @"-v", @"0xFFF1", @"-p", @"0x8001", @"-vn", @"2", @"-vs", @"2.0", @"-da", @"sha256", rawImagePath, imagePath
    ]];
    NSError * launchError = nil;
    [task launchAndReturnError:&launchError];
    XCTAssertNil(launchError);
    [task waitUntilExit];
    XCTAssertEqual([task terminationStatus], 0);

    __block NSFileHandle * readHandle;
    __block uint64_t imageSize;
    __block uint32_t lastBlockIndex = UINT32_MAX;

    // TODO: Maybe we should move more of this logic into sOTAProviderDelegate
    // or some other helper, once we have multiple tests sending images?  For
    // example, we could have something where you can do one of two things:
    //
    // 1) register a "raw image" with it, and it generates the
    //    image-with header.
    // 2) register a pre-generated image with it and it uses "ota_image_tool.py
    //    extract" to extract the raw image.
    //
    // Once that's done the helper could track the transfer state for a
    // particular image, etc, with us just forwarding our notifications to it.
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);

        sOTAProviderDelegate.queryImageHandler = nil;
        [sOTAProviderDelegate respondAvailableWithDelay:@(0) uri:imagePath updateToken:updateToken completion:completion];
        [queryExpectation fulfill];
    };
    sOTAProviderDelegate.transferBeginHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSString * fileDesignator,
        NSNumber * offset, MTRStatusCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(fileDesignator, imagePath);
        XCTAssertEqualObjects(offset, @(0));

        readHandle = [NSFileHandle fileHandleForReadingAtPath:fileDesignator];
        XCTAssertNotNil(readHandle);

        NSError * endSeekError;
        XCTAssertTrue([readHandle seekToEndReturningOffset:&imageSize error:&endSeekError]);
        XCTAssertNil(endSeekError);

        sOTAProviderDelegate.transferBeginHandler = nil;
        [sOTAProviderDelegate respondSuccess:completion];
        [bdxBeginExpectation fulfill];
    };
    sOTAProviderDelegate.blockQueryHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSNumber * blockSize,
        NSNumber * blockIndex, NSNumber * bytesToSkip, BlockQueryCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(blockSize, @(1024)); // Seems to always be 1024.
        XCTAssertEqualObjects(blockIndex, @(lastBlockIndex + 1));
        XCTAssertEqualObjects(bytesToSkip, @(0)); // Don't expect to see skips here.
        // Make sure we actually end up with multiple blocks.
        XCTAssertTrue(blockSize.unsignedLongLongValue < rawImageSize);

        XCTAssertNotNil(readHandle);
        uint64_t offset = blockSize.unsignedLongLongValue * blockIndex.unsignedLongLongValue;
        NSError * seekError = nil;
        [readHandle seekToOffset:offset error:&seekError];
        XCTAssertNil(seekError);

        NSError * readError = nil;
        NSData * data = [readHandle readDataUpToLength:blockSize.unsignedLongValue error:&readError];
        XCTAssertNil(readError);
        XCTAssertNotNil(data);

        BOOL isEOF = offset + blockSize.unsignedLongValue >= imageSize;

        ++lastBlockIndex;

        if (isEOF) {
            sOTAProviderDelegate.blockQueryHandler = nil;
        }

        completion(data, isEOF);

        if (isEOF) {
            [bdxQueryExpectation fulfill];
        }
    };
    sOTAProviderDelegate.transferEndHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSError * _Nullable error) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertNil(error);

        sOTAProviderDelegate.transferEndHandler = nil;
        [bdxEndExpectation fulfill];
    };
    sOTAProviderDelegate.applyUpdateRequestHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * params, ApplyUpdateRequestCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(params.updateToken, updateToken);
        XCTAssertEqualObjects(params.newVersion, @(18)); // TODO: Factor this out better!

        XCTAssertTrue([[NSFileManager defaultManager] contentsEqualAtPath:rawImagePath
                                                                  andPath:@"/tmp/chip-ota-requestor-downloaded-image1"]);

        sOTAProviderDelegate.applyUpdateRequestHandler = nil;
        [sOTAProviderDelegate respondWithDiscontinueToApplyUpdateRequestWithCompletion:completion];
        [applyUpdateRequestExpectation fulfill];
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our callbacks in order.  Give it a bit more time, because
    // we want to allow time for the BDX download.
    [self waitForExpectations:@[ queryExpectation, bdxBeginExpectation, bdxQueryExpectation ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other, and nothing
    // defines the ordering of announceResponseExpectation with respect to _any_
    // of the above expectations.
    [self waitForExpectations:@[ bdxEndExpectation, applyUpdateRequestExpectation, announceResponseExpectation ]
                      timeout:kTimeoutInSeconds];
}

- (void)test999_TearDown
{
    __auto_type * device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId1) controller:sController];
    ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);

    device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId2) controller:sController];
    ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);
    [[self class] shutdownStack];
}

@end
