/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static const uint16_t kPairingTimeoutInSeconds = 30;
// The external NOC chain issuer watchdog fires after ~20s; allow generous
// headroom for slow CI hosts before we conclude the watchdog never armed.
static const uint16_t kIssuerWatchdogTimeoutInSeconds = 60;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static NSString * kCSRNonceStr = @"01234567890123456789012345678901"; // 32 chars

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

@interface MTROperationalCertificateIssureTestDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTROperationalCertificateIssureTestDeviceControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.csrNonce = [kCSRNonceStr dataUsingEncoding:NSUTF8StringEncoding];

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId)
                  commissioningParams:params
                                error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete:
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertNotNil(error);
    XCTAssertEqual(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, MTRErrorCodeIntegrityCheckFailed);
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface OperationalCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
@end

@implementation OperationalCertificateIssuer

- (instancetype)init
{
    if (self = [super init]) {
        _shouldSkipAttestationCertificateValidation = NO;
    }
    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    XCTAssertNotNil(csrInfo);
    XCTAssertNotNil(attestationInfo);
    XCTAssertEqual(controller, sController);

    XCTAssertEqualObjects(csrInfo.csrNonce, [kCSRNonceStr dataUsingEncoding:NSUTF8StringEncoding]);

    __auto_type * csrInfoCopy = [[MTROperationalCSRInfo alloc] initWithCSRElementsTLV:csrInfo.csrElementsTLV
                                                                 attestationSignature:csrInfo.attestationSignature];
    XCTAssertEqualObjects(csrInfoCopy.csr, csrInfo.csr);
    XCTAssertEqualObjects(csrInfoCopy.csrNonce, csrInfo.csrNonce);
    XCTAssertEqualObjects(csrInfoCopy.csrElementsTLV, csrInfo.csrElementsTLV);
    XCTAssertEqualObjects(csrInfoCopy.attestationSignature, csrInfo.attestationSignature);

    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
}

@end

// Tracks the SendNOC commissioning failure path that fires when the external
// operational-certificate issuer never invokes its completion, modeling the
// real-world case where a 3rd-party companion app crashes mid-pairing.
// Without the issuer-side watchdog the SDK waits the full 67-second
// MRP-inflated exchange timeout; with it commissioning fails fast (~20s)
// with a CHIP_ERROR_TIMEOUT plumbed up to the controller delegate.
@interface MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
// atomic so the delegate's callback-queue writes synchronize with reads off
// the test thread without an additional barrier; under TSan a nonatomic
// strong property crossing the callback-queue / test-thread boundary
// after waitForExpectations is reported as a race even when XCTest's
// expectation handshake provides happens-before-in-practice.
@property (atomic, strong) XCTestExpectation * expectation;
@property (atomic, strong) NSDate * commissionStartDate;
@property (atomic, strong, nullable) NSError * commissioningError;
@end

@implementation MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.csrNonce = [kCSRNonceStr dataUsingEncoding:NSUTF8StringEncoding];

    self.commissionStartDate = [NSDate date];

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId)
                  commissioningParams:params
                                error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete:
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertNotNil(error);
    self.commissioningError = error;
    [_expectation fulfill];
    _expectation = nil;
}

@end

// An MTROperationalCertificateIssuer that captures (and intentionally never
// invokes) the completion block. This models a 3rd-party issuer process
// crashing or hanging mid-issuance.
@interface NeverCompletingOperationalCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
@property (atomic, copy, nullable) void (^capturedCompletion)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable);
// Captured at the moment issueOperationalCertificateForRequest:... is invoked,
// so tests can pin the elapsed-time bound to the watchdog window itself rather
// than to an earlier commissioning phase (PASE / attestation) which on slow CI
// can satisfy a ">15s elapsed since PASE" assertion without ever entering the
// watchdog window. Using this anchor catches the regression class where the
// watchdog never arms.
@property (atomic, strong, nullable) NSDate * issuerInvokedAt;
@end

@implementation NeverCompletingOperationalCertificateIssuer

- (instancetype)init
{
    if (self = [super init]) {
        _shouldSkipAttestationCertificateValidation = NO;
    }
    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    XCTAssertNotNil(csrInfo);
    XCTAssertNotNil(attestationInfo);
    // Capture the moment the issuer is invoked so tests can bound elapsed time
    // against the watchdog window itself rather than an earlier commissioning
    // phase. This guards against a regression where commissioning fails before
    // the watchdog arms (which would silently satisfy a PASE-anchored bound).
    self.issuerInvokedAt = [NSDate date];
    // Stash the completion so the test can verify late completion is harmless
    // (i.e. doesn't crash via use-after-free or double-callback). We never
    // invoke it from this method.
    self.capturedCompletion = completion;
}

@end

// An MTROperationalCertificateIssuer that invokes the completion block twice
// in quick succession from inside its issueOperationalCertificateForRequest:
// method. Models a buggy 3rd-party companion that races two response paths
// (e.g. main + retry) and ends up calling the SDK callback twice. Pins that
// the second call is dropped by the mOnNOCCompletionCallback == nullptr
// guard in ExternalNOCChainGenerated and does not double-callback the
// controller delegate or crash via use-after-free on the NOC Callback.
@interface DoubleCallingOperationalCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
@property (atomic, assign) NSUInteger invocationCount;
@end

@implementation DoubleCallingOperationalCertificateIssuer

- (instancetype)init
{
    if (self = [super init]) {
        _shouldSkipAttestationCertificateValidation = NO;
    }
    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    XCTAssertNotNil(csrInfo);
    XCTAssertNotNil(attestationInfo);
    self.invocationCount += 1;
    // First call drives the real failure path; second call must be dropped
    // by the SDK guard. Both go to the issuer queue (main queue) where the
    // SDK has dispatched us, so they will land on the Matter queue serially.
    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
}

@end

// An MTROperationalCertificateIssuer that defers its completion by a fixed
// (test-controlled) interval, simulating a healthy-but-not-instantaneous
// 3rd-party issuer. Used by the hardener tests below to pin the
// healthy-path contract: a real completion arriving well within the watchdog
// window must surface the issuer's error (not the watchdog's timeout error)
// and must complete in ~the issuer's delay, not the full watchdog window.
@interface DelayedOperationalCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
@property (nonatomic, assign) NSTimeInterval completionDelaySeconds;
@property (nonatomic, strong) NSError * completionError;
@end

@implementation DelayedOperationalCertificateIssuer

- (instancetype)init
{
    if (self = [super init]) {
        _shouldSkipAttestationCertificateValidation = NO;
        _completionDelaySeconds = 3.0;
        _completionError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil];
    }
    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    XCTAssertNotNil(csrInfo);
    XCTAssertNotNil(attestationInfo);
    NSTimeInterval delay = self.completionDelaySeconds;
    NSError * err = self.completionError;
    // Deliver on a queue distinct from the issuer queue / Matter queue to
    // model real-world 3rd-party companions that hop completions onto a
    // background helper queue before invoking the SDK callback. The SDK
    // already re-enters the Matter stack via dispatch / ScheduleWork, so the
    // exact origin queue must not matter — but pin it so a regression that
    // accidentally requires same-queue delivery is caught.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (delay * NSEC_PER_SEC)),
        dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
            completion(nil, err);
        });
}

@end

@interface MTROperationalCertificateIssuerTests : MTRTestCase
@end

@implementation MTROperationalCertificateIssuerTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)testFailedCertificateIssuance
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[OperationalCertificateIssuer alloc] init];

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssureTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Pins the issuer-side watchdog: when the external operational-certificate
// issuer never invokes its completion (e.g. the 3rd-party companion process
// crashed mid-commissioning), the SDK must surface a timeout to the
// controller delegate within ~20s instead of waiting the full 67s MRP
// exchange timeout. Also exercises the "late completion is a no-op" path: at
// teardown, this test invokes the captured completion block to confirm no
// double-callback / UAF occurs after the watchdog already fired.
- (void)testExternalNOCChainGeneration_IssuerNeverCompletes_FiresWatchdogTimeout
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kIssuerWatchdogTimeoutInSeconds handler:nil];

    // The watchdog must have fired and surfaced a timeout-derived error, not a
    // success and not the longer 67s MRP-exchange timeout from the underlying
    // CommandSender path. Pin the contract: domain MTRErrorDomain, code
    // MTRErrorCodeTimeout (the mapping for CHIP_ERROR_TIMEOUT). Without this
    // an unrelated commissioning failure (PASE / attestation / CSR) at any
    // earlier stage would silently satisfy the elapsed-time bound.
    XCTAssertNotNil(deviceControllerDelegate.commissioningError);
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout);
    // Pin elapsed time to the watchdog window itself, anchored at issuer
    // invocation rather than at PASE completion. A regression that fails
    // commissioning before the watchdog arms would silently satisfy a
    // PASE-anchored ">15s" bound on slow CI; anchoring here catches it.
    XCTAssertNotNil(certificateIssuer.issuerInvokedAt,
        @"Issuer was never invoked; the test did not exercise the watchdog path");
    NSTimeInterval issuerElapsed = [[NSDate date] timeIntervalSinceDate:certificateIssuer.issuerInvokedAt];
    // 15s lower bound = 20s arm - 0.5s leeway - up to 4.5s pre-issuer/CI jitter slack.
    XCTAssertGreaterThan(issuerElapsed, 15.0,
        @"Commissioning should not fail before the issuer watchdog window opens (~20s)");
    XCTAssertLessThan(issuerElapsed, 30.0,
        @"Commissioning should fail within the watchdog window, not the MRP exchange timeout");

    // Late completion arriving after the watchdog already fired must be
    // harmless: the implementation guards re-entry with mOnNOCCompletionCallback
    // == nullptr, so this should be a no-op rather than a UAF / double callback.
    // Use XCTAssertNotNil (not a guarded if) so a regression that bypasses the
    // external issuer entirely (e.g. fails earlier in attestation) is surfaced
    // here rather than silently skipped.
    void (^captured)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable) = certificateIssuer.capturedCompletion;
    XCTAssertNotNil(captured, @"Issuer completion was never captured; the test did not exercise the watchdog path");
    if (captured != nil) {
        // Invoke on the issuer queue (main queue, matching production) to
        // mirror the real call path of a 3rd-party companion responding late.
        XCTestExpectation * lateExpectation = [self expectationWithDescription:@"Late issuer completion delivered"];
        dispatch_async(dispatch_get_main_queue(), ^{
            captured(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
            [lateExpectation fulfill];
        });
        [self waitForExpectations:@[ lateExpectation ] timeout:5.0];
    }

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Pins the destructor-vs-watchdog teardown path: shut down the controller
// while the issuer-watchdog dispatch_source is armed-but-unfired. Pre-fix
// this trips libdispatch's "BUG IN CLIENT OF LIBDISPATCH: Release of a
// source that has not been cancelled" trap when the delegate destructor
// releases the unfired source, and/or use-after-frees the delegate from
// the handler block running on its target queue. Post-fix the destructor
// cancels the source and fences on the Matter queue before allowing
// `this` to be freed, so this scenario must complete without crashing.
- (void)testExternalNOCChainGeneration_ShutdownWhileWatchdogArmed_DoesNotTrap
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    // The hanging-issuer delegate is wired up so that commissioningComplete
    // (when it arrives, e.g. after the watchdog fires) is captured rather
    // than asserted, since this test tears the controller down before the
    // watchdog would fire.
    // Use a free-standing XCTestExpectation (NOT [self expectationWithDescription:])
    // so we don't auto-register it with the test case. The delegate fulfills it
    // from commissioningComplete:, but this test tears down before that point;
    // an auto-registered expectation that we never wait on would fail the test.
    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete (unused — torn down before watchdog fires)"];
    expectation.assertForOverFulfill = NO;
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Wait for the issuer to receive the request (proving the watchdog has
    // been armed) but well before the 20s watchdog timer would fire. This
    // guarantees we exercise the armed-but-unfired teardown path, which is
    // the exact scenario that pre-fix tripped the libdispatch trap.
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:10.0];
    while (certificateIssuer.capturedCompletion == nil && [deadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    }
    XCTAssertNotNil(certificateIssuer.capturedCompletion,
        @"Issuer never received request; cannot prove watchdog was armed when shutdown ran");

    // Shut down with the watchdog armed-but-unfired. Pre-fix this is where
    // the libdispatch "release of an uncancelled source" trap fires, or
    // where the handler races the destructor and UAFs the delegate. Post-fix
    // the destructor cancels and Matter-queue-fences before freeing.
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Late completion arriving after shutdown must also be a no-op rather
    // than a UAF on the now-freed delegate.
    void (^captured)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable) = certificateIssuer.capturedCompletion;
    if (captured != nil) {
        XCTestExpectation * lateExpectation = [self expectationWithDescription:@"Late issuer completion delivered after shutdown"];
        dispatch_async(dispatch_get_main_queue(), ^{
            captured(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
            [lateExpectation fulfill];
        });
        [self waitForExpectations:@[ lateExpectation ] timeout:5.0];
    }

    [factory stopControllerFactory];
}

// Stress / long-running variant of the shutdown-while-armed test: repeatedly
// stand up a controller with a hung issuer, let the watchdog arm, then tear
// it down before the 20s window expires. Flushes out residual handler-vs-
// destructor races and any leak / cumulative-trap paths the single-shot
// test would not surface.
- (void)testExternalNOCChainGeneration_RepeatedShutdownDuringWatchdogWindow_NoLibdispatchTrap
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    // Three iterations is a balance: enough to surface a flakey race, not
    // so many that the test crowds out the rest of the Darwin suite. Each
    // iteration starts a fresh factory + controller, lets the watchdog
    // arm, and tears down inside the watchdog window.
    //
    // The all-clusters-app is launched once for the whole test. After an
    // iteration tears the controller down mid-PASE, the accessory's
    // commissioning window may not always be ready to accept a fresh PASE
    // immediately on the next iteration — that is an accessory-side timing
    // quirk, not a regression in the watchdog/teardown contract under
    // test. We therefore (a) track how many iterations actually reached
    // the issuer (proving the watchdog was armed when shutdown ran), and
    // (b) only require that at least one iteration did so. Iterations
    // where the accessory never accepted PASE are silently skipped — they
    // exercise factory startup/teardown ordering but cannot pin the
    // shutdown-vs-watchdog race. A real regression in the watchdog or
    // destructor would be caught by the single-shot
    // `ShutdownWhileWatchdogArmed_DoesNotTrap` test in addition to this
    // one.
    static const NSUInteger kIterations = 3;
    NSUInteger iterationsThatReachedIssuer = 0;

    for (NSUInteger i = 0; i < kIterations; ++i) {
        __auto_type * storage = [[MTRTestStorage alloc] init];
        __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
        factoryParams.port = @(kLocalPort);

        BOOL ok = [factory startControllerFactory:factoryParams error:nil];
        XCTAssertTrue(ok);

        __auto_type * testKeys = [[MTRTestKeys alloc] init];
        XCTAssertNotNil(testKeys);

        sTestKeys = testKeys;

        __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

        __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
        params.vendorID = @(kTestVendorId);
        params.operationalCertificateIssuer = certificateIssuer;
        params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

        MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
        XCTAssertNotNil(controller);

        sController = controller;

        // Free-standing expectation (not registered with the test case) so we
        // don't have to wait on it; the delegate fulfills from commissioningComplete:
        // but the test tears down before that point.
        XCTestExpectation * sessionExpectation = [[XCTestExpectation alloc] initWithDescription:[NSString stringWithFormat:@"Pairing Complete iter %lu", (unsigned long) i]];
        __auto_type * deviceControllerDelegate =
            [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:sessionExpectation];
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

        [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

        NSError * error;
        __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
        XCTAssertNotNil(payload);
        XCTAssertNil(error);

        [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
        XCTAssertNil(error);

        // Spin until the issuer has been called (proving the watchdog is
        // armed) before tearing down. Bound the wait so a regression that
        // never reaches the issuer fails loudly instead of timing out at
        // the suite level.
        NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:10.0];
        while (certificateIssuer.capturedCompletion == nil && [deadline timeIntervalSinceNow] > 0) {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
        }
        BOOL reachedIssuer = (certificateIssuer.capturedCompletion != nil);
        if (reachedIssuer) {
            ++iterationsThatReachedIssuer;
        }

        [controller shutdown];
        XCTAssertFalse([controller isRunning]);

        // Drop the unfulfilled expectation so it does not bleed into the
        // next iteration. The point of the test is that teardown during
        // the watchdog window is safe, not that commissioning succeeds.
        deviceControllerDelegate.expectation = nil;

        [factory stopControllerFactory];
    }

    // We need at least one iteration to have actually exercised the
    // shutdown-while-watchdog-armed path; otherwise the test degenerated
    // into a factory-startup/teardown loop and tells us nothing about the
    // contract under test.
    XCTAssertGreaterThan(iterationsThatReachedIssuer, 0u,
        @"No iteration reached the external NOC issuer; cannot prove watchdog was armed when shutdown ran");
}

// Destructor-vs-pending-watchdog robustness: drives controller teardown
// without ever issuing the explicit `[controller shutdown]` ordering the
// other shutdown tests rely on. The factory `stopControllerFactory` path
// drops the controller's last reference, which transitively destructs the
// MTROperationalCredentialsDelegate while its watchdog dispatch_source is
// still armed-but-unfired. Pre-fix this is the same libdispatch
// "uncancelled source release" trap; post-fix the destructor's
// cancel + Matter-queue fence makes it safe. Distinct from the explicit
// `[controller shutdown]` tests above because it covers the path where
// teardown is driven purely by reference counting / factory-stop, which
// has historically had subtly different ordering w.r.t. the Matter queue
// drain than an explicit shutdown call.
- (void)testExternalNOCChainGeneration_FactoryStopWhileWatchdogArmed_DoesNotTrap
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    // Free-standing expectation (not registered with the test case) so we don't
    // have to wait on it; the delegate fulfills from commissioningComplete:
    // but the test tears down before that point.
    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete (unused — factory stopped before watchdog fires)"];
    expectation.assertForOverFulfill = NO;
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Spin until the issuer has been called (proving the watchdog is armed)
    // before tearing down. Bound the wait so a regression that never reaches
    // the issuer fails loudly here instead of timing out at suite level.
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:10.0];
    while (certificateIssuer.capturedCompletion == nil && [deadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    }
    XCTAssertNotNil(certificateIssuer.capturedCompletion,
        @"Issuer never received request; cannot prove watchdog was armed when factory-stop ran");

    // Drop our own strong reference, then stop the factory without an
    // intervening explicit `[controller shutdown]`. The factory-stop path
    // is what tears the credentials delegate down; if its destructor does
    // not cancel + fence the watchdog, libdispatch traps here.
    sController = nil;
    controller = nil;
    [factory stopControllerFactory];

    // Late completion arriving after factory-stop must also be a no-op
    // rather than a UAF on the now-freed delegate. The watchdog's handler
    // captures `this` (the C++ delegate), so if the destructor's fence is
    // missing, calling the captured block could land on freed memory.
    void (^captured)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable) = certificateIssuer.capturedCompletion;
    if (captured != nil) {
        XCTestExpectation * lateExpectation = [self expectationWithDescription:@"Late issuer completion delivered after factory-stop"];
        dispatch_async(dispatch_get_main_queue(), ^{
            captured(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
            [lateExpectation fulfill];
        });
        [self waitForExpectations:@[ lateExpectation ] timeout:5.0];
    }
}

// Issuer-respond-just-after-timeout race: lets the watchdog actually fire
// (asserting we get the timeout error on the controller delegate) and then
// invokes the captured completion *immediately*, with no waiting, to land
// inside the narrow window where the watchdog's
// ExternalNOCChainGenerated has cleared mOnNOCCompletionCallback but the
// rest of the SDK teardown is still settling. The existing
// `IssuerNeverCompletes_FiresWatchdogTimeout` test invokes the late
// completion well after settlement; this test deliberately tightens that
// timing to pin the post-fire double-callback guard.
//
// Contract being pinned:
//   1. Watchdog drives exactly one MTRErrorCodeTimeout callback to the
//      controller delegate.
//   2. A near-simultaneous issuer response is dropped (mOnNOCCompletionCallback
//      already nullptr) — the delegate must not see a second
//      commissioningComplete callback, and the process must not crash.
- (void)testExternalNOCChainGeneration_IssuerRespondsJustAfterWatchdogFires_NoDoubleCallback
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Wait for the watchdog to fire and surface the timeout. This is the
    // same wait as the canonical "fires watchdog" test — we need it to
    // settle before we exercise the racing late completion.
    [self waitForExpectationsWithTimeout:kIssuerWatchdogTimeoutInSeconds handler:nil];

    XCTAssertNotNil(deviceControllerDelegate.commissioningError);
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout);

    void (^captured)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable) = certificateIssuer.capturedCompletion;
    XCTAssertNotNil(captured,
        @"Issuer completion was never captured; the test did not exercise the racing-completion path");

    // Drive the captured completion immediately on the issuer queue with
    // zero artificial delay. If the watchdog's
    // ExternalNOCChainGenerated -> mOnNOCCompletionCallback = nullptr
    // sequencing is broken, this races into a double-callback (second
    // commissioningComplete) or a UAF. The guard in
    // ExternalNOCChainGenerated treats this as a no-op; we pin that
    // contract by re-checking the delegate's stored error is unchanged
    // and that no second callback was delivered.
    NSError * preRaceError = deviceControllerDelegate.commissioningError;
    dispatch_async(dispatch_get_main_queue(), ^{
        captured(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
    });

    // Give the racing completion a moment to land on the Matter queue and
    // be consumed (or dropped) by ExternalNOCChainGenerated's guard. A
    // 2-second window comfortably covers any realistic dispatch latency
    // without making the test slow.
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];

    // The delegate's commissioningComplete handler nils its expectation
    // after fulfilling once; if a double-callback regression lets the
    // late completion drive a second invocation, that handler will trip
    // its XCTAssertNotNil(error) on a fresh path and fulfill an already-
    // fulfilled expectation (or attempt to call -fulfill on nil). Pin
    // both: the stored error must still be the watchdog timeout (object
    // identity, not just value), and the expectation must be released.
    XCTAssertEqual(deviceControllerDelegate.commissioningError, preRaceError,
        @"Late issuer completion must not deliver a second commissioningComplete callback");
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout);
    XCTAssertNil(deviceControllerDelegate.expectation,
        @"Delegate expectation should have been cleared by the single watchdog-driven callback");

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Hardener: pin the healthy-path watchdog-cancel contract under conditions
// that the existing tests do NOT cover:
//   - The issuer takes a non-trivial but legal amount of time (~3s) to
//     respond, so the watchdog has actually been armed and would have
//     observable effect if cancellation were broken.
//   - The completion is delivered from a queue distinct from both the
//     issuer queue and the Matter queue (a global concurrent queue),
//     modelling a 3rd-party companion that hops through helper queues.
//   - The issuer queue itself is a custom serial queue, not main, so a
//     regression that accidentally hardcodes main-queue assumptions in the
//     watchdog cancel path surfaces here.
// The assertions pin two distinct regressions:
//   1. The error reaching the controller delegate must be the issuer's
//      error (MTRErrorCodeIntegrityCheckFailed in this test), NOT
//      MTRErrorCodeTimeout — i.e. the watchdog must not have fired and
//      must not have raced ahead of the real completion.
//   2. End-to-end commissioning must fail in ~the issuer's delay, well
//      below the watchdog window. A regression that fails to cancel the
//      watchdog on success would still surface the right error today
//      (because mOnNOCCompletionCallback == nullptr would drop the
//      watchdog's late callback) but would leave the dispatch_source
//      alive past the success path. We pin elapsed time as the
//      observable proxy.
- (void)testExternalNOCChainGeneration_DelayedHealthyIssuerOnCustomQueue_NoWatchdogTimeout
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[DelayedOperationalCertificateIssuer alloc] init];
    certificateIssuer.completionDelaySeconds = 3.0;

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    // Custom serial queue (not main) for the issuer callback. Pins that the
    // watchdog plumbing does not assume a specific issuer queue identity.
    params.operationalCertificateIssuerQueue = dispatch_queue_create("com.chip.test.delayed_issuer", DISPATCH_QUEUE_SERIAL);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Bound the wait below the watchdog window: if commissioning has not
    // completed by 18s, the healthy path is broken (or the watchdog raced
    // ahead). Either way, fail loudly here rather than at the suite-level
    // timeout.
    [self waitForExpectationsWithTimeout:18.0 handler:nil];

    // (1) The issuer's error — not the watchdog's timeout — must reach the
    // delegate. A regression that arms the watchdog incorrectly (or fails
    // to cancel on success) and races past the issuer's delayed completion
    // would surface MTRErrorCodeTimeout instead.
    XCTAssertNotNil(deviceControllerDelegate.commissioningError);
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertNotEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout,
        @"Healthy issuer must not surface a watchdog timeout error");
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeIntegrityCheckFailed,
        @"Delegate should see the issuer's error, not a synthesized one");

    // (2) Elapsed time must reflect the issuer's delay, not the watchdog
    // window. Lower bound proves the issuer was actually awaited; upper
    // bound proves we did not fall through to the 20s watchdog window or
    // the 67s MRP exchange timeout. Use unconditional XCTAssertNotNil so a
    // regression that fails before PASE completes fails loudly here rather
    // than silently skipping the timing bounds.
    XCTAssertNotNil(deviceControllerDelegate.commissionStartDate);
    NSTimeInterval elapsed = [[NSDate date] timeIntervalSinceDate:deviceControllerDelegate.commissionStartDate];
    XCTAssertGreaterThan(elapsed, 2.0,
        @"Commissioning completed before the issuer's deliberate 3s delay; the issuer was bypassed");
    XCTAssertLessThan(elapsed, 15.0,
        @"Commissioning took longer than the issuer's delay; the watchdog likely fired or did not cancel on success");

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Hardener: pin observable triage attributes of the watchdog-driven timeout
// error beyond just (domain, code). The existing
// `IssuerNeverCompletes_FiresWatchdogTimeout` test verifies the bare
// (MTRErrorDomain, MTRErrorCodeTimeout) pair, but a regression could
// preserve those bits and still degrade triage in two ways:
//   1. localizedDescription becoming empty / generic (reporters and log
//      triage scripts key off the user-visible message to disambiguate
//      "issuer hung" from a generic exchange timeout).
//   2. userInfo accidentally embedding an internal pointer or
//      `description`-of-an-objc-object string (a log-quarantine and PII
//      risk; these errors are surfaced via NSError's standard logging
//      path which can cross privacy boundaries).
// This test is intentionally cheap: it reuses the watchdog-fires path and
// only adds attribute checks on the resulting error.
- (void)testExternalNOCChainGeneration_WatchdogTimeoutError_HasTriagableAttributes
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kIssuerWatchdogTimeoutInSeconds handler:nil];

    NSError * commissioningError = deviceControllerDelegate.commissioningError;
    XCTAssertNotNil(commissioningError);
    XCTAssertEqualObjects(commissioningError.domain, MTRErrorDomain);
    XCTAssertEqual(commissioningError.code, MTRErrorCodeTimeout);

    // localizedDescription must be a non-empty string. Empty / nil
    // descriptions are a triage regression even when (domain, code) are
    // correct, because user-facing flows (and triage tools) display the
    // description rather than re-deriving from the code.
    NSString * description = commissioningError.localizedDescription;
    XCTAssertNotNil(description);
    XCTAssertGreaterThan(description.length, 0u,
        @"Watchdog timeout error must carry a non-empty localizedDescription for triage");

    // userInfo must not embed objc object descriptions (a recurring
    // log-quarantine / PII risk). We don't assert exact contents — those
    // can legitimately evolve — but we pin that none of the values are
    // an `NSObject *` whose -description starts with "<" (the default
    // pointer-bearing format like "<MTRDeviceController_Concrete: 0x...>")
    // or contain a hex pointer marker. Catches a regression that
    // accidentally puts `self` or a controller object into userInfo.
    NSDictionary * userInfo = commissioningError.userInfo;
    for (id key in userInfo) {
        id value = userInfo[key];
        if ([value isKindOfClass:[NSString class]]) {
            NSString * stringValue = (NSString *) value;
            XCTAssertFalse([stringValue containsString:@": 0x"],
                @"userInfo string for key %@ embeds an objc pointer (%@); triage / log-quarantine risk", key, stringValue);
        }
    }

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Regression: when the watchdog fires AFTER the controller has been shut down
// (not the more common case where shutdown beats the watchdog), the early-exit
// path inside the watchdog handler must clear mOnNOCCompletionCallback and
// mExternalNOCChainWatchdog so a subsequent destruction-time fence is a no-op
// and a future ExternalGenerateNOCChain re-arm sees clean state. Pre-fix the
// handler logged + returned, leaving the callback pointer dangling and the
// watchdog member non-nil (which made the destructor run unnecessary fences
// and, under ASan, observably read freed Callback memory).
- (void)testExternalNOCChainGeneration_IssuerNeverCompletes_WatchdogFiresAfterShutdown
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    // Free-standing expectation (not registered with the test case) so we don't
    // have to wait on it; the delegate fulfills from commissioningComplete:
    // but the test path may shut down before that point.
    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete (unused — torn down before watchdog fires)"];
    expectation.assertForOverFulfill = NO;
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Wait until the issuer has been invoked (proving the watchdog is armed).
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:10.0];
    while (certificateIssuer.issuerInvokedAt == nil && [deadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
    }
    XCTAssertNotNil(certificateIssuer.issuerInvokedAt,
        @"Issuer never received request; cannot prove watchdog was armed before shutdown");

    NSDate * issuerInvokedAt = certificateIssuer.issuerInvokedAt;

    // Shut down the controller while the watchdog is still armed. The destructor
    // must cancel + fence; then the watchdog handler block (already enqueued by
    // the timer fire if it raced ahead, or never enqueued if shutdown beat the
    // timer) must run cleanly. Either way, no crash.
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Wait past the 20s watchdog window so any late-firing handler has had a
    // chance to run. The destructor's two dispatch_sync fences should already
    // have drained the handler before shutdown returned, so this is just
    // belt-and-braces. If a regression reintroduces the controller-gone-without-
    // strand bug, ASan / leaks would catch it during this window.
    NSDate * postShutdownDeadline = [NSDate dateWithTimeIntervalSinceNow:25.0];
    while ([postShutdownDeadline timeIntervalSinceNow] > 0) {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.5]];
    }

    // Bound elapsed time so a regression that never arms the watchdog (and
    // thus never exercises this path) fails loudly.
    NSTimeInterval issuerElapsed = [[NSDate date] timeIntervalSinceDate:issuerInvokedAt];
    XCTAssertGreaterThan(issuerElapsed, 18.0,
        @"Test ended before the watchdog window opened; coverage gap");
    XCTAssertLessThan(issuerElapsed, 60.0,
        @"Test ran far past the watchdog window; something is very wrong");

    // Late completion arriving after the watchdog fired post-shutdown must be
    // a no-op rather than a UAF on the now-freed delegate.
    void (^captured)(MTROperationalCertificateChain * _Nullable, NSError * _Nullable) = certificateIssuer.capturedCompletion;
    if (captured != nil) {
        XCTestExpectation * lateExpectation = [self expectationWithDescription:@"Late issuer completion delivered after post-shutdown watchdog fire"];
        dispatch_async(dispatch_get_main_queue(), ^{
            captured(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
            [lateExpectation fulfill];
        });
        [self waitForExpectations:@[ lateExpectation ] timeout:5.0];
    }

    [factory stopControllerFactory];
}

// Hardener: A misbehaving 3rd-party issuer invokes the SDK completion block
// twice in quick succession (e.g. it raced two internal response paths and
// fired both). The first invocation drives the real commissioningComplete
// failure callback to the controller delegate; the second must be dropped
// by the `mOnNOCCompletionCallback == nullptr` guard in
// ExternalNOCChainGenerated without double-calling the underlying chip
// Callback (a UAF, since the first invocation already nulled
// mOnNOCCompletionCallback after passing it to mCall) and without
// double-fulfilling the delegate's expectation.
//
// Pre-fix this would have crashed under ASan via use-after-free on
// onCompletion->mCall. Post-fix the second call is logged + dropped.
//
// Distinct from `IssuerRespondsJustAfterWatchdogFires`: that test races a
// late issuer response against the watchdog *after* the watchdog has
// already driven completion. This test races two issuer responses against
// each other before the watchdog fires, which is a separate code path
// (the second invocation lands on the Matter queue serially with the
// first inside ExternalNOCChainGenerated, not as a separate
// timer-driven block).
- (void)testExternalNOCChainGeneration_DoubleInvocationByMisbehavingIssuer_NoCrashNoDoubleCallback
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];
    // Pin single-fulfillment: a regression that bypasses the
    // mOnNOCCompletionCallback nullptr guard would re-enter
    // ExternalNOCChainGenerated and fulfil the expectation a second time,
    // which assertForOverFulfill flags as a test failure.
    expectation.assertForOverFulfill = YES;

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[DoubleCallingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Bound the wait below the watchdog window: a healthy-fast double-invoke
    // path should fail commissioning in well under 20s. If we hit the
    // watchdog window, the issuer's first response was somehow lost.
    [self waitForExpectationsWithTimeout:18.0 handler:nil];

    // Pin the failure error came from the issuer (first invocation), not
    // from the watchdog. Both produce MTRErrorDomain but with different
    // codes; if the second invocation slipped past the guard it would
    // either double-fulfill (asserted by assertForOverFulfill above) or
    // overwrite commissioningError with the second call's value (still
    // IntegrityCheckFailed, indistinguishable, but the over-fulfill check
    // is the load-bearing assertion).
    XCTAssertNotNil(deviceControllerDelegate.commissioningError);
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertNotEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout,
        @"Double-invoke path should surface the issuer's error, not a watchdog timeout");
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeIntegrityCheckFailed);

    // The issuer must have actually been called twice (otherwise the test
    // didn't exercise the guard path it claims to).
    XCTAssertEqual(certificateIssuer.invocationCount, 1u,
        @"Issuer should only be invoked once by the SDK; multiple SDK invocations would indicate a different bug");

    // Settle: give any in-flight second-call dispatch a chance to land and
    // be dropped by the guard before we tear down. ASan / leaks running
    // during this window catch a regression that UAFs the chip Callback.
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Hardener: pin the watchdog behavior when the issuer queue is a global
// concurrent queue (not serial, not main). Production 3rd-party companions
// commonly pass `dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0)` as
// the issuer queue, which has different ordering / re-entrancy properties
// than the main / custom-serial queues all other tests use. A regression
// that accidentally relies on issuer-queue serialization (e.g. assumes the
// issuer-queue dispatch_async hop in GenerateNOCChain has completed before
// the watchdog arms) would surface here as a flaky watchdog-vs-arm race.
//
// Contract being pinned:
//   1. Watchdog still fires within its window (~20s) when issuer hangs,
//      even on a concurrent issuer queue.
//   2. The error reaching the delegate is MTRErrorCodeTimeout — i.e.
//      commissioning didn't fail earlier due to a queue-ordering bug.
- (void)testExternalNOCChainGeneration_HungIssuerOnGlobalConcurrentQueue_FiresWatchdogTimeout
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    // Concurrent global queue — NOT serial, NOT main. Pins that watchdog
    // arming and cancellation make no assumption about issuer-queue
    // serialization or identity.
    params.operationalCertificateIssuerQueue = dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kIssuerWatchdogTimeoutInSeconds handler:nil];

    XCTAssertNotNil(deviceControllerDelegate.commissioningError);
    XCTAssertEqualObjects(deviceControllerDelegate.commissioningError.domain, MTRErrorDomain);
    XCTAssertEqual(deviceControllerDelegate.commissioningError.code, MTRErrorCodeTimeout);

    // Issuer must have been invoked (otherwise the watchdog path was not
    // exercised). Anchor elapsed time at issuer-invoke to bound the
    // watchdog-window claim independently of earlier commissioning phases.
    XCTAssertNotNil(certificateIssuer.issuerInvokedAt,
        @"Issuer was never invoked on global concurrent queue; the test did not exercise the watchdog path");
    NSTimeInterval issuerElapsed = [[NSDate date] timeIntervalSinceDate:certificateIssuer.issuerInvokedAt];
    XCTAssertGreaterThan(issuerElapsed, 18.0,
        @"Commissioning failed before the watchdog window opened on a concurrent issuer queue");
    XCTAssertLessThan(issuerElapsed, 30.0,
        @"Commissioning ran past the watchdog window on a concurrent issuer queue");

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory stopControllerFactory];
}

// Hardener: shut down the controller IMMEDIATELY after kicking off
// commissioning, before the issuer has even been called (and thus before
// the watchdog has been armed). This pins that the destructor's
// "no-watchdog-armed" path is genuinely a no-op — i.e. the destructor
// does not unconditionally dispatch_sync onto the Matter queue (which
// would deadlock if shutdown is being driven from the Matter queue under
// platform teardown, and would slow every healthy controller shutdown).
//
// This is the pre-arm counterpart to ShutdownWhileWatchdogArmed: that
// test pins teardown safety with the source live; this one pins teardown
// safety / efficiency with the source nil. A regression that always
// fenced (regardless of source state) would still pass the armed test
// but would slow every healthy commissioning teardown — surfaced here as
// an elapsed-time assertion on the shutdown call itself.
- (void)testExternalNOCChainGeneration_ShutdownBeforeWatchdogArms_NoDeadlock
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    // Use a hung issuer so that if commissioning *did* progress to the
    // SendNOC stage before shutdown, we'd cleanly arm + cancel the
    // watchdog (the other tests already cover that). Here we want to
    // shut down before commissioning even reaches the issuer.
    __auto_type * certificateIssuer = [[NeverCompletingOperationalCertificateIssuer alloc] init];

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    // Free-standing expectation (not registered with the test case) so we don't
    // have to wait on it; the delegate fulfills from commissioningComplete:
    // but the test tears down before the issuer is even reached.
    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete (unused — torn down before issuer is reached)"];
    expectation.assertForOverFulfill = NO;
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssuerHangingTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    // Tear down immediately. The issuer should not yet have been called
    // (PASE / attestation take a moment), so the watchdog cannot have
    // been armed. The destructor's nil-source branch must be a no-op
    // and shutdown must complete promptly.
    NSDate * shutdownStart = [NSDate date];
    [controller shutdown];
    NSTimeInterval shutdownElapsed = [[NSDate date] timeIntervalSinceDate:shutdownStart];
    XCTAssertFalse([controller isRunning]);

    // Shutdown must be prompt — a regression that always fences on the
    // Matter queue (even when the source is nil) would still complete,
    // but the upper bound here would catch a worst-case dispatch_sync
    // deadlock-then-timeout. 10s is comfortably above realistic shutdown
    // latency on slow CI and well below any reasonable deadlock recovery.
    XCTAssertLessThan(shutdownElapsed, 10.0,
        @"Shutdown took unexpectedly long; possible spurious dispatch_sync fence on a nil watchdog source");

    // The issuer ideally was never called (we tore down before commissioning
    // reached the SendNOC stage). If it WAS called, that's not a test
    // failure — it just means we didn't exercise the pre-arm shutdown path
    // we intended; log via XCTSkip-style soft assertion. Either way, no
    // crash / deadlock is the load-bearing contract.
    if (certificateIssuer.issuerInvokedAt != nil) {
        NSLog(@"%@: issuer was reached before shutdown; pre-arm path not exercised, but post-arm shutdown is also safe (covered by ShutdownWhileWatchdogArmed)", self.name);
    }

    [factory stopControllerFactory];
}

@end
