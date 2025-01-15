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
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

// system dependencies
#import <XCTest/XCTest.h>

static const uint16_t kPairingTimeoutInSeconds = 30;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

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

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId)
                  commissioningParams:[[MTRCommissioningParameters alloc] init]
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

    __auto_type * csrInfoCopy = [[MTROperationalCSRInfo alloc] initWithCSRElementsTLV:csrInfo.csrElementsTLV
                                                                 attestationSignature:csrInfo.attestationSignature];
    XCTAssertEqualObjects(csrInfoCopy.csr, csrInfo.csr);
    XCTAssertEqualObjects(csrInfoCopy.csrNonce, csrInfo.csrNonce);
    XCTAssertEqualObjects(csrInfoCopy.csrElementsTLV, csrInfo.csrElementsTLV);
    XCTAssertEqualObjects(csrInfoCopy.attestationSignature, csrInfo.attestationSignature);

    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeIntegrityCheckFailed userInfo:nil]);
}

@end

@interface MTROperationalCertificateIssuerTests : XCTestCase
@end

@implementation MTROperationalCertificateIssuerTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)testFailedCertificateIssuance
{
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

@end
