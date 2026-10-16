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
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static NSString * kOnboardingPayload2 = @"MT:Y.K90KCZ16FQ1648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;
static NSString * kCSRNonceStr = @"01234567890123456789012345678901"; // 32 chars

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

@interface MTROperationalCertificateIssureTestDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@property (nonatomic) uint64_t nodeID;
@property (nonatomic) MTRErrorCode expectedErrorCode;
@property (nonatomic, nullable) NSNumber * expectedCHIPErrorCode;
@end

@implementation MTROperationalCertificateIssureTestDeviceControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _nodeID = kDeviceId;
        _expectedErrorCode = MTRErrorCodeIntegrityCheckFailed;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.csrNonce = [kCSRNonceStr dataUsingEncoding:NSUTF8StringEncoding];

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(_nodeID)
                  commissioningParams:params
                                error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete:
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertNotNil(error);
    XCTAssertEqual(error.domain, MTRErrorDomain);
    XCTAssertEqual(error.code, _expectedErrorCode);
    if (_expectedCHIPErrorCode) {
        XCTAssertEqualObjects(error.userInfo[@"errorCode"], _expectedCHIPErrorCode);
    }
    [_expectation fulfill];
    _expectation = nil;
}

@end

typedef void (^IssuerCompletion)(MTROperationalCertificateChain * _Nullable info, NSError * _Nullable error);

@interface OperationalCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;
@property (nonatomic) BOOL releasesCompletionWithoutCalling;
@property (nonatomic, copy, nullable) void (^holdCompletion)(IssuerCompletion completion);
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

    if (self.releasesCompletionWithoutCalling) {
        return;
    }

    if (self.holdCompletion) {
        self.holdCompletion(completion);
        return;
    }

    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
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

- (MTRDeviceController *)startControllerWithIssuer:(OperationalCertificateIssuer *)certificateIssuer
                                          delegate:(id<MTRDeviceControllerDelegate>)delegate
{
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

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);
    [controller setDeviceControllerDelegate:delegate queue:callbackQueue];
    return controller;
}

- (void)stopController:(MTRDeviceController *)controller
{
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

- (void)setUpCommissioningSessionWithController:(MTRDeviceController *)controller payload:(NSString *)payloadString nodeID:(uint64_t)nodeID
{
    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:payloadString error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(nodeID) error:&error];
    XCTAssertNil(error);
}

- (void)runCommissioningWithIssuer:(OperationalCertificateIssuer *)certificateIssuer
                 expectedErrorCode:(MTRErrorCode)expectedErrorCode
             expectedCHIPErrorCode:(nullable NSNumber *)expectedCHIPErrorCode
{
    BOOL started = [self startAppWithName:@"all-clusters"
                                arguments:@[]
                                  payload:kOnboardingPayload];
    XCTAssertTrue(started);

    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssureTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    deviceControllerDelegate.expectedErrorCode = expectedErrorCode;
    deviceControllerDelegate.expectedCHIPErrorCode = expectedCHIPErrorCode;

    MTRDeviceController * controller = [self startControllerWithIssuer:certificateIssuer delegate:deviceControllerDelegate];
    [self setUpCommissioningSessionWithController:controller payload:kOnboardingPayload nodeID:kDeviceId];

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];

    [self stopController:controller];
}

- (void)testFailedCertificateIssuance
{
    [self runCommissioningWithIssuer:[[OperationalCertificateIssuer alloc] init]
                   expectedErrorCode:MTRErrorCodeIntegrityCheckFailed
               expectedCHIPErrorCode:nil];
}

- (void)testIssuerReleasesCompletionWithoutCalling
{
    __auto_type * certificateIssuer = [[OperationalCertificateIssuer alloc] init];
    certificateIssuer.releasesCompletionWithoutCalling = YES;
    [self runCommissioningWithIssuer:certificateIssuer
                   expectedErrorCode:MTRErrorCodeGeneralError
               expectedCHIPErrorCode:@(0xAC)]; // CHIP_ERROR_INTERNAL
}

- (void)runCanceledRequestReleasingItsCompletionAfterNextRequest:(BOOL)afterNextRequest
{
    XCTAssertTrue([self startAppWithName:@"all-clusters" arguments:@[] payload:kOnboardingPayload]);
    XCTAssertTrue([self startAppWithName:@"all-clusters" arguments:@[] payload:kOnboardingPayload2]);

    NSMutableArray<IssuerCompletion> * heldCompletions = [NSMutableArray array];
    __block XCTestExpectation * requestExpectation = [self expectationWithDescription:@"First NOC request"];
    __auto_type * certificateIssuer = [[OperationalCertificateIssuer alloc] init];
    certificateIssuer.holdCompletion = ^(IssuerCompletion completion) {
        [heldCompletions addObject:completion];
        [requestExpectation fulfill];
    };

    XCTestExpectation * cancelExpectation = [self expectationWithDescription:@"First commissioning canceled"];
    __auto_type * deviceControllerDelegate =
        [[MTROperationalCertificateIssureTestDeviceControllerDelegate alloc] initWithExpectation:cancelExpectation];
    deviceControllerDelegate.expectedErrorCode = MTRErrorCodeCancelled;

    MTRDeviceController * controller = [self startControllerWithIssuer:certificateIssuer delegate:deviceControllerDelegate];
    [self setUpCommissioningSessionWithController:controller payload:kOnboardingPayload nodeID:kDeviceId];
    [self waitForExpectations:@[ requestExpectation ] timeout:kPairingTimeoutInSeconds];

    NSError * error;
    XCTAssertTrue([controller cancelCommissioningForNodeID:@(kDeviceId) error:&error]);
    XCTAssertNil(error);
    [self waitForExpectations:@[ cancelExpectation ] timeout:kPairingTimeoutInSeconds];

    if (afterNextRequest) {
        requestExpectation = [self expectationWithDescription:@"Second NOC request"];
    } else {
        [heldCompletions removeAllObjects];
        certificateIssuer.holdCompletion = nil;
    }

    XCTestExpectation * secondExpectation = [self expectationWithDescription:@"Second commissioning complete"];
    deviceControllerDelegate.expectation = secondExpectation;
    deviceControllerDelegate.expectedErrorCode = MTRErrorCodeIntegrityCheckFailed;
    deviceControllerDelegate.nodeID = kDeviceId + 1;
    [self setUpCommissioningSessionWithController:controller payload:kOnboardingPayload2 nodeID:kDeviceId + 1];

    if (afterNextRequest) {
        [self waitForExpectations:@[ requestExpectation ] timeout:kPairingTimeoutInSeconds];
        XCTAssertEqual(heldCompletions.count, 2);

        // Release the first request's completion uncalled, then complete the second.
        IssuerCompletion secondCompletion = heldCompletions[1];
        [heldCompletions removeAllObjects];
        secondCompletion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
        secondCompletion = nil;
    }
    [self waitForExpectations:@[ secondExpectation ] timeout:kPairingTimeoutInSeconds];

    [self stopController:controller];
}

- (void)testCanceledRequestCompletionReleasedWhileIdleIsIgnored
{
    [self runCanceledRequestReleasingItsCompletionAfterNextRequest:NO];
}

- (void)testCanceledRequestCompletionReleasedDuringNextRequestIsIgnored
{
    [self runCanceledRequestReleasingItsCompletionAfterNextRequest:YES];
}

@end
