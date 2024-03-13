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

// Fixture: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1 \
    --dac_provider credentials/development/commissioner_dut/struct_cd_origin_pid_vid_correct/test_case_vector.json \
    --product-id 32768 --discriminator 3839
// For manual testing, CASE retry code paths can be tested by adding --faults chip_CASEServerBusy_f1 (or similar)

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static uint64_t sDeviceId = 100000000;
static NSString * kOnboardingPayload = @"MT:Y.K90SO527JA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

// A no-op MTRDeviceAttestationDelegate which lets us test (by default, in CI)
// commissioning flows that have such a delegate.
@interface NoOpAttestationDelegate : NSObject <MTRDeviceAttestationDelegate>
@property (nonatomic) XCTestExpectation * expectation;

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation;
@end

@implementation NoOpAttestationDelegate
- (instancetype)initWithExpectation:(XCTestExpectation *)expectation
{
    if (!(self = [super init])) {
        return nil;
    }

    _expectation = expectation;
    return self;
}

- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
{
    [self.expectation fulfill];
    // Hard-coded to what our example server app uses for now.
    XCTAssertEqualObjects(attestationDeviceInfo.vendorID, @(0xFFF2));
    XCTAssertEqualObjects(attestationDeviceInfo.productID, @(0x8001));
    XCTAssertEqualObjects(attestationDeviceInfo.basicInformationVendorID, @(0xFFF1));
    XCTAssertEqualObjects(attestationDeviceInfo.basicInformationProductID, @(0x8000));
    [controller continueCommissioningDevice:opaqueDeviceHandle ignoreAttestationFailure:NO error:nil];
}

@end

@interface MTRPairingTestControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@property (nonatomic, nullable) id<MTRDeviceAttestationDelegate> attestationDelegate;
@property (nonatomic, nullable) NSNumber * failSafeExtension;
@property (nullable) NSError * commissioningCompleteError;
@end

@implementation MTRPairingTestControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
      attestationDelegate:(id<MTRDeviceAttestationDelegate>)attestationDelegate
        failSafeExtension:(NSNumber *)failSafeExtension
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _attestationDelegate = attestationDelegate;
        _failSafeExtension = failSafeExtension;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    XCTAssertNil(error);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.deviceAttestationDelegate = self.attestationDelegate;
    params.failSafeTimeout = self.failSafeExtension;

    NSError * commissionError = nil;
    XCTAssertTrue([controller commissionNodeWithID:@(sDeviceId) commissioningParams:params error:&commissionError],
        @"Failed to start commissioning for node ID %" PRIu64 ": %@", sDeviceId, commissionError);

    // Keep waiting for onCommissioningComplete
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError * _Nullable)error
{
    self.commissioningCompleteError = error;
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface MTRPairingTests : XCTestCase
@property (nullable) MTRPairingTestControllerDelegate * controllerDelegate;
@end

@implementation MTRPairingTests

+ (void)setUp
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);

    XCTAssertNotNil(sTestKeys = [[MTRTestKeys alloc] init]);

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:sTestKeys.ipk fabricID:@(1) nocSigner:sTestKeys];
    params.vendorID = @(kTestVendorId);
    XCTAssertNotNil(sController = [factory createControllerOnNewFabric:params error:nil]);
}

+ (void)tearDown
{
    [sController shutdown];
    XCTAssertFalse([sController isRunning]);
    sController = nil;

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)tearDown
{
    [sController setDeviceControllerDelegate:(id _Nonnull) nil queue:dispatch_get_main_queue()]; // TODO: do we need a clearDeviceControllerDelegate API?
    self.controllerDelegate = nil;
}

// attestationDelegate and failSafeExtension can both be nil
- (void)doPairingTestWithAttestationDelegate:(id<MTRDeviceAttestationDelegate>)attestationDelegate failSafeExtension:(NSNumber *)failSafeExtension
{
    // Don't reuse node ids, because that will confuse us.
    ++sDeviceId;
    XCTestExpectation * expectation = [self expectationWithDescription:@"Commissioning Complete"];

    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:expectation
                                                                                 attestationDelegate:attestationDelegate
                                                                                   failSafeExtension:failSafeExtension];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    [sController setDeviceControllerDelegate:controllerDelegate queue:callbackQueue];
    self.controllerDelegate = controllerDelegate;

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(sDeviceId) error:&error]);
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
    XCTAssertNil(controllerDelegate.commissioningCompleteError);

    ResetCommissionee([MTRBaseDevice deviceWithNodeID:@(sDeviceId) controller:sController], dispatch_get_main_queue(), self,
        kTimeoutInSeconds);
}

- (void)test001_PairWithoutAttestationDelegate
{
    [self doPairingTestWithAttestationDelegate:nil failSafeExtension:nil];
}

- (void)test002_PairWithAttestationDelegateNoFailsafeExtension
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Attestation delegate called"];

    [self doPairingTestWithAttestationDelegate:[[NoOpAttestationDelegate alloc] initWithExpectation:expectation]
                             failSafeExtension:nil];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test003_PairWithAttestationDelegateFailsafeExtensionShort
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Attestation delegate called"];

    // Extend by a time that is going to be smaller than the 60s default we
    // already have set via CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC on the
    // server side, minus whatever time that has likely passed.
    [self doPairingTestWithAttestationDelegate:[[NoOpAttestationDelegate alloc] initWithExpectation:expectation]
                             failSafeExtension:@(30)];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test004_PairWithAttestationDelegateFailsafeExtensionLong
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Attestation delegate called"];

    // Extend by a time that is going to be larger than the 60s default we
    // already have set via CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC on the
    // server side.
    [self doPairingTestWithAttestationDelegate:[[NoOpAttestationDelegate alloc] initWithExpectation:expectation]
                             failSafeExtension:@(90)];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)doPairingAndWaitForProgress:(NSString *)trigger
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Trigger message seen"];
    expectation.assertForOverFulfill = NO;
    MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if ([message containsString:trigger]) {
            [expectation fulfill];
        }
    });

    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:nil
                                                                                 attestationDelegate:nil
                                                                                   failSafeExtension:nil];
    [sController setDeviceControllerDelegate:controllerDelegate queue:dispatch_get_main_queue()];
    self.controllerDelegate = controllerDelegate;

    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:NULL];
    XCTAssertNotNil(payload);
    NSError * error;
    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(++sDeviceId) error:&error]);
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
    MTRSetLogCallback(0, nil);
}

- (void)doPairingTestAfterCancellationAtProgress:(NSString *)trigger
{
    // Run pairing up and wait for the trigger
    [self doPairingAndWaitForProgress:trigger];

    // Call StopPairing and wait for the commissioningComplete callback
    XCTestExpectation * expectation = [self expectationWithDescription:@"commissioningComplete delegate method called"];
    self.controllerDelegate.expectation = expectation;

    NSError * error;
    XCTAssertTrue([sController stopDevicePairing:sDeviceId error:&error], @"stopDevicePairing failed: %@", error);
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];

    // Validate that the completion correctly indicated cancellation
    error = self.controllerDelegate.commissioningCompleteError;
    XCTAssertEqualObjects(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeCancelled);

    // Now pair again. If the previous attempt was cancelled correctly this should work fine.
    [self doPairingTestWithAttestationDelegate:nil failSafeExtension:nil];
}

- (void)test005_pairingAfterCancellation_ReadCommissioningInfo
{
    // @"Sending read request for commissioning information"
    [self doPairingTestAfterCancellationAtProgress:@"Performing next commissioning step 'ReadCommissioningInfo'"];
}

- (void)test006_pairingAfterCancellation_ConfigRegulatoryCommand
{
    [self doPairingTestAfterCancellationAtProgress:@"Performing next commissioning step 'ConfigRegulatory'"];
}

- (void)test007_pairingAfterCancellation_FindOperational
{
    // Ensure CASE establishment has started by waiting for 'FindOrEstablishSession'
    [self doPairingTestAfterCancellationAtProgress:@"FindOrEstablishSession:"];
}

@end
