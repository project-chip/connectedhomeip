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

#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#define ENABLE_OTA_TESTS 0
#else
#define ENABLE_OTA_TESTS 1
#endif

// TODO: Disable test005_DoBDXTransferAllowUpdateRequest,
// test006_DoBDXTransferWithTwoOTARequesters and
// test007_DoBDXTransferIncrementalOtaUpdate until PR #26040 is merged.
// Currently the poll interval causes delays in the BDX transfer and
// results in the test taking a long time.
#ifdef ENABLE_REAL_OTA_UPDATE_TESTS
#undef ENABLE_REAL_OTA_UPDATE_TESTS
#endif

#if ENABLE_OTA_TESTS

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static const uint16_t kTimeoutWithUpdateInSeconds = 60;
static const uint64_t kDeviceId1 = 0x12341234;
static const uint64_t kDeviceId2 = 0x12341235;
#ifdef ENABLE_REAL_OTA_UPDATE_TESTS
static const uint64_t kDeviceId3 = 0x12341236;
#endif // ENABLE_REAL_OTA_UPDATE_TESTS
// NOTE: These onboarding payloads are for the chip-ota-requestor-app, not chip-all-clusters-app
static NSString * kOnboardingPayload1 = @"MT:-24J0SO527K10648G00"; // Discriminator: 1111
static NSString * kOnboardingPayload2 = @"MT:-24J0AFN00L10648G00"; // Discriminator: 1112
#ifdef ENABLE_REAL_OTA_UPDATE_TESTS
static NSString * kOnboardingPayload3 = @"MT:-24J0IRV01L10648G00"; // Discriminator: 1113
#endif // ENABLE_REAL_OTA_UPDATE_TESTS

static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kOTAProviderEndpointId = 0;

static MTRDeviceController * sController = nil;

static NSNumber * kUpdatedSoftwareVersion_5 = @5;

static NSString * kUpdatedSoftwareVersionString_5 = @"5.0";

static NSNumber * kUpdatedSoftwareVersion_10 = @10;

static NSString * kUpdatedSoftwareVersionString_10 = @"10.0";

// kOtaRequestorBasePort gets the discriminator added to it to figure out the
// port the ota-requestor app should be using.  This ensures that apps with
// distinct discriminators use distinct ports.
static const uint16_t kOtaRequestorBasePort = 5542 - 1111;

@class MTROTARequestorAppRunner;

@interface MTROTAProviderTests : XCTestCase
- (NSTask *)createTaskForPath:(NSString *)path;
- (NSString *)createImageFromRawImage:(NSString *)rawImage withVersion:(NSNumber *)version;
- (MTRDevice *)commissionDeviceWithPayload:(NSString *)payloadString nodeID:(NSNumber *)nodeID;
- (void)registerRunningRequestor:(MTROTARequestorAppRunner *)requestor;
@end

static unsigned sAppRunnerIndex = 1;

@interface MTROTARequestorAppRunner : NSObject
@property (nonatomic, copy) NSString * downloadFilePath;

- (instancetype)initWithPayload:(NSString *)payload testcase:(MTROTAProviderTests *)testcase;
- (MTRDevice *)commissionWithNodeID:(NSNumber *)nodeID;
@end

@implementation MTROTARequestorAppRunner {
    unsigned _uniqueIndex;
    NSTask * _appTask;
    MTROTAProviderTests * _testcase;
    NSString * _payload;
    MTRDevice * commissionedDevice;
}

- (MTRDevice *)commissionWithNodeID:(NSNumber *)nodeID
{
    return [_testcase commissionDeviceWithPayload:_payload nodeID:nodeID];
}

- (instancetype)initWithPayload:(NSString *)payload testcase:(MTROTAProviderTests *)testcase
{
    if (!(self = [super init])) {
        return nil;
    }

    _uniqueIndex = sAppRunnerIndex++;
    _testcase = testcase;
    _payload = payload;
    _downloadFilePath = [NSString stringWithFormat:@"/tmp/chip-ota-requestor-downloaded-image%u", _uniqueIndex];

    NSError * error;
    __auto_type * parsedPayload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payload error:&error];
    XCTAssertNotNil(parsedPayload);
    XCTAssertNil(error);

    XCTAssertFalse(parsedPayload.hasShortDiscriminator);

    __auto_type * discriminator = parsedPayload.discriminator;

    _appTask = [testcase createTaskForPath:@"out/debug/ota-requestor-app/chip-ota-requestor-app"];

    __auto_type * arguments = @[
        @"--interface-id",
        @"-1",
        @"--secured-device-port",
        [NSString stringWithFormat:@"%u", kOtaRequestorBasePort + discriminator.unsignedShortValue],
        @"--discriminator",
        [NSString stringWithFormat:@"%u", discriminator.unsignedShortValue],
        @"--KVS",
        [NSString stringWithFormat:@"/tmp/chip-ota-requestor-kvs%u", _uniqueIndex],
        @"--otaDownloadPath",
        _downloadFilePath,
        @"--autoApplyImage",
    ];

    [_appTask setArguments:arguments];

    NSString * outFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/ota-requestor-app-%u.log", _uniqueIndex];
    NSString * errorFile = [NSString stringWithFormat:@"/tmp/darwin/framework-tests/ota-requestor-app-err-%u.log", _uniqueIndex];

    // Make sure the files exist.
    [[NSFileManager defaultManager] createFileAtPath:outFile contents:nil attributes:nil];
    [[NSFileManager defaultManager] createFileAtPath:errorFile contents:nil attributes:nil];

    _appTask.standardOutput = [NSFileHandle fileHandleForWritingAtPath:outFile];
    _appTask.standardError = [NSFileHandle fileHandleForWritingAtPath:errorFile];

    [_appTask launchAndReturnError:&error];
    XCTAssertNil(error);

    NSLog(@"Started requestor with arguments %@ stdout=%@ and stderr=%@", arguments, outFile, errorFile);

    [_testcase registerRunningRequestor:self];

    return self;
}

- (void)terminate
{
    [_appTask terminate];
}

@end

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
    responseParams.status = @(MTROTASoftwareUpdateProviderStatusNotAvailable);
    completion(responseParams, nil);
}

- (void)respondBusyWithDelay:(NSNumber *)delay completion:(QueryImageCompletion)completion
{
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderStatusBusy);
    responseParams.delayedActionTime = delay;
    completion(responseParams, nil);
}

- (void)respondAvailableWithDelay:(NSNumber *)delay
                              uri:(NSString *)uri
                      updateToken:(NSData *)updateToken
                  softwareVersion:(NSNumber *)softwareVersion
            softwareVersionString:(NSString *)softwareVersionString
                       completion:(QueryImageCompletion)completion
{
    __auto_type * responseParams = [[MTROTASoftwareUpdateProviderClusterQueryImageResponseParams alloc] init];
    responseParams.status = @(MTROTASoftwareUpdateProviderStatusUpdateAvailable);
    responseParams.delayedActionTime = delay;
    responseParams.imageURI = uri;
    // TODO: Figure out whether we need better
    // SoftwareVersion/SoftwareVersionString/UpdateToken bits.
    responseParams.softwareVersion = softwareVersion;
    responseParams.softwareVersionString = softwareVersionString;
    responseParams.updateToken = updateToken;
    completion(responseParams, nil);
}

- (void)respondWithErrorToApplyUpdateRequestWithCompletion:(ApplyUpdateRequestCompletion)completion
{
    [self respondErrorWithCompletion:^(NSError * _Nullable error) {
        completion(nil, error);
    }];
}

- (void)respondToApplyUpdateRequestWithAction:(MTROTASoftwareUpdateProviderApplyUpdateAction)action
                                   completion:(ApplyUpdateRequestCompletion)completion
{
    __auto_type * params = [[MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams alloc] init];
    params.action = @(action);
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

/**
 * Helper that, given a raw image, creates an image-with-header from it, drives
 * a BDX transfer, etc.
 */
@interface MTROTAProviderTransferChecker : NSObject

- (instancetype)initWithRawImagePath:(NSString *)rawImagePath
            otaImageDownloadFilePath:(NSString *)otaImageDownloadFilePath
                              nodeID:(NSNumber *)nodeID
                     softwareVersion:(NSNumber *)softwareVersion
               softwareVersionString:(NSString *)softwareVersionString
                   applyUpdateAction:(MTROTASoftwareUpdateProviderApplyUpdateAction)applyUpdateAction
                            testcase:(XCTestCase *)testcase;

@property (nonatomic, readonly) XCTestExpectation * queryExpectation;
@property (nonatomic, readonly) XCTestExpectation * bdxBeginExpectation;
@property (nonatomic, readonly) XCTestExpectation * bdxQueryExpectation;
@property (nonatomic, readonly) XCTestExpectation * bdxEndExpectation;
@property (nonatomic, readonly) XCTestExpectation * applyUpdateRequestExpectation;
@property (nonatomic, readonly) XCTestExpectation * notifyUpdateAppliedExpectation;
@end

@implementation MTROTAProviderTransferChecker

- (instancetype)initWithRawImagePath:(NSString *)rawImagePath
            otaImageDownloadFilePath:(NSString *)otaImageDownloadFilePath
                              nodeID:(NSNumber *)nodeID
                     softwareVersion:(NSNumber *)softwareVersion
               softwareVersionString:(NSString *)softwareVersionString
                   applyUpdateAction:(MTROTASoftwareUpdateProviderApplyUpdateAction)applyUpdateAction
                            testcase:(MTROTAProviderTests *)testcase
{
    if (!(self = [super init])) {
        return nil;
    }

    _queryExpectation = [testcase expectationWithDescription:@"handleQueryImageForNodeID called"];
    _bdxBeginExpectation = [testcase expectationWithDescription:@"handleBDXTransferSessionBeginForNodeID called"];
    _bdxQueryExpectation = [testcase expectationWithDescription:@"handleBDXQueryForNodeID called"];
    _bdxEndExpectation = [testcase expectationWithDescription:@"handleBDXTransferSessionEndForNodeID called"];
    _applyUpdateRequestExpectation = [testcase expectationWithDescription:@"handleApplyUpdateRequestForNodeID called"];
    _notifyUpdateAppliedExpectation = [testcase expectationWithDescription:@"handleNotifyUpdateAppliedForNodeID called"];

    NSString * imagePath = [testcase createImageFromRawImage:rawImagePath withVersion:softwareVersion];

    NSData * updateToken = [sOTAProviderDelegate generateUpdateToken];

    __block NSFileHandle * readHandle;
    __block uint64_t imageSize;
    __block uint32_t lastBlockIndex = UINT32_MAX;

    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, nodeID);
        XCTAssertEqual(controller, sController);

        sOTAProviderDelegate.queryImageHandler = nil;
        [sOTAProviderDelegate respondAvailableWithDelay:@(0)
                                                    uri:imagePath
                                            updateToken:updateToken
                                        softwareVersion:softwareVersion
                                  softwareVersionString:softwareVersionString
                                             completion:completion];
        [self.queryExpectation fulfill];
    };
    sOTAProviderDelegate.transferBeginHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSString * fileDesignator,
        NSNumber * offset, MTRStatusCompletion completion) {
        XCTAssertEqualObjects(nodeID, nodeID);
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
        [self.bdxBeginExpectation fulfill];
    };
    sOTAProviderDelegate.blockQueryHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSNumber * blockSize,
        NSNumber * blockIndex, NSNumber * bytesToSkip, BlockQueryCompletion completion) {
        XCTAssertEqualObjects(nodeID, nodeID);
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(blockSize, @(1024)); // Seems to always be 1024.
        XCTAssertEqualObjects(blockIndex, @(lastBlockIndex + 1));
        XCTAssertEqualObjects(bytesToSkip, @(0)); // Don't expect to see skips here.
        // Make sure we actually end up with multiple blocks.
        XCTAssertTrue(blockSize.unsignedLongLongValue < imageSize);

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
            [self.bdxQueryExpectation fulfill];
        }
    };
    sOTAProviderDelegate.transferEndHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSError * _Nullable error) {
        XCTAssertEqualObjects(nodeID, nodeID);
        XCTAssertEqual(controller, sController);
        XCTAssertNil(error);

        sOTAProviderDelegate.transferEndHandler = nil;
        [self.bdxEndExpectation fulfill];
    };
    sOTAProviderDelegate.applyUpdateRequestHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * params, ApplyUpdateRequestCompletion completion) {
        XCTAssertEqualObjects(nodeID, nodeID);
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(params.updateToken, updateToken);
        XCTAssertEqualObjects(params.newVersion, softwareVersion);

        XCTAssertTrue([[NSFileManager defaultManager] contentsEqualAtPath:rawImagePath andPath:otaImageDownloadFilePath]);

        sOTAProviderDelegate.applyUpdateRequestHandler = nil;
        [sOTAProviderDelegate respondToApplyUpdateRequestWithAction:applyUpdateAction completion:completion];
        [self.applyUpdateRequestExpectation fulfill];
    };

    if (applyUpdateAction == MTROTASoftwareUpdateProviderApplyUpdateActionProceed) {
        sOTAProviderDelegate.notifyUpdateAppliedHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
            MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams * params, MTRStatusCompletion completion) {
            XCTAssertEqualObjects(nodeID, nodeID);
            XCTAssertEqual(controller, sController);
            XCTAssertEqualObjects(params.updateToken, updateToken);
            XCTAssertEqualObjects(params.softwareVersion, softwareVersion);

            sOTAProviderDelegate.notifyUpdateAppliedHandler = nil;
            [sOTAProviderDelegate respondSuccess:completion];
            [self.notifyUpdateAppliedExpectation fulfill];
        };
    }

    return self;
}
@end

static BOOL sStackInitRan = NO;
static BOOL sNeedsStackShutdown = YES;

@implementation MTROTAProviderTests {
    NSMutableSet<NSNumber *> * _commissionedNodeIDs;
    NSMutableSet<MTROTARequestorAppRunner *> * _runningRequestors;
}

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

    _commissionedNodeIDs = [[NSMutableSet alloc] init];
    _runningRequestors = [[NSMutableSet alloc] init];

    XCTAssertNil(sOTAProviderDelegate.queryImageHandler);
    XCTAssertNil(sOTAProviderDelegate.applyUpdateRequestHandler);
    XCTAssertNil(sOTAProviderDelegate.notifyUpdateAppliedHandler);
    XCTAssertNil(sOTAProviderDelegate.transferBeginHandler);
    XCTAssertNil(sOTAProviderDelegate.blockQueryHandler);
    XCTAssertNil(sOTAProviderDelegate.transferEndHandler);

    // Start a new controller for each test, with a new fabric.  Otherwise
    // reusing the same node id for our commissionee devices will cause us to
    // try to reuse sessions in ways that fail.
    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);

    MTRDeviceController * controller = [[MTRDeviceControllerFactory sharedInstance] createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    sController = controller;
}

- (void)tearDown
{
    for (NSNumber * nodeID in _commissionedNodeIDs) {
        __auto_type * device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sController];
        ResetCommissionee(device, dispatch_get_main_queue(), self, kTimeoutInSeconds);
    }

    for (MTROTARequestorAppRunner * runner in _runningRequestors) {
        [runner terminate];
    }
    // Break cycle.
    _runningRequestors = nil;

    if (sController != nil) {
        [sController shutdown];
        XCTAssertFalse([sController isRunning]);
        sController = nil;
    }

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

    [_commissionedNodeIDs addObject:nodeID];

    return [MTRDevice deviceWithNodeID:nodeID controller:sController];
}

- (void)registerRunningRequestor:(MTROTARequestorAppRunner *)requestor
{
    [_runningRequestors addObject:requestor];
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
}

+ (void)shutdownStack
{
    sNeedsStackShutdown = NO;

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

/**
 * Given a path relative to the Matter root, create an absolute path to the file.
 */
- (NSString *)absolutePathFor:(NSString *)matterRootRelativePath
{
    // Find the right absolute path to our file.  PWD should
    // point to our src/darwin/Framework.
    NSString * pwd = [[NSProcessInfo processInfo] environment][@"PWD"];
    NSMutableArray * pathComponents = [[NSMutableArray alloc] init];
    [pathComponents addObject:[pwd substringToIndex:(pwd.length - @"src/darwin/Framework".length)]];
    [pathComponents addObjectsFromArray:[matterRootRelativePath pathComponents]];
    return [NSString pathWithComponents:pathComponents];
}

/**
 * Create a task given a path relative to the Matter root.
 */
- (NSTask *)createTaskForPath:(NSString *)path
{
    NSTask * task = [[NSTask alloc] init];
    [task setLaunchPath:[self absolutePathFor:path]];
    return task;
}

/**
 * Runs a task to completion and makes sure it succeeds.
 */
- (void)runTask:(NSTask *)task
{
    NSError * launchError;
    [task launchAndReturnError:&launchError];
    XCTAssertNil(launchError);

    [task waitUntilExit];
    XCTAssertEqual([task terminationStatus], 0);
}

/**
 * Returns path to the raw image.
 */
- (NSString *)createRawImageWithVersion:(NSNumber *)version
{
    NSTask * buildTask = [self createTaskForPath:@"scripts/examples/gn_build_example.sh"];
    NSString * objdir =
        [self absolutePathFor:[NSString stringWithFormat:@"out/debug/ota-requestor-app-v%u", version.unsignedIntValue]];
    [buildTask setArguments:@[
        [self absolutePathFor:@"examples/ota-requestor-app/linux"],
        objdir,
        @"chip_config_network_layer_ble=false",
        @"non_spec_compliant_ota_action_delay_floor=0",
        [NSString stringWithFormat:@"chip_device_config_device_software_version=%u", version.unsignedIntValue],
        [NSString stringWithFormat:@"chip_device_config_device_software_version_string=\"%u.0\"", version.unsignedIntValue],
    ]];

    [self runTask:buildTask];

    NSString * sourcePath = [NSString pathWithComponents:@[ objdir, @"chip-ota-requestor-app" ]];
    NSString * destPath = [NSString stringWithFormat:@"/tmp/ota-raw-image-v%u", version.unsignedIntValue];

    // We don't care about error on remove; the file might not be there.  But if
    // it _is_ there, we have to remove, or the copy will fail.
    [[NSFileManager defaultManager] removeItemAtPath:destPath error:nil];

    NSError * copyError;
    BOOL ok = [[NSFileManager defaultManager] copyItemAtPath:sourcePath toPath:destPath error:&copyError];
    XCTAssertNil(copyError);
    XCTAssertTrue(ok);

    return destPath;
}

/**
 * Returns path to the created image.
 */
- (NSString *)createImageFromRawImage:(NSString *)rawImage withVersion:(NSNumber *)version
{
    NSString * image = [rawImage stringByReplacingOccurrencesOfString:@"raw-image" withString:@"image"];

    NSTask * task = [self createTaskForPath:@"src/app/ota_image_tool.py"];
    [task setArguments:@[
        @"create", @"-v", @"0xFFF1", @"-p", @"0x8001", @"-vn", version.stringValue, @"-vs",
        [NSString stringWithFormat:@"%.1f", version.floatValue], @"-da", @"sha256", rawImage, image
    ]];

    [self runTask:task];

    return image;
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
    params.announcementReason = @(MTROTASoftwareUpdateRequestorAnnouncementReasonSimpleAnnouncement);
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

- (void)test001_ReceiveQueryImageRequest_RespondUpdateNotAvailable
{
    // Test that if we advertise ourselves as a provider we end up getting a
    // QueryImage callbacks that we can respond to.
    __auto_type * runner = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device = [runner commissionWithNodeID:@(kDeviceId1)];

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

- (void)test002_ReceiveTwoQueryImageRequests_RespondExplicitBusy
{
    // Test that if we advertise ourselves as a provider and respond BUSY to
    // QueryImage callback, then we get a second QueryImage callback later on
    // that we can then respond to however we wish.
    __auto_type * runner = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device = [runner commissionWithNodeID:@(kDeviceId1)];

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

- (void)test003_ReceiveQueryImageRequestWhileHandlingBDX_RespondImplicitBusy
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
    __auto_type * runner1 = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device1 = [runner1 commissionWithNodeID:@(kDeviceId1)];

    __auto_type * runner2 = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload2 testcase:self];
    __auto_type * device2 = [runner2 commissionWithNodeID:@(kDeviceId2)];

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
                                        softwareVersion:kUpdatedSoftwareVersion_5
                                  softwareVersionString:kUpdatedSoftwareVersionString_5
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
                                            softwareVersion:kUpdatedSoftwareVersion_5
                                      softwareVersionString:kUpdatedSoftwareVersionString_5
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
    __auto_type * runner = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device = [runner commissionWithNodeID:@(kDeviceId1)];

    // First, create an image.  Make it at least 4096 bytes long, so we get
    // multiple BDX blocks going.
    const size_t rawImageSize = 4112;
    NSData * rawImagePiece = [@"1234567890abcdef" dataUsingEncoding:NSUTF8StringEncoding];
    XCTAssertEqual(rawImageSize % rawImagePiece.length, 0);
    NSMutableData * fakeImage = [NSMutableData dataWithCapacity:rawImageSize];
    while (fakeImage.length < rawImageSize) {
        [fakeImage appendData:rawImagePiece];
    }
    NSString * rawImagePath = @"/tmp/ota-test004-raw-image";

    [[NSFileManager defaultManager] createFileAtPath:rawImagePath contents:fakeImage attributes:nil];

    __auto_type * checker =
        [[MTROTAProviderTransferChecker alloc] initWithRawImagePath:rawImagePath
                                           otaImageDownloadFilePath:runner.downloadFilePath
                                                             nodeID:@(kDeviceId1)
                                                    softwareVersion:kUpdatedSoftwareVersion_5
                                              softwareVersionString:kUpdatedSoftwareVersionString_5
                                                  applyUpdateAction:MTROTASoftwareUpdateProviderApplyUpdateActionDiscontinue
                                                           testcase:self];
    // We do not expect the update to actually be applied here.
    checker.notifyUpdateAppliedExpectation.inverted = YES;

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our callbacks in order.  Give it a bit more time, because
    // we want to allow time for the BDX download.
    [self waitForExpectations:@[ checker.queryExpectation, checker.bdxBeginExpectation, checker.bdxQueryExpectation ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other, and nothing
    // defines the ordering of announceResponseExpectation with respect to _any_
    // of the above expectations.
    [self waitForExpectations:@[ checker.bdxEndExpectation, checker.applyUpdateRequestExpectation, announceResponseExpectation ]
                      timeout:kTimeoutInSeconds];

    // We are not expecting checker.notifyUpdateAppliedExpectation to actually
    // be called fulfilled, but we still need to wait on it here.  Since we set
    // inverted = YES, on it, this is basically a no-op, except for making
    // XCTest not complain about unwaited expectations.
    [self waitForExpectations:@[ checker.notifyUpdateAppliedExpectation ] timeout:kTimeoutInSeconds];
}

// TODO: Enable tests 005, 006 and 007 when PR #26040 is merged. Currently the poll interval causes delays in the BDX transfer
// and results in the tests taking a long time. With PR #26040 we eliminate the poll interval completely and hence the tests can run
// in a short time.
#ifdef ENABLE_REAL_OTA_UPDATE_TESTS
- (void)test005_DoBDXTransferAllowUpdateRequest
{
    // In this test we do the following:
    //
    // 1) Check if the ota image file and raw image file required for this test exist.
    // 2) Advertise ourselves to device.
    // 3) When device queries for an image, pass the image path for the ota file generated beforehand as a pre-requisite
    // 4) When device tries to start a bdx transfer, respond with success.
    // 5) Send the data as the BDX transfer proceeds.
    // 6) Confirm the downloaded ota image matches the raw image file that was generated before the test was run as a pre-requisite
    // 7) When device invokes ApplyUpdateRequest, respond with Proceed so that the update proceeds
    // 8) Wait for the app to restart and wait for the NotifyUpdateApplied message to confirm the app has updated to the new version

    NSString * otaRawImagePath = [self createRawImageWithVersion:kUpdatedSoftwareVersion_5];

    // Check whether the ota raw image exists at otaRawImagePath
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:otaRawImagePath]);

    __auto_type * runner = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device = [runner commissionWithNodeID:@(kDeviceId1)];

    __auto_type * checker =
        [[MTROTAProviderTransferChecker alloc] initWithRawImagePath:otaRawImagePath
                                           otaImageDownloadFilePath:runner.downloadFilePath
                                                             nodeID:@(kDeviceId1)
                                                    softwareVersion:kUpdatedSoftwareVersion_5
                                              softwareVersionString:kUpdatedSoftwareVersionString_5
                                                  applyUpdateAction:MTROTASoftwareUpdateProviderApplyUpdateActionProceed
                                                           testcase:self];

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our callbacks in order.  Give it a bit more time, because
    // we want to allow time for the BDX download.
    [self waitForExpectations:@[
        checker.queryExpectation, checker.bdxBeginExpectation, checker.bdxQueryExpectation, checker.bdxEndExpectation
    ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other.
    [self waitForExpectations:@[ checker.applyUpdateRequestExpectation, checker.notifyUpdateAppliedExpectation ]
                      timeout:kTimeoutInSeconds
                 enforceOrder:YES];

    // Nothing defines the ordering of announceResponseExpectation with respect
    // to _any_ of the above expectations.
    [self waitForExpectations:@[ announceResponseExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test006_DoBDXTransferWithTwoOTARequesters
{
    // In this test, we test BDX transfers between one provider and two OTA requestors device1 and device2.
    //
    // 1) We announce ourselves to device1 first.
    // 2) When device1 queries, we return image available at imagePath1.
    // 3) We set the BDX begin and end handlers for device1 and device1 starts the BDX transfer.
    // 4) At this time, we announce ourselves to device2.
    // 5) When device2 queries, we return image available with a busy delay of 30 secs.
    // 6) When device1 completes the BDX transfer, we wait for device2 to query.
    // 7) When device 2 queries again, we return image available at imagePath2.
    // 8) We set the BDX begin and end handlers for device2 and device2 starts the BDX transfer.
    // 9) Device1 applies the update independently and reboots with the new image.
    // 10) At this point, we set the apply update handlers for device2.
    // 11) Device2 applies the update and reboots with its new image.

    NSString * otaRawImagePath1 = [self createRawImageWithVersion:kUpdatedSoftwareVersion_10];
    NSString * otaRawImagePath2 = [self createRawImageWithVersion:kUpdatedSoftwareVersion_5];

    // Check whether the ota raw image exists at otaRawImagePath1 and otaRawImagePath2
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:otaRawImagePath1]);
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:otaRawImagePath2]);

    XCTestExpectation * queryExpectation1 = [self expectationWithDescription:@"handleQueryImageForNodeID called for device1"];
    XCTestExpectation * queryExpectation2 =
        [self expectationWithDescription:@"handleQueryImageForNodeID called for device2 - busy"];
    XCTestExpectation * queryExpectation3 =
        [self expectationWithDescription:@"handleQueryImageForNodeID called for device2 - available"];

    XCTestExpectation * bdxBeginExpectation1 =
        [self expectationWithDescription:@"handleBDXTransferSessionBeginForNodeID called for device1"];
    XCTestExpectation * bdxBeginExpectation2 =
        [self expectationWithDescription:@"handleBDXTransferSessionBeginForNodeID called for device2"];

    XCTestExpectation * bdxQueryExpectation1 = [self expectationWithDescription:@"handleBDXQueryForNodeID called for device1"];
    XCTestExpectation * bdxQueryExpectation2 = [self expectationWithDescription:@"handleBDXQueryForNodeID called for device2"];

    XCTestExpectation * bdxEndExpectation1 =
        [self expectationWithDescription:@"handleBDXTransferSessionEndForNodeID called for device1"];
    XCTestExpectation * bdxEndExpectation2 =
        [self expectationWithDescription:@"handleBDXTransferSessionEndForNodeID called for device2"];

    XCTestExpectation * applyUpdateRequestExpectation1 =
        [self expectationWithDescription:@"handleApplyUpdateRequestForNodeID called for device1"];
    XCTestExpectation * applyUpdateRequestExpectation2 =
        [self expectationWithDescription:@"handleApplyUpdateRequestForNodeID called for device2"];

    XCTestExpectation * notifyUpdateAppliedExpectation1 =
        [self expectationWithDescription:@"handleNotifyUpdateAppliedForNodeID called for device1"];
    XCTestExpectation * notifyUpdateAppliedExpectation2 =
        [self expectationWithDescription:@"handleNotifyUpdateAppliedForNodeID called for device2"];

    __block XCTestExpectation * announceResponseExpectation2;

    NSString * imagePath1 = [self createImageFromRawImage:otaRawImagePath1 withVersion:kUpdatedSoftwareVersion_10];
    NSString * imagePath2 = [self createImageFromRawImage:otaRawImagePath2 withVersion:kUpdatedSoftwareVersion_5];

    NSData * updateToken1 = [sOTAProviderDelegate generateUpdateToken];
    NSData * updateToken2 = [sOTAProviderDelegate generateUpdateToken];

    __block NSFileHandle * readHandle;
    __block uint64_t imageSize;
    __block uint32_t lastBlockIndex = UINT32_MAX;
    const uint16_t busyDelay = 30; // 30 second

    __auto_type * runner1 = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload1 testcase:self];
    __auto_type * device1 = [runner1 commissionWithNodeID:@(kDeviceId1)];

    __auto_type * runner2 = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload2 testcase:self];
    __auto_type * device2 = [runner2 commissionWithNodeID:@(kDeviceId2)];

    // This to keep track of whether queryImageHandler for device 2 was called or not. The first time it's called we will
    // fulfill queryExpectation2 and proceed with BDX for device 1.
    __block bool firstQueryImageForDevice2Received = false;

    // Set up the query handler for device 1 to return image available at imagePath1
    sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);

        sOTAProviderDelegate.queryImageHandler = nil;
        [sOTAProviderDelegate respondAvailableWithDelay:@(0)
                                                    uri:imagePath1
                                            updateToken:updateToken1
                                        softwareVersion:kUpdatedSoftwareVersion_10
                                  softwareVersionString:kUpdatedSoftwareVersionString_10
                                             completion:completion];
        [queryExpectation1 fulfill];
    };

    // Set up the BDX transfer begin, block query and transfer end handlers for device1 so it can go ahead with the BDX transfer
    sOTAProviderDelegate.transferBeginHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSString * fileDesignator,
        NSNumber * offset, MTRStatusCompletion outerCompletion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(fileDesignator, imagePath1);
        XCTAssertEqualObjects(offset, @(0));

        readHandle = [NSFileHandle fileHandleForReadingAtPath:fileDesignator];
        XCTAssertNotNil(readHandle);

        NSError * endSeekError;
        XCTAssertTrue([readHandle seekToEndReturningOffset:&imageSize error:&endSeekError]);
        XCTAssertNil(endSeekError);
        sOTAProviderDelegate.transferBeginHandler = nil;

        // Don't actually respond until the second requestor has queried us for
        // an image to ensure BDX for device1 starts only after device2 has queried us once.

        // Set up the query handler for device2 to return image available at imagePath2 but a busy delay of 30 secs
        sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
            MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion innerCompletion) {
            XCTAssertEqualObjects(nodeID, @(kDeviceId2));
            XCTAssertEqual(controller, sController);

            [sOTAProviderDelegate respondAvailableWithDelay:@(busyDelay)
                                                        uri:imagePath2
                                                updateToken:updateToken2
                                            softwareVersion:kUpdatedSoftwareVersion_5
                                      softwareVersionString:kUpdatedSoftwareVersionString_5
                                                 completion:innerCompletion];
            if (!firstQueryImageForDevice2Received) {
                [queryExpectation2 fulfill];

                // Respond with success for the tranfer begin completion for device1
                [sOTAProviderDelegate respondSuccess:outerCompletion];
                [bdxBeginExpectation1 fulfill];
            }
            firstQueryImageForDevice2Received = true;
        };

        // Announce ourselves to device2
        announceResponseExpectation2 = [self announceProviderToDevice:device2];
    };
    sOTAProviderDelegate.blockQueryHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSNumber * blockSize,
        NSNumber * blockIndex, NSNumber * bytesToSkip, BlockQueryCompletion completion) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(blockSize, @(1024)); // Seems to always be 1024.
        XCTAssertEqualObjects(blockIndex, @(lastBlockIndex + 1));
        XCTAssertEqualObjects(bytesToSkip, @(0)); // Don't expect to see skips here.
        // Make sure we actually end up with multiple blocks.
        XCTAssertTrue(blockSize.unsignedLongLongValue < imageSize);

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
            [bdxQueryExpectation1 fulfill];
        }
    };
    sOTAProviderDelegate.transferEndHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSError * _Nullable error) {
        XCTAssertEqualObjects(nodeID, @(kDeviceId1));
        XCTAssertEqual(controller, sController);
        XCTAssertNil(error);
        sOTAProviderDelegate.transferEndHandler = nil;
        [bdxEndExpectation1 fulfill];

        // BDX transfer with device1 has completed
        // Set up the query handler for device2 to return image available at imagePath2
        sOTAProviderDelegate.queryImageHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
            MTROTASoftwareUpdateProviderClusterQueryImageParams * params, QueryImageCompletion completion) {
            XCTAssertEqualObjects(nodeID, @(kDeviceId2));
            XCTAssertEqual(controller, sController);

            sOTAProviderDelegate.queryImageHandler = nil;
            [sOTAProviderDelegate respondAvailableWithDelay:@(0)
                                                        uri:imagePath2
                                                updateToken:updateToken2
                                            softwareVersion:kUpdatedSoftwareVersion_5
                                      softwareVersionString:kUpdatedSoftwareVersionString_5
                                                 completion:completion];
            NSLog(@"OTA queryExpectation3");
            [queryExpectation3 fulfill];
        };

        // Set up the BDX transfer begin, block query and transfer end handlers for device2 so it can go ahead with the BDX transfer
        sOTAProviderDelegate.transferBeginHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
            NSString * fileDesignator, NSNumber * offset, MTRStatusCompletion completion) {
            XCTAssertEqualObjects(nodeID, @(kDeviceId2));
            XCTAssertEqual(controller, sController);
            XCTAssertEqualObjects(fileDesignator, imagePath2);
            XCTAssertEqualObjects(offset, @(0));

            readHandle = [NSFileHandle fileHandleForReadingAtPath:fileDesignator];
            XCTAssertNotNil(readHandle);

            NSError * endSeekError;
            XCTAssertTrue([readHandle seekToEndReturningOffset:&imageSize error:&endSeekError]);
            XCTAssertNil(endSeekError);
            sOTAProviderDelegate.transferBeginHandler = nil;
            [sOTAProviderDelegate respondSuccess:completion];
            lastBlockIndex = UINT32_MAX;
            [bdxBeginExpectation2 fulfill];
        };
        sOTAProviderDelegate.blockQueryHandler = ^(NSNumber * nodeID, MTRDeviceController * controller, NSNumber * blockSize,
            NSNumber * blockIndex, NSNumber * bytesToSkip, BlockQueryCompletion completion) {
            XCTAssertEqualObjects(nodeID, @(kDeviceId2));
            XCTAssertEqual(controller, sController);
            XCTAssertEqualObjects(blockSize, @(1024)); // Seems to always be 1024.
            XCTAssertEqualObjects(blockIndex, @(lastBlockIndex + 1));
            XCTAssertEqualObjects(bytesToSkip, @(0)); // Don't expect to see skips here.
            // Make sure we actually end up with multiple blocks.
            XCTAssertTrue(blockSize.unsignedLongLongValue < imageSize);

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
                [bdxQueryExpectation2 fulfill];
            }
        };
        sOTAProviderDelegate.transferEndHandler
            = ^(NSNumber * nodeID, MTRDeviceController * controller, NSError * _Nullable error) {
                  XCTAssertEqualObjects(nodeID, @(kDeviceId2));
                  XCTAssertEqual(controller, sController);
                  XCTAssertNil(error);
                  sOTAProviderDelegate.transferEndHandler = nil;
                  [bdxEndExpectation2 fulfill];
              };
    };

    // Flags to track if the applyUpdateRequest from both device1 and device2 has been handled.
    __block bool device1HasHandledApplyUpdateRequest = false;
    __block bool device2HasHandledApplyUpdateRequest = false;

    // Flags to track if device1 and device have notified that the update has been applied.
    __block bool device1HasNotifiedUpdateApplied = false;
    __block bool device2HasNotifiedUpdateApplied = false;

    // Set up the apply update request and update applied handlers for device1 and device2. Use the nodeID to verify which
    // device sent the applyUpdateRequest and subsequent notifyUpdateApplied commands.
    sOTAProviderDelegate.applyUpdateRequestHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * params, ApplyUpdateRequestCompletion completion) {
        XCTAssertTrue([nodeID isEqualToNumber:@(kDeviceId1)] || [nodeID isEqualToNumber:@(kDeviceId2)]);

        bool isDeviceID1 = [nodeID isEqualToNumber:@(kDeviceId1)];
        NSData * updateToken = (isDeviceID1) ? updateToken1 : updateToken2;

        // Device1 is updated to version 10 and device2 to version 5.
        NSNumber * kSoftwareVersion = (isDeviceID1) ? kUpdatedSoftwareVersion_10 : kUpdatedSoftwareVersion_5;
        NSString * otaImageFilePath = (isDeviceID1) ? otaRawImagePath1 : otaRawImagePath2;
        NSString * otaDownloadedFilePath = (isDeviceID1) ? runner1.downloadFilePath : runner2.downloadFilePath;

        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(params.updateToken, updateToken);
        XCTAssertEqualObjects(params.newVersion, kSoftwareVersion);

        XCTAssertTrue([[NSFileManager defaultManager] contentsEqualAtPath:otaImageFilePath andPath:otaDownloadedFilePath]);

        [sOTAProviderDelegate respondToApplyUpdateRequestWithAction:MTROTASoftwareUpdateProviderApplyUpdateActionProceed
                                                         completion:completion];

        if (isDeviceID1) {
            device1HasHandledApplyUpdateRequest = true;
            [applyUpdateRequestExpectation1 fulfill];
        } else {
            device2HasHandledApplyUpdateRequest = true;
            [applyUpdateRequestExpectation2 fulfill];
        }

        // If the applyUpdateRequest from both device1 and device2 has been handled, reset the
        // applyUpdateRequestHandler to nil
        if (device1HasHandledApplyUpdateRequest && device2HasHandledApplyUpdateRequest) {
            sOTAProviderDelegate.applyUpdateRequestHandler = nil;
        }
    };

    sOTAProviderDelegate.notifyUpdateAppliedHandler = ^(NSNumber * nodeID, MTRDeviceController * controller,
        MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams * params, MTRStatusCompletion completion) {
        XCTAssertTrue([nodeID isEqualToNumber:@(kDeviceId1)] || [nodeID isEqualToNumber:@(kDeviceId2)]);

        bool isDeviceID1 = [nodeID isEqualToNumber:@(kDeviceId1)];
        NSData * updateToken = (isDeviceID1) ? updateToken1 : updateToken2;

        // Device1 is updated to version 10 and device2 to version 5.
        NSNumber * kSoftwareVersion = (isDeviceID1) ? kUpdatedSoftwareVersion_10 : kUpdatedSoftwareVersion_5;

        XCTAssertEqual(controller, sController);
        XCTAssertEqualObjects(params.updateToken, updateToken);
        XCTAssertEqualObjects(params.softwareVersion, kSoftwareVersion);

        [sOTAProviderDelegate respondSuccess:completion];

        if (isDeviceID1) {
            device1HasNotifiedUpdateApplied = true;
            [notifyUpdateAppliedExpectation1 fulfill];
        } else {
            device2HasNotifiedUpdateApplied = true;
            [notifyUpdateAppliedExpectation2 fulfill];
        }

        // If both device1 and device2 have notified that the update has been applied, reset the
        // notifyUpdateAppliedHandler to nil
        if (device1HasNotifiedUpdateApplied && device2HasNotifiedUpdateApplied) {
            sOTAProviderDelegate.notifyUpdateAppliedHandler = nil;
        }
    };

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation1 = [self announceProviderToDevice:device1];

    // Make sure we get our callbacks in order for both device1 and device2.  Since we do not
    // send image available to device2 until BDX for device1 has ended, queryExpectation3 must follow
    // bdxEndExpectation1.
    //
    // Give it a bit more time, because we want to allow time for the BDX downloads.
    [self waitForExpectations:@[
        queryExpectation1, bdxBeginExpectation1, bdxQueryExpectation1, bdxEndExpectation1, queryExpectation3, bdxBeginExpectation2,
        bdxQueryExpectation2, bdxEndExpectation2
    ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Make sure we get our query expectation for device2 fulfilled when we send busy for device2.
    [self waitForExpectations:@[ queryExpectation2 ] timeout:(kTimeoutInSeconds)];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other.
    [self waitForExpectations:@[
        applyUpdateRequestExpectation1, notifyUpdateAppliedExpectation1, applyUpdateRequestExpectation2,
        notifyUpdateAppliedExpectation2
    ]
                      timeout:kTimeoutInSeconds
                 enforceOrder:NO];

    // Nothing defines the ordering of announceResponseExpectation with respect
    // to _any_ of the above expectations.
    [self waitForExpectations:@[ announceResponseExpectation1, announceResponseExpectation2 ] timeout:kTimeoutInSeconds];
}

- (void)test007_DoBDXTransferIncrementalOtaUpdate
{
    // In this test, we test incremental OTA update with device3. First we update device3
    // to version 5 using image at imagePath1. Once device3 updates and reboots, we update
    // it to version 10 using image at imagePath2.
    //
    // 1) We announce ourselves to device3.
    // 2) When device3 queries, we return image available at imagePath1 with version number 5.
    // 3) We set the BDX begin and end handlers for device3 and device3 starts the BDX transfer.
    // 4) Device3 completes the BDX transfer
    // 5) Device3 applies the update and reboots with the new image with version number 5
    // 6) We announce ourselves to device 3 again.
    // 7) When device3 queries again, we return image available at imagePath2 with version number 10.
    // 8) We set the BDX begin and end handlers for device3 and device3 starts the BDX transfer.
    // 4) Device3 completes the BDX transfer
    // 5) Device3 applies the update and reboots with the new image with version number 10

    NSString * otaRawImagePath1 = [self createRawImageWithVersion:kUpdatedSoftwareVersion_5];
    NSString * otaRawImagePath2 = [self createRawImageWithVersion:kUpdatedSoftwareVersion_10];

    // Check whether the ota raw image exists at otaRawImagePath1 and otaRawImagePath2
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:otaRawImagePath1]);
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:otaRawImagePath2]);

    __auto_type * runner = [[MTROTARequestorAppRunner alloc] initWithPayload:kOnboardingPayload3 testcase:self];
    __auto_type * device = [runner commissionWithNodeID:@(kDeviceId3)];

    __auto_type * checker =
        [[MTROTAProviderTransferChecker alloc] initWithRawImagePath:otaRawImagePath1
                                           otaImageDownloadFilePath:runner.downloadFilePath
                                                             nodeID:@(kDeviceId3)
                                                    softwareVersion:kUpdatedSoftwareVersion_5
                                              softwareVersionString:kUpdatedSoftwareVersionString_5
                                                  applyUpdateAction:MTROTASoftwareUpdateProviderApplyUpdateActionProceed
                                                           testcase:self];

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation = [self announceProviderToDevice:device];

    // Make sure we get our callbacks in order.  Give it a bit more time, because
    // we want to allow time for the BDX download.
    [self waitForExpectations:@[
        checker.queryExpectation, checker.bdxBeginExpectation, checker.bdxQueryExpectation, checker.bdxEndExpectation
    ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other.
    [self waitForExpectations:@[ checker.applyUpdateRequestExpectation, checker.notifyUpdateAppliedExpectation ]
                      timeout:kTimeoutInSeconds
                 enforceOrder:YES];

    // Nothing defines the ordering of announceResponseExpectation with respect
    // to _any_ of the above expectations.
    [self waitForExpectations:@[ announceResponseExpectation ] timeout:kTimeoutInSeconds];

    // Provide an incremental update and makes sure the app is updated to the new version

    __auto_type * checker1 =
        [[MTROTAProviderTransferChecker alloc] initWithRawImagePath:otaRawImagePath2
                                           otaImageDownloadFilePath:runner.downloadFilePath
                                                             nodeID:@(kDeviceId3)
                                                    softwareVersion:kUpdatedSoftwareVersion_10
                                              softwareVersionString:kUpdatedSoftwareVersionString_10
                                                  applyUpdateAction:MTROTASoftwareUpdateProviderApplyUpdateActionProceed
                                                           testcase:self];

    // Advertise ourselves as an OTA provider.
    XCTestExpectation * announceResponseExpectation1 = [self announceProviderToDevice:device];

    // Make sure we get our callbacks in order.  Give it a bit more time, because
    // we want to allow time for the BDX download.
    [self waitForExpectations:@[
        checker1.queryExpectation, checker1.bdxBeginExpectation, checker1.bdxQueryExpectation, checker1.bdxEndExpectation
    ]
                      timeout:(kTimeoutWithUpdateInSeconds) enforceOrder:YES];

    // Nothing really defines the ordering of bdxEndExpectation and
    // applyUpdateRequestExpectation with respect to each other.
    [self waitForExpectations:@[ checker1.applyUpdateRequestExpectation, checker1.notifyUpdateAppliedExpectation ]
                      timeout:kTimeoutInSeconds
                 enforceOrder:YES];

    // Nothing defines the ordering of announceResponseExpectation with respect
    // to _any_ of the above expectations.
    [self waitForExpectations:@[ announceResponseExpectation1 ] timeout:kTimeoutInSeconds];
}
#endif // ENABLE_REAL_OTA_UPDATE_TESTS

- (void)test999_TearDown
{
    [[self class] shutdownStack];
}

@end

#else // ENABLE_OTA_TESTS

@interface MTROTAProviderTests : XCTestCase
@end

@implementation MTROTAProviderTests
@end

#endif // ENABLE_OTA_TESTS
