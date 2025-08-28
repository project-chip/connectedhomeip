/**
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

#import "MTRDefines_Internal.h"
#import "MTRErrorTestUtils.h"
#import "MTRSecureCodingTestHelpers.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const uint16_t kPairingTimeoutInSeconds = 30;
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

// The expectation will be fulfilled from deviceAttestationCompletedForController:...
- (instancetype)initWithExpectation:(nullable XCTestExpectation *)expectation;

// If blockCommissioning is YES, this delegate will never proceed from
// its attestation verification callback.
- (instancetype)initWithExpectation:(nullable XCTestExpectation *)expectation blockCommissioning:(BOOL)blockCommissioning;

// The callback will be called from deviceAttestationCompletedForController:...
- (instancetype)initWithCallback:(void (^_Nullable)(void))callback blockCommissioning:(BOOL)blockCommissioning;

@end

@implementation NoOpAttestationDelegate {
    void (^_Nullable _callback)(void);
    BOOL _blockCommissioning;
}

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation
{
    return [self initWithExpectation:expectation blockCommissioning:NO];
}

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation blockCommissioning:(BOOL)blockCommissioning;
{
    return [self initWithCallback:^{ [expectation fulfill]; } blockCommissioning:blockCommissioning];
}

- (instancetype)initWithCallback:(void (^)(void))callback blockCommissioning:(BOOL)blockCommissioning
{
    if (!(self = [super init])) {
        return nil;
    }

    _callback = callback;
    _blockCommissioning = blockCommissioning;
    return self;
}

- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
{
    // Hard-coded to what our example server app uses for now.
    XCTAssertEqualObjects(attestationDeviceInfo.vendorID, @(0xFFF2));
    XCTAssertEqualObjects(attestationDeviceInfo.productID, @(0x8001));
    XCTAssertEqualObjects(attestationDeviceInfo.basicInformationVendorID, @(0xFFF1));
    XCTAssertEqualObjects(attestationDeviceInfo.basicInformationProductID, @(0x8000));
    XCTAssertEqualObjects(attestationDeviceInfo.certificateDeclaration,
        attestationDeviceInfo.certificationDeclaration);
    XCTAssertNotNil(attestationDeviceInfo.certificationDeclaration);

    if (_callback) {
        _callback();
    }

    if (!_blockCommissioning) {
        [controller continueCommissioningDevice:opaqueDeviceHandle ignoreAttestationFailure:NO error:nil];
    }
}

@end

@interface MTRPairingTestControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@property (nonatomic, nullable) id<MTRDeviceAttestationDelegate> attestationDelegate;
@property (nonatomic, nullable) NSNumber * failSafeExtension;
@property (nonatomic) BOOL shouldReadEndpointInformation;
@property (nullable) NSArray<MTRAttributeRequestPath *> * extraAttributesToRead;
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
    params.readEndpointInformation = self.shouldReadEndpointInformation;
    params.extraAttributesToRead = self.extraAttributesToRead;

    NSError * commissionError = nil;
    XCTAssertTrue([controller commissionNodeWithID:@(sDeviceId) commissioningParams:params error:&commissionError],
        @"Failed to start commissioning for node ID %" PRIu64 ": %@", sDeviceId, commissionError);

    // Keep waiting for onCommissioningComplete
}

- (void)controller:(MTRDeviceController *)controller readCommissioneeInfo:(MTRCommissioneeInfo *)info
{
    XCTAssertNotNil(info.productIdentity);
    XCTAssertEqualObjects(info.productIdentity.vendorID, /* Test Vendor 1 */ @0xFFF1);

    NSError * decodeError;
    id roundTrippedInfo = RoundTripEncodable(info, &decodeError);
    XCTAssertNil(decodeError);
    XCTAssertEqualObjects(info, roundTrippedInfo);

    if (self.shouldReadEndpointInformation) {
        XCTAssertNotNil(info.endpointsById);
        XCTAssertNotNil(info.rootEndpoint);
        XCTAssertGreaterThanOrEqual(info.rootEndpoint.children.count, 1); // at least one application endpoint
        for (MTREndpointInfo * endpoint in info.endpointsById.allValues) {
            XCTAssertGreaterThanOrEqual(endpoint.deviceTypes.count, 1);
            XCTAssertNotNil(endpoint.children);
            XCTAssertNotNil(endpoint.partsList);
            XCTAssertGreaterThanOrEqual(endpoint.partsList.count, endpoint.children.count);
            for (MTREndpointInfo * child in endpoint.children) {
                XCTAssertTrue([endpoint.partsList containsObject:child.endpointID]);
            }
        }

        // There is currently no convenient way to initialize an MTRCommissioneeInfo
        // object from basic ObjC data types, so we do some unit testing here.
        NSError * err;
        NSData * data = [NSKeyedArchiver archivedDataWithRootObject:info requiringSecureCoding:YES error:&err];
        XCTAssertNil(err);
        MTRCommissioneeInfo * decoded = [NSKeyedUnarchiver unarchivedObjectOfClass:MTRCommissioneeInfo.class fromData:data error:&err];
        XCTAssertNil(err);
        XCTAssertNotNil(decoded);
        XCTAssertTrue([decoded isEqual:info]);
        XCTAssertEqualObjects(decoded.productIdentity, info.productIdentity);
        XCTAssertEqualObjects(decoded.endpointsById, info.endpointsById);
        XCTAssertEqualObjects(decoded.rootEndpoint.children, info.rootEndpoint.children);
    } else {
        XCTAssertNil(info.endpointsById);
        XCTAssertNil(info.rootEndpoint);
    }

    // In all cases, we should have the network commissioning feature maps in
    // the list.
    __auto_type isNetworkCommissioningFeatureMap = ^(MTRAttributePath * path) {
        return [path.cluster isEqual:@(MTRClusterIDTypeNetworkCommissioningID)] &&
            [path.attribute isEqual:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)];
    };
    NSUInteger networkCommissioningFeatureMapCount = 0;
    for (MTRAttributePath * path in info.attributes) {
        if (isNetworkCommissioningFeatureMap(path)) {
            ++networkCommissioningFeatureMapCount;
        }
    }
    XCTAssertGreaterThan(networkCommissioningFeatureMapCount, 0);

    if (self.extraAttributesToRead) {
        // The attributes we tried to read should really have worked.
        XCTAssertNotNil(info.attributes);
        XCTAssertEqual(info.attributes.count, 2 + networkCommissioningFeatureMapCount);
        for (MTRAttributePath * path in info.attributes) {
            if (isNetworkCommissioningFeatureMap(path)) {
                // We checked for these already.
                continue;
            }

            XCTAssertEqualObjects(path.endpoint, @(0));
            if ([path.cluster isEqual:@(MTRClusterIDTypeDescriptorID)]) {
                XCTAssertEqualObjects(path.attribute, @(MTRAttributeIDTypeGlobalAttributeAttributeListID));
            } else if ([path.cluster isEqual:@(MTRClusterIDTypeBasicInformationID)]) {
                XCTAssertEqualObjects(path.attribute, @(MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID));
            } else {
                XCTFail("Unexpected cluster id %@", path.cluster);
            }
        }
    }
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError * _Nullable)error
{
    self.commissioningCompleteError = error;
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface MTRPairingTestMonitoringControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (atomic, readwrite) BOOL statusUpdateCalled;
@property (atomic, readwrite) BOOL commissioningSessionEstablishmentDoneCalled;
@property (atomic, readwrite) BOOL commissioningCompleteCalled;
@property (atomic, readwrite) BOOL readCommissioneeInfoCalled;
@property (atomic, readwrite, strong) XCTestExpectation * allCallbacksCalledExpectation;
@end

@implementation MTRPairingTestMonitoringControllerDelegate
- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRPairingTestMonitoringControllerDelegate: %p statusUpdateCalled %@ commissioningSessionEstablishmentDoneCalled %@ commissioningCompleteCalled %@ readCommissioneeInfoCalled %@>", self, MTR_YES_NO(_statusUpdateCalled), MTR_YES_NO(_commissioningSessionEstablishmentDoneCalled), MTR_YES_NO(_commissioningCompleteCalled), MTR_YES_NO(_readCommissioneeInfoCalled)];
}

- (void)_checkIfAllCallbacksCalled
{
    if (self.allCallbacksCalledExpectation) {
        if (self.statusUpdateCalled && self.commissioningSessionEstablishmentDoneCalled && self.commissioningCompleteCalled && self.readCommissioneeInfoCalled) {
            [self.allCallbacksCalledExpectation fulfill];
            self.allCallbacksCalledExpectation = nil;
        }
    }
}

- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status
{
    self.statusUpdateCalled = YES;
    [self _checkIfAllCallbacksCalled];
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    self.commissioningSessionEstablishmentDoneCalled = YES;
    [self _checkIfAllCallbacksCalled];
}

- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error
                   nodeID:(NSNumber * _Nullable)nodeID
                  metrics:(MTRMetrics *)metrics
{
    self.commissioningCompleteCalled = YES;
    [self _checkIfAllCallbacksCalled];
}

- (void)controller:(MTRDeviceController *)controller readCommissioneeInfo:(MTRCommissioneeInfo *)info
{
    self.readCommissioneeInfoCalled = YES;
    [self _checkIfAllCallbacksCalled];
}

@end

@interface MTRPairingTests : MTRTestCase
@property (nullable) MTRPairingTestControllerDelegate * controllerDelegate;
@end

@implementation MTRPairingTests

+ (void)setUp
{
    [super setUp];

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

    [super tearDown];
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

    [super tearDown];
}

- (void)startServerApp
{
    // For manual testing, CASE retry code paths can be tested by adding --faults chip_CASEServerBusy_f1 (or similar)
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[
                                    @"--dac_provider",
                                    [self absolutePathFor:@"credentials/development/commissioner_dut/struct_cd_origin_pid_vid_correct/test_case_vector.json"],
                                ]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);
}

// attestationDelegate and failSafeExtension can both be nil
- (void)doPairingTestWithAttestationDelegate:(id<MTRDeviceAttestationDelegate>)attestationDelegate failSafeExtension:(NSNumber *)failSafeExtension
{
    [self doPairingTestWithAttestationDelegate:attestationDelegate failSafeExtension:failSafeExtension startServerApp:YES];
}

- (void)doPairingTestWithAttestationDelegate:(id<MTRDeviceAttestationDelegate>)attestationDelegate failSafeExtension:(NSNumber *)failSafeExtension startServerApp:(BOOL)startServerApp
{
    if (startServerApp) {
        [self startServerApp];
    }

    // Don't reuse node ids, because that will confuse us.
    ++sDeviceId;
    XCTestExpectation * expectation = [self expectationWithDescription:@"Commissioning Complete"];

    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:expectation
                                                                                 attestationDelegate:attestationDelegate
                                                                                   failSafeExtension:failSafeExtension];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    [sController setDeviceControllerDelegate:controllerDelegate queue:callbackQueue];
    self.controllerDelegate = controllerDelegate;

    // Test that a monitoring delegate works
    __auto_type * monitoringControllerDelegate = [[MTRPairingTestMonitoringControllerDelegate alloc] init];
    XCTestExpectation * allCallbacksCalledExpectation = [self expectationWithDescription:@"All callbacks called on monitoring delegate"];
    monitoringControllerDelegate.allCallbacksCalledExpectation = allCallbacksCalledExpectation;
    [sController addDeviceControllerDelegate:monitoringControllerDelegate queue:callbackQueue];
    XCTAssertEqual([sController unitTestDelegateCount], 2);

    // Test that the addDeviceControllerDelegate delegate is held weakly by the controller
    @autoreleasepool {
        __auto_type * monitoringControllerDelegate = [[MTRPairingTestMonitoringControllerDelegate alloc] init];
        [sController addDeviceControllerDelegate:monitoringControllerDelegate queue:callbackQueue];
        XCTAssertEqual([sController unitTestDelegateCount], 3);
    }
    XCTAssertEqual([sController unitTestDelegateCount], 2);

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(sDeviceId) error:&error]);
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation, allCallbacksCalledExpectation ] timeout:kPairingTimeoutInSeconds];
    XCTAssertNil(controllerDelegate.commissioningCompleteError);

    // Test that the monitoring delegate got all the callbacks
    XCTAssertTrue(monitoringControllerDelegate.statusUpdateCalled);
    XCTAssertTrue(monitoringControllerDelegate.commissioningSessionEstablishmentDoneCalled);
    XCTAssertTrue(monitoringControllerDelegate.commissioningCompleteCalled);
    XCTAssertTrue(monitoringControllerDelegate.readCommissioneeInfoCalled);
    [sController removeDeviceControllerDelegate:monitoringControllerDelegate];
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

- (void)doPairingAndWaitForProgress:(NSString *)trigger attestationDelegate:(nullable id<MTRDeviceAttestationDelegate>)attestationDelegate
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Trigger message seen"];
    expectation.assertForOverFulfill = NO;
    MTRSetLogCallback(MTRLogTypeDetail, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if ([message containsString:trigger]) {
            [expectation fulfill];
            [NSThread sleepForTimeInterval:0.5]; // yield and give the test thread a head start
        }
    });

    // If there is an attestation delegate, make sure sure our fail-safe extension is long
    // enough that we actually trigger a fail-safe extension (so longer than the 1-minute default).
    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:nil
                                                                                 attestationDelegate:attestationDelegate
                                                                                   failSafeExtension:@(90)];
    [sController setDeviceControllerDelegate:controllerDelegate queue:dispatch_get_main_queue()];
    self.controllerDelegate = controllerDelegate;

    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:NULL];
    XCTAssertNotNil(payload);
    NSError * error;
    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(++sDeviceId) error:&error]);
    XCTAssertNil(error);

    // Wait for the trigger message and then return to the caller
    // Don't wait for anything else here, since the pairing process is going to
    // continue asynchronously, and the caller may want to cancel it in a specific state.
    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
    MTRSetLogCallback(0, nil);
}

- (void)doPairingTestAfterCancellationAtProgress:(NSString *)trigger attestationDelegate:(nullable id<MTRDeviceAttestationDelegate>)attestationDelegate
{
    [self startServerApp];

    // Run pairing up and wait for the trigger
    [self doPairingAndWaitForProgress:trigger attestationDelegate:attestationDelegate];

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
    [self doPairingTestWithAttestationDelegate:nil failSafeExtension:nil startServerApp:NO];
}

- (void)doPairingTestAfterCancellationAtProgress:(NSString *)trigger
{
    [self doPairingTestAfterCancellationAtProgress:trigger attestationDelegate:nil];
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

- (void)test008_pairingAfterCancellation_DeviceAttestationVerification
{
    // Cancel pairing while we are waiting for our client to decide what to do
    // with the attestation information we got.  Note that the delegate is
    // called on some arbitrary queue, so we need to make sure we wait for it to
    // actually be called; we can't just have it set a boolean that we then
    // check, because that can race.
    XCTestExpectation * expectation = [self expectationWithDescription:@"Attestation delegate called"];
    __auto_type * attestationDelegate = [[NoOpAttestationDelegate alloc] initWithExpectation:expectation blockCommissioning:YES];

    [self doPairingTestAfterCancellationAtProgress:@"Successfully extended fail-safe timer to handle DA failure" attestationDelegate:attestationDelegate];
    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
}

- (void)test009_PairWithReadingEndpointInformation
{
    [self startServerApp];

    XCTestExpectation * expectation = [self expectationWithDescription:@"Commissioning Complete"];
    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:expectation
                                                                                 attestationDelegate:nil
                                                                                   failSafeExtension:nil];

    // Endpoint info is validated by MTRPairingTestControllerDelegate
    controllerDelegate.shouldReadEndpointInformation = YES;

    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    [sController setDeviceControllerDelegate:controllerDelegate queue:callbackQueue];
    self.controllerDelegate = controllerDelegate;

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(++sDeviceId) error:&error]);
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
    XCTAssertNil(controllerDelegate.commissioningCompleteError);
}

- (void)test010_PairWithReadingExtraAttributes
{
    [self startServerApp];

    XCTestExpectation * expectation = [self expectationWithDescription:@"Commissioning Complete"];
    __auto_type * controllerDelegate = [[MTRPairingTestControllerDelegate alloc] initWithExpectation:expectation
                                                                                 attestationDelegate:nil
                                                                                   failSafeExtension:nil];

    controllerDelegate.extraAttributesToRead = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:@(0)
                                                 clusterID:@(MTRClusterIDTypeDescriptorID)
                                               attributeID:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)],
        [MTRAttributeRequestPath requestPathWithEndpointID:@(0)
                                                 clusterID:@(MTRClusterIDTypeBasicInformationID)
                                               attributeID:@(MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID)],
    ];

    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    [sController setDeviceControllerDelegate:controllerDelegate queue:callbackQueue];
    self.controllerDelegate = controllerDelegate;

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertTrue([sController setupCommissioningSessionWithPayload:payload newNodeID:@(++sDeviceId) error:&error]);
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
    XCTAssertNil(controllerDelegate.commissioningCompleteError);
}

@end
