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
static const uint64_t kDeviceId = 0x12341234;
static const uint64_t kControllerId = 0x56788765;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

static MTRBaseDevice * sConnectedDevice;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

@interface MTRCertificateValidityTestControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, readonly) XCTestExpectation * expectation;
@property (nonatomic, readonly) NSNumber * commissioneeNodeID;
@end

@implementation MTRCertificateValidityTestControllerDelegate
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

@interface MTRTestCertificateIssuer : NSObject <MTROperationalCertificateIssuer>

@property (nonatomic, readonly) MTRTestKeys * rootKey;
@property (nonatomic, copy, readonly) MTRCertificateDERBytes rootCertificate;
@property (nonatomic, copy, readonly) NSDateInterval * validityPeriod;
@property (nonatomic, copy, readonly) NSNumber * fabricID;
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;

- (nullable instancetype)initWithRootKey:(MTRTestKeys *)key
                                fabricID:(NSNumber *)fabricID
                          validityPeriod:(NSDateInterval *)validityPeriod;

- (nullable MTRCertificateDERBytes)issueOperationalCertificateForNode:(NSNumber *)nodeID
                                                 operationalPublicKey:(SecKeyRef)operationalPublicKey;

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion;
@end

@implementation MTRTestCertificateIssuer
- (nullable instancetype)initWithRootKey:(MTRTestKeys *)key
                                fabricID:(NSNumber *)fabricID
                          validityPeriod:(NSDateInterval *)validityPeriod
{
    if (!(self = [super init])) {
        return nil;
    }

    NSError * error;
    __auto_type * rootCertificate = [MTRCertificates createRootCertificate:key
                                                                  issuerID:nil
                                                                  fabricID:fabricID
                                                            validityPeriod:validityPeriod
                                                                     error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(rootCertificate);

    if (rootCertificate == nil) {
        return nil;
    }

    _validityPeriod = validityPeriod;
    _rootCertificate = rootCertificate;
    _rootKey = key;
    _fabricID = fabricID;
    _shouldSkipAttestationCertificateValidation = NO;

    return self;
}

- (nullable MTRCertificateDERBytes)issueOperationalCertificateForNode:(NSNumber *)nodeID
                                                 operationalPublicKey:(SecKeyRef)operationalPublicKey
{
    return [MTRCertificates createOperationalCertificate:self.rootKey
                                      signingCertificate:self.rootCertificate
                                    operationalPublicKey:operationalPublicKey
                                                fabricID:self.fabricID
                                                  nodeID:nodeID
                                   caseAuthenticatedTags:nil
                                          validityPeriod:self.validityPeriod
                                                   error:nil];
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    NSError * error;
    __auto_type * publicKey = [MTRCertificates publicKeyFromCSR:csrInfo.csr error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(publicKey);

    NSDictionary * attributes =
        @{ (id) kSecAttrKeyType : (id) kSecAttrKeyTypeECSECPrimeRandom, (id) kSecAttrKeyClass : (id) kSecAttrKeyClassPublic };
    CFErrorRef keyCreationError = NULL;
    SecKeyRef operationalPublicKey
        = SecKeyCreateWithData((__bridge CFDataRef) publicKey, (__bridge CFDictionaryRef) attributes, &keyCreationError);
    XCTAssertNotNil((__bridge id) operationalPublicKey);
    XCTAssertNil((__bridge id) keyCreationError);

    __auto_type * operationalCertificate = [self issueOperationalCertificateForNode:@(kDeviceId)
                                                               operationalPublicKey:operationalPublicKey];
    XCTAssertNotNil(operationalCertificate);

    __auto_type * certChain = [[MTROperationalCertificateChain alloc] initWithOperationalCertificate:operationalCertificate
                                                                             intermediateCertificate:nil
                                                                                     rootCertificate:self.rootCertificate
                                                                                        adminSubject:nil];
    XCTAssertNotNil(certChain);
    completion(certChain, nil);
}
@end

@interface MTRTestExpiredCertificateIssuer : MTRTestCertificateIssuer

- (nullable instancetype)initWithRootKey:(MTRTestKeys *)key fabricID:(NSNumber *)fabricID;

@end

@implementation MTRTestExpiredCertificateIssuer
- (nullable instancetype)initWithRootKey:(MTRTestKeys *)key fabricID:(NSNumber *)fabricID
{
    // Ensure oldDate is before newDate and both are in the past.
    __auto_type * oldDate = [NSDate dateWithTimeIntervalSinceNow:-5];
    __auto_type * newDate = [NSDate dateWithTimeIntervalSinceNow:-2];
    __auto_type * validityPeriod = [[NSDateInterval alloc] initWithStartDate:oldDate endDate:newDate];

    return [super initWithRootKey:key fabricID:fabricID validityPeriod:validityPeriod];
}

@end

@interface MTRCertificateValidityTests : XCTestCase
@end

static BOOL sNeedsStackShutdown = YES;

@implementation MTRCertificateValidityTests

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
}

- (MTRBaseDevice *)commissionDeviceWithPayload:(NSString *)payloadString nodeID:(NSNumber *)nodeID
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:[NSString stringWithFormat:@"Commissioning Complete for %@", nodeID]];
    __auto_type * deviceControllerDelegate = [[MTRCertificateValidityTestControllerDelegate alloc] initWithExpectation:expectation
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

    return [MTRBaseDevice deviceWithNodeID:nodeID controller:sController];
}

- (void)initStack:(MTRTestCertificateIssuer *)certificateIssuer
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];

    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * controllerOperationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(controllerOperationalKeys);

    __auto_type * controllerOperationalCert =
        [certificateIssuer issueOperationalCertificateForNode:@(kControllerId)
                                         operationalPublicKey:controllerOperationalKeys.publicKey];
    XCTAssertNotNil(controllerOperationalCert);

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:certificateIssuer.rootKey.ipk
                                                              operationalKeypair:controllerOperationalKeys
                                                          operationalCertificate:controllerOperationalCert
                                                         intermediateCertificate:nil
                                                                 rootCertificate:certificateIssuer.rootCertificate];
    XCTAssertNotNil(params);

    params.vendorID = @(kTestVendorId);
    params.operationalCertificateIssuer = certificateIssuer;
    params.operationalCertificateIssuerQueue = dispatch_get_main_queue();

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    sConnectedDevice = [self commissionDeviceWithPayload:kOnboardingPayload nodeID:@(kDeviceId)];
}

+ (void)shutdownStack
{
    sNeedsStackShutdown = NO;

    MTRDeviceController * controller = sController;
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

- (void)test001_TestExpiredCertificates
{
    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * certificateIssuer = [[MTRTestExpiredCertificateIssuer alloc] initWithRootKey:testKeys fabricID:@(1)];
    XCTAssertNotNil(certificateIssuer);

    [self initStack:certificateIssuer];

    XCTestExpectation * toggleExpectation = [self expectationWithDescription:@"Toggle command executed"];

    __auto_type * onOffCluster = [[MTRBaseClusterOnOff alloc] initWithDevice:sConnectedDevice
                                                                  endpointID:@(1)
                                                                       queue:dispatch_get_main_queue()];
    [onOffCluster toggleWithCompletion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [toggleExpectation fulfill];
    }];
    [self waitForExpectations:@[ toggleExpectation ] timeout:kTimeoutInSeconds];

    ResetCommissionee(sConnectedDevice, dispatch_get_main_queue(), self, kTimeoutInSeconds);

    [[self class] shutdownStack];
}

@end
