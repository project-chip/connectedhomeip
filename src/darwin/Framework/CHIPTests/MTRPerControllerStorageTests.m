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

#import <Matter/Matter.h>

#import <dns_sd.h>
#import <os/lock.h>

#import "MTRDefines_Internal.h"
#import "MTRDeviceClusterData.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceStorageBehaviorConfiguration.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRDevice_Internal.h"
#import "MTRErrorTestUtils.h"
#import "MTRFabricInfoChecker.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"
#import "MTRTestResetCommissioneeHelper.h"

static const uint16_t kPairingTimeoutInSeconds = 30;
static const uint16_t kTimeoutInSeconds = 3;
static const uint16_t kSubscriptionTimeoutInSeconds = 60;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kSubscriptionPoolBaseTimeoutInSeconds = 30;

@interface MTRPerControllerStorageTestsControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@property (nonatomic, strong) NSNumber * deviceID;
@end

@implementation MTRPerControllerStorageTestsControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation newNodeID:(NSNumber *)newNodeID
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _deviceID = newNodeID;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];

    NSError * commissionError = nil;
    [controller commissionNodeWithID:self.deviceID commissioningParams:params error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete:
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface MTRPerControllerStorageTestsSuspensionDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@property (nonatomic, assign) BOOL expectedSuspensionState;
@end

@implementation MTRPerControllerStorageTestsSuspensionDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation expectedSuspensionState:(BOOL)expectedSuspensionState
{
    self = [super init];
    if (self) {
        _expectation = expectation;
        _expectedSuspensionState = expectedSuspensionState;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller
    suspendedChangedTo:(BOOL)suspended
{
    XCTAssertEqual(suspended, self.expectedSuspensionState);
    [self.expectation fulfill];
}
@end

@interface MTRPerControllerStorageTestsDeallocDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, nullable) dispatch_block_t onDevicesChanged;
@end

@implementation MTRPerControllerStorageTestsDeallocDelegate
- (void)devicesChangedForController:(MTRDeviceController *)controller
{
    if (self.onDevicesChanged) {
        self.onDevicesChanged();
    }
}
@end

@interface MTRPerControllerStorageTestsCertificateIssuer : NSObject <MTROperationalCertificateIssuer>
- (instancetype)initWithRootCertificate:(MTRCertificateDERBytes)rootCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                             signingKey:(id<MTRKeypair>)signingKey
                               fabricID:(NSNumber *)fabricID;

@property (nonatomic, readonly) MTRCertificateDERBytes rootCertificate;
@property (nonatomic, readonly, nullable) MTRCertificateDERBytes intermediateCertificate;
@property (nonatomic, readonly) id<MTRKeypair> signingKey;
@property (nonatomic, readonly) NSNumber * fabricID;

// The node ID to use for the next operational certificate we issue.  This will
// be set to null after every certificate issuance.
@property (nonatomic, nullable) NSNumber * nextNodeID;

@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;

@end

@implementation MTRPerControllerStorageTestsCertificateIssuer

- (instancetype)initWithRootCertificate:(MTRCertificateDERBytes)rootCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                             signingKey:(id<MTRKeypair>)signingKey
                               fabricID:(NSNumber *)fabricID
{
    if (!(self = [super init])) {
        return nil;
    }

    _rootCertificate = rootCertificate;
    _intermediateCertificate = intermediateCertificate;
    _signingKey = signingKey;
    _fabricID = fabricID;
    _nextNodeID = nil;
    _shouldSkipAttestationCertificateValidation = NO;

    return self;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    if (self.nextNodeID == nil) {
        completion(nil, [NSError errorWithDomain:@"TestError" code:0 userInfo:@{ @"reason" : @"nextNodeID is nil" }]);
        return;
    }

    MTRCertificateDERBytes signingCertificate;
    if (self.intermediateCertificate != nil) {
        signingCertificate = self.intermediateCertificate;
    } else {
        signingCertificate = self.rootCertificate;
    }

    __auto_type * csr = csrInfo.csr;
    XCTAssertNotNil(csr);

    NSError * error;
    __auto_type * rawPublicKey = [MTRCertificates publicKeyFromCSR:csr error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(rawPublicKey);

    if (error != nil) {
        completion(nil, error);
        return;
    }

    NSDictionary * attributes = @{
        (__bridge NSString *) kSecAttrKeyType : (__bridge NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPublic
    };
    CFErrorRef keyCreationError = NULL;
    SecKeyRef publicKey
        = SecKeyCreateWithData((__bridge CFDataRef) rawPublicKey, (__bridge CFDictionaryRef) attributes, &keyCreationError);
    XCTAssertNil((__bridge id) keyCreationError);
    XCTAssertNotNil((__bridge id) publicKey);

    __auto_type * operationalCert = [MTRCertificates createOperationalCertificate:self.signingKey
                                                               signingCertificate:signingCertificate
                                                             operationalPublicKey:publicKey
                                                                         fabricID:self.fabricID
                                                                           nodeID:self.nextNodeID
                                                            caseAuthenticatedTags:nil
                                                                            error:&error];
    // Release no-longer-needed key before we do anything else.
    CFRelease(publicKey);
    XCTAssertNil(error);
    XCTAssertNotNil(operationalCert);

    if (error != nil) {
        completion(nil, error);
        return;
    }

    __auto_type * certChain = [[MTROperationalCertificateChain alloc] initWithOperationalCertificate:operationalCert
                                                                             intermediateCertificate:self.intermediateCertificate
                                                                                     rootCertificate:self.rootCertificate
                                                                                        adminSubject:nil];
    completion(certChain, nil);
}

@end

@interface MTRPerControllerStorageTestsOperationalBrowser : NSObject

// expectedNodeID should be a 16-char uppercase hex string encoding the 64-bit
// node ID.
- (instancetype)initWithNodeID:(NSString *)expectedNodeID;
- (void)shutdown;

@property (nonatomic, readwrite) NSString * expectedNodeID;
@property (nonatomic, readwrite, nullable) XCTestExpectation * addedExpectation;
@property (nonatomic, readwrite, nullable) XCTestExpectation * removedExpectation;

- (void)onBrowse:(DNSServiceFlags)flags error:(DNSServiceErrorType)error instanceName:(const char *)instanceName;

@end

static void OnBrowse(DNSServiceRef serviceRef, DNSServiceFlags flags, uint32_t interfaceId,
    DNSServiceErrorType error, const char * name, const char * type, const char * domain, void * context)
{
    __auto_type * self = (__bridge MTRPerControllerStorageTestsOperationalBrowser *) context;
    [self onBrowse:flags error:error instanceName:name];
}

@implementation MTRPerControllerStorageTestsOperationalBrowser {
    DNSServiceRef _browseRef;
}

- (instancetype)initWithNodeID:(NSString *)expectedNodeID
{
    if (!(self = [super init])) {
        return nil;
    }

    _expectedNodeID = expectedNodeID;

    __auto_type queue = dispatch_get_main_queue();

    __auto_type err = DNSServiceBrowse(&_browseRef, /* DNSServiceFlags = */ 0, kDNSServiceInterfaceIndexAny, "_matter._tcp", "local.", OnBrowse, (__bridge void *) self);
    XCTAssertEqual(err, kDNSServiceErr_NoError);
    if (err != kDNSServiceErr_NoError) {
        return nil;
    }

    err = DNSServiceSetDispatchQueue(_browseRef, queue);
    XCTAssertEqual(err, kDNSServiceErr_NoError);
    if (err != kDNSServiceErr_NoError) {
        DNSServiceRefDeallocate(_browseRef);
        _browseRef = nil;
        return nil;
    }

    return self;
}

- (void)shutdown
{
    if (_browseRef) {
        DNSServiceRefDeallocate(_browseRef);
        _browseRef = nil;
    }
}

- (void)onBrowse:(DNSServiceFlags)flags error:(DNSServiceErrorType)error instanceName:(const char *)instanceName
{
    XCTAssertEqual(error, kDNSServiceErr_NoError);
    if (error != kDNSServiceErr_NoError) {
        DNSServiceRefDeallocate(_browseRef);
        _browseRef = nil;
        return;
    }

    __auto_type len = strlen(instanceName);
    XCTAssertEqual(len, 33); // Matter instance names are 33 chars.

    if (len != 33) {
        return;
    }

    // Skip over compressed fabric id and dash.
    // TODO: Consider checking the compressed fabric ID?  That's a bit hard to
    // do, in general, since it depends on our keys.
    const char * nodeID = &instanceName[17];
    NSString * browsedNode = [NSString stringWithUTF8String:nodeID];
    if (![browsedNode isEqual:self.expectedNodeID]) {
        return;
    }

    if (flags & kDNSServiceFlagsAdd) {
        if (self.addedExpectation) {
            XCTestExpectation * expectation = self.addedExpectation;
            self.addedExpectation = nil;
            [expectation fulfill];
        }
    } else {
        if (self.removedExpectation) {
            XCTestExpectation * expectation = self.removedExpectation;
            self.removedExpectation = nil;
            [expectation fulfill];
        }
    }
}

@end

@interface MTRPerControllerStorageTests : MTRTestCase
@end

@implementation MTRPerControllerStorageTests {
    dispatch_queue_t _storageQueue;
    BOOL _localTestStorageEnabledBeforeUnitTest;
}

- (void)setUp
{
    // Set detectLeaks true first, in case our superclass wants to do something
    // in setUp when it's set.
    self.detectLeaks = YES;

    // Per-test setup, runs before each test.
    [super setUp];
    [self setContinueAfterFailure:NO];

    // Make sure local test storage is off, because we assume that the storage
    // delegate we provide is in fact used for local storage as part of our
    // tests.
    _localTestStorageEnabledBeforeUnitTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;

    _storageQueue = dispatch_queue_create("test.storage.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
}

- (void)tearDown
{
    // Per-test teardown, runs after each test.
    [self stopFactory];
    _storageQueue = nil;

    // Restore local test storage setting to previous state.
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeUnitTest;

    [super tearDown];
}

- (void)stopFactory
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    [factory stopControllerFactory];
    XCTAssertFalse(factory.isRunning);
}

// Test helpers

- (void)commissionWithController:(MTRDeviceController *)controller newNodeID:(NSNumber *)newNodeID
{
    [self commissionWithController:controller newNodeID:newNodeID onboardingPayload:kOnboardingPayload];
}

- (void)commissionWithController:(MTRDeviceController *)controller newNodeID:(NSNumber *)newNodeID onboardingPayload:(NSString *)onboardingPayload
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * deviceControllerDelegate = [[MTRPerControllerStorageTestsControllerDelegate alloc] initWithExpectation:expectation
                                                                                                               newNodeID:newNodeID];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:onboardingPayload error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(payload);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:newNodeID error:&error];
    XCTAssertNil(error);

    [self waitForExpectations:@[ expectation ] timeout:kPairingTimeoutInSeconds];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                        caseAuthenticatedTags:(NSSet * _Nullable)caseAuthenticatedTags
                                               paramsModifier:(void (^_Nullable)(MTRDeviceControllerExternalCertificateParameters *))paramsModifier
                                                        error:(NSError * __autoreleasing *)error
{
    XCTAssertTrue(error != NULL);

    // Specify a fixed issuerID, so we get the same cert if we use the same keys.
    __auto_type * root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:error];
    XCTAssertNil(*error);
    XCTAssertNotNil(root);

    __auto_type * operationalPublicKey = [operationalKeys copyPublicKey];
    XCTAssert(operationalPublicKey != NULL);

    __auto_type * operational = [MTRCertificates createOperationalCertificate:rootKeys
                                                           signingCertificate:root
                                                         operationalPublicKey:operationalPublicKey
                                                                     fabricID:fabricID
                                                                       nodeID:nodeID
                                                        caseAuthenticatedTags:caseAuthenticatedTags
                                                                        error:error];
    CFRelease(operationalPublicKey);

    XCTAssertNil(*error);
    XCTAssertNotNil(operational);

    __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:storage
                                                                                        storageDelegateQueue:_storageQueue
                                                                                            uniqueIdentifier:storage.controllerID
                                                                                                         ipk:rootKeys.ipk
                                                                                                    vendorID:@(kTestVendorId)
                                                                                          operationalKeypair:operationalKeys
                                                                                      operationalCertificate:operational
                                                                                     intermediateCertificate:nil
                                                                                             rootCertificate:root];
    XCTAssertNotNil(params);

    if (paramsModifier) {
        paramsModifier(params);
    }

    return [[MTRDeviceController alloc] initWithParameters:params error:error];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                        caseAuthenticatedTags:(NSSet * _Nullable)caseAuthenticatedTags
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
                               concurrentSubscriptionPoolSize:(NSUInteger)concurrentSubscriptionPoolSize
                                 storageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration * _Nullable)storageBehaviorConfiguration
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                       caseAuthenticatedTags:caseAuthenticatedTags
                              paramsModifier:^(MTRDeviceControllerExternalCertificateParameters * params) {
                                  // TODO: This is only used by testControllerServer.  If that moves
                                  // elsewhere, take this back out again.
                                  params.shouldAdvertiseOperational = YES;

                                  __auto_type * ourCertificateIssuer = [[MTRPerControllerStorageTestsCertificateIssuer alloc] initWithRootCertificate:params.rootCertificate
                                                                                                                              intermediateCertificate:nil
                                                                                                                                           signingKey:rootKeys
                                                                                                                                             fabricID:fabricID];
                                  XCTAssertNotNil(ourCertificateIssuer);

                                  if (certificateIssuer) {
                                      *certificateIssuer = ourCertificateIssuer;
                                  }

                                  [params setOperationalCertificateIssuer:ourCertificateIssuer queue:dispatch_get_main_queue()];

                                  if (concurrentSubscriptionPoolSize > 0) {
                                      params.concurrentSubscriptionEstablishmentsAllowedOnThread = concurrentSubscriptionPoolSize;
                                  }

                                  if (storageBehaviorConfiguration) {
                                      params.storageBehaviorConfiguration = storageBehaviorConfiguration;
                                  }
                              }
                                       error:error];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                        caseAuthenticatedTags:(nullable NSSet *)caseAuthenticatedTags
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
{
    return [self startControllerWithRootKeys:rootKeys operationalKeys:operationalKeys fabricID:fabricID nodeID:nodeID storage:storage caseAuthenticatedTags:caseAuthenticatedTags error:error certificateIssuer:certificateIssuer concurrentSubscriptionPoolSize:0 storageBehaviorConfiguration:nil];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
                               concurrentSubscriptionPoolSize:(NSUInteger)concurrentSubscriptionPoolSize
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                       caseAuthenticatedTags:nil
                                       error:error
                           certificateIssuer:certificateIssuer
              concurrentSubscriptionPoolSize:concurrentSubscriptionPoolSize
                storageBehaviorConfiguration:nil];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
                                 storageBehaviorConfiguration:(MTRDeviceStorageBehaviorConfiguration * _Nullable)storageBehaviorConfiguration
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                       caseAuthenticatedTags:nil
                                       error:error
                           certificateIssuer:certificateIssuer
              concurrentSubscriptionPoolSize:0
                storageBehaviorConfiguration:storageBehaviorConfiguration];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                       caseAuthenticatedTags:nil
                                       error:error
                           certificateIssuer:certificateIssuer
              concurrentSubscriptionPoolSize:0
                storageBehaviorConfiguration:nil];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                        caseAuthenticatedTags:(nullable NSSet *)caseAuthenticatedTags
                                                        error:(NSError * __autoreleasing *)error
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                       caseAuthenticatedTags:caseAuthenticatedTags
                                       error:error
                           certificateIssuer:nil
              concurrentSubscriptionPoolSize:0
                storageBehaviorConfiguration:nil];
}

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                                        error:(NSError * __autoreleasing *)error
{
    return [self startControllerWithRootKeys:rootKeys
                             operationalKeys:operationalKeys
                                    fabricID:fabricID
                                      nodeID:nodeID
                                     storage:storage
                                       error:error
                           certificateIssuer:nil];
}

- (void)test001_BasicControllerStartup
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);
    XCTAssertFalse(controller.suspended);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // This was the first controller we brought up, so it should have come up
    // with fabric index 1.
    __auto_type * fabricInfoList = factory.knownFabrics;
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID,
            @"nodeID" : nodeID,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1),
        },
    ]]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test002_TryStartingTwoControllersWithSameNodeID
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Try to bring up another controller with the same identity.  This should
    // fail, since our controller is still running
    MTRDeviceController * otherController = [self startControllerWithRootKeys:rootKeys
                                                              operationalKeys:operationalKeys
                                                                     fabricID:fabricID
                                                                       nodeID:nodeID
                                                                      storage:storageDelegate
                                                                        error:&error];
    XCTAssertNil(otherController);
    XCTAssertNotNil(error);

    // Our controller should still be running.
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test003_TestTwoControllersSameUUID
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);
    XCTAssertEqual(operationalKeys.signatureCount, 0);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * fabricID = @(456);

    NSNumber * nodeID1 = @(123);

    NSError * error;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID1
                                                                 storage:storageDelegate
                                                                   error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID1);

    // Try to bring up another controller with the same uniqueIdentifier (but a different
    // node identity).  This should fail, since our controller is still running.
    NSNumber * nodeID2 = @(789);
    MTRDeviceController * otherController = [self startControllerWithRootKeys:rootKeys
                                                              operationalKeys:operationalKeys
                                                                     fabricID:fabricID
                                                                       nodeID:nodeID2
                                                                      storage:storageDelegate
                                                                        error:&error];
    XCTAssertNil(otherController);
    XCTAssertNotNil(error);

    // Our controller should still be running.
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test004_TestBasicSessionResumption
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);
    XCTAssertEqual(operationalKeys.signatureCount, 0);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Start the controller again using the same identity.  This should work,
    // because we cleared out the fabric info when the controller shut down.
    controller = [self startControllerWithRootKeys:rootKeys
                                   operationalKeys:operationalKeys
                                          fabricID:fabricID
                                            nodeID:nodeID
                                           storage:storageDelegate
                                             error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now we should have come up with fabric index 2.
    __auto_type * fabricInfoList = factory.knownFabrics;
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID,
            @"nodeID" : nodeID,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(2),
        },
    ]]);

    // Try sending an attribute read and make sure it works.
    __auto_type * device = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * onOffCluster = [[MTRBaseClusterOnOff alloc] initWithDevice:device endpointID:@(1) queue:queue];
    __auto_type * readExpectation = [self expectationWithDescription:@"Read OnOff attribute"];
    [onOffCluster readAttributeOnOffWithCompletion:^(NSNumber * value, NSError * _Nullable error) {
        XCTAssertNil(error);
        // We expect the device to be off.
        XCTAssertEqualObjects(value, @(0));
        [readExpectation fulfill];
    }];

    [self waitForExpectations:@[ readExpectation ] timeout:kTimeoutInSeconds];

    // We should have done CASE resumption, so not done any new signing using
    // our keys.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    // Reset our commissionee.
    ResetCommissionee(device, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test005_TestSessionResumptionDataClearingNodeIDChanged
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);
    XCTAssertEqual(operationalKeys.signatureCount, 0);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID1 = @(123);
    NSNumber * nodeID2 = @(246);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID1
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID1);

    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device1 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];

    // Now change ACLs so that nodeID2 has access and nodeID1 does not.
    __auto_type * admin = [[MTRAccessControlClusterAccessControlEntryStruct alloc] init];
    admin.privilege = @(MTRAccessControlEntryPrivilegeAdminister);
    admin.authMode = @(MTRAccessControlEntryAuthModeCASE);
    admin.subjects = @[ nodeID2 ];

    __auto_type * aclCluster = [[MTRBaseClusterAccessControl alloc] initWithDevice:device1 endpointID:@(0) queue:queue];

    XCTestExpectation * aclWriteExpectation = [self expectationWithDescription:@"ACLs changed so new node ID can administer"];
    [aclCluster writeAttributeACLWithValue:@[ admin ]
                                completion:^(NSError * _Nullable err) {
                                    XCTAssertNil(err);
                                    [aclWriteExpectation fulfill];
                                }];

    [self waitForExpectations:@[ aclWriteExpectation ] timeout:kTimeoutInSeconds];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // There should have been no more CASE establishment going on.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    // Bring up a controller with the same storage and keys and so on but nodeID2.
    controller = [self startControllerWithRootKeys:rootKeys
                                   operationalKeys:operationalKeys
                                          fabricID:fabricID
                                            nodeID:nodeID2
                                           storage:storageDelegate
                                             error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID2);

    // Try sending an attribute read and make sure it works.
    __auto_type * device2 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * onOffCluster = [[MTRBaseClusterOnOff alloc] initWithDevice:device2 endpointID:@(1) queue:queue];
    __auto_type * readExpectation = [self expectationWithDescription:@"Read OnOff attribute"];
    [onOffCluster readAttributeOnOffWithCompletion:^(NSNumber * value, NSError * _Nullable error) {
        XCTAssertNil(error);
        // We expect the device to be off.
        XCTAssertEqualObjects(value, @(0));
        [readExpectation fulfill];
    }];

    [self waitForExpectations:@[ readExpectation ] timeout:kTimeoutInSeconds];

    // We should note have done CASE resumption, since our identity changed.
    XCTAssertEqual(operationalKeys.signatureCount, 2);

    // Reset our commissionee.
    ResetCommissionee(device2, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test006_TestSessionResumptionDataClearingCATsChanged
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);
    XCTAssertEqual(operationalKeys.signatureCount, 0);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device1 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];

    // Now change ACLs so that CAT 0x12340001 has access and nodeID does not.
    uint32_t cat = 0x12340001;
    NSNumber * catSubject = @(0xFFFFFFFD00000000 | cat);
    __auto_type * admin = [[MTRAccessControlClusterAccessControlEntryStruct alloc] init];
    admin.privilege = @(MTRAccessControlEntryPrivilegeAdminister);
    admin.authMode = @(MTRAccessControlEntryAuthModeCASE);
    admin.subjects = @[ catSubject ];

    __auto_type * aclCluster = [[MTRBaseClusterAccessControl alloc] initWithDevice:device1 endpointID:@(0) queue:queue];

    XCTestExpectation * aclWriteExpectation = [self expectationWithDescription:@"ACLs changed so new node ID can administer"];
    [aclCluster writeAttributeACLWithValue:@[ admin ]
                                completion:^(NSError * _Nullable err) {
                                    XCTAssertNil(err);
                                    [aclWriteExpectation fulfill];
                                }];

    [self waitForExpectations:@[ aclWriteExpectation ] timeout:kTimeoutInSeconds];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // There should have been no more CASE establishment going on.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    // Bring up a controller with the same storage and keys and so on but using
    // our new CAT
    controller = [self startControllerWithRootKeys:rootKeys
                                   operationalKeys:operationalKeys
                                          fabricID:fabricID
                                            nodeID:nodeID
                                           storage:storageDelegate
                             caseAuthenticatedTags:[NSSet setWithArray:@[ @(cat) ]]
                                             error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Try sending an attribute read and make sure it works.
    __auto_type * device2 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * onOffCluster = [[MTRBaseClusterOnOff alloc] initWithDevice:device2 endpointID:@(1) queue:queue];
    __auto_type * readExpectation = [self expectationWithDescription:@"Read OnOff attribute"];
    [onOffCluster readAttributeOnOffWithCompletion:^(NSNumber * value, NSError * _Nullable error) {
        XCTAssertNil(error);
        // We expect the device to be off.
        XCTAssertEqualObjects(value, @(0));
        [readExpectation fulfill];
    }];

    [self waitForExpectations:@[ readExpectation ] timeout:kTimeoutInSeconds];

    // We should note have done CASE resumption, since our CATs changed.
    XCTAssertEqual(operationalKeys.signatureCount, 2);

    // Reset our commissionee.
    ResetCommissionee(device2, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test007_TestMultipleControllers
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);
    XCTAssertEqual(operationalKeys.signatureCount, 0);

    NSNumber * nodeID1 = @(123);
    NSNumber * nodeID2 = @(456);
    NSNumber * fabricID1 = @(1);
    NSNumber * fabricID2 = @(2);

    __auto_type * storageDelegate1 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    // Start several controllers that have distinct identities but share some
    // node/fabric IDs.
    NSError * error;
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller1 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID1
                                                                   nodeID:nodeID1
                                                                  storage:storageDelegate1
                                                                    error:&error
                                                        certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller1);
    XCTAssertTrue([controller1 isRunning]);

    __auto_type * storageDelegate2 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller2 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID1
                                                                   nodeID:nodeID2
                                                                  storage:storageDelegate2
                                                                    error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller2);
    XCTAssertTrue([controller2 isRunning]);

    __auto_type * storageDelegate3 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller3 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID2
                                                                   nodeID:nodeID1
                                                                  storage:storageDelegate3
                                                                    error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller3);
    XCTAssertTrue([controller3 isRunning]);

    // Now check our fabric table
    __auto_type * fabricInfoList = factory.knownFabrics;
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID1,
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1),
        },
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID1,
            @"nodeID" : nodeID2,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(2),
        },
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID2,
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(3),
        },
    ]]);

    // Restart controller2
    [controller2 shutdown];
    XCTAssertFalse([controller2 isRunning]);
    controller2 = [self startControllerWithRootKeys:rootKeys
                                    operationalKeys:operationalKeys
                                           fabricID:fabricID1
                                             nodeID:nodeID2
                                            storage:storageDelegate2
                                              error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller2);
    XCTAssertTrue([controller2 isRunning]);

    // Now check our fabric table again.
    fabricInfoList = factory.knownFabrics;
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID1,
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1),
        },
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID1,
            @"nodeID" : nodeID2,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(4),
        },
        @{
            @"rootPublicKey" : rootKeys.publicKeyData,
            @"vendorID" : @(kTestVendorId),
            @"fabricID" : fabricID2,
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(3),
        },
    ]]);

    // Now commission the device from controller1
    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller1 newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    // Ensure that controller2 does not have the same node ID as controller1.
    XCTAssertNotEqualObjects(controller1.controllerNodeID, controller2.controllerNodeID);

    __auto_type * device1 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller1];
    __auto_type * device2 = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller2];

    dispatch_queue_t queue = dispatch_get_main_queue();
    __auto_type * onOff1 = [[MTRBaseClusterOnOff alloc] initWithDevice:device1 endpointID:@(1) queue:queue];
    __auto_type * onOff2 = [[MTRBaseClusterOnOff alloc] initWithDevice:device2 endpointID:@(1) queue:queue];

    // Check that device1 can read the On/Off attribute
    XCTestExpectation * canReadExpectation1 = [self expectationWithDescription:@"Initial commissioner can read on/off"];
    [onOff1 readAttributeOnOffWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
        XCTAssertNil(err);
        XCTAssertEqualObjects(value, @(0));
        [canReadExpectation1 fulfill];
    }];

    [self waitForExpectations:@[ canReadExpectation1 ] timeout:kTimeoutInSeconds];

    // Check that device2 cannot read the On/Off attribute due to missing ACLs.
    XCTestExpectation * cantReadExpectation1 = [self expectationWithDescription:@"New node can't read on/off yet"];
    [onOff2 readAttributeOnOffWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
        XCTAssertNil(value);
        XCTAssertNotNil(err);
        XCTAssertTrue([MTRErrorTestUtils error:err isInteractionModelError:MTRInteractionErrorCodeUnsupportedAccess]);
        [cantReadExpectation1 fulfill];
    }];

    [self waitForExpectations:@[ cantReadExpectation1 ] timeout:kTimeoutInSeconds];

    // Now change ACLs so that device2 can read.
    __auto_type * admin1 = [[MTRAccessControlClusterAccessControlEntryStruct alloc] init];
    admin1.privilege = @(MTRAccessControlEntryPrivilegeAdminister);
    admin1.authMode = @(MTRAccessControlEntryAuthModeCASE);
    admin1.subjects = @[ controller1.controllerNodeID ];

    __auto_type * admin2 = [[MTRAccessControlClusterAccessControlEntryStruct alloc] init];
    admin2.privilege = @(MTRAccessControlEntryPrivilegeAdminister);
    admin2.authMode = @(MTRAccessControlEntryAuthModeCASE);
    admin2.subjects = @[ controller2.controllerNodeID ];

    __auto_type * acl1 = [[MTRBaseClusterAccessControl alloc] initWithDevice:device1 endpointID:@(0) queue:queue];

    XCTestExpectation * let2ReadExpectation = [self expectationWithDescription:@"ACLs changed so new node can read"];
    [acl1 writeAttributeACLWithValue:@[ admin1, admin2 ]
                          completion:^(NSError * _Nullable err) {
                              XCTAssertNil(err);
                              [let2ReadExpectation fulfill];
                          }];

    [self waitForExpectations:@[ let2ReadExpectation ] timeout:kTimeoutInSeconds];

    // Check that device2 can read the On/Off attribute
    XCTestExpectation * canReadExpectation2 = [self expectationWithDescription:@"New node can read on/off"];
    [onOff2 readAttributeOnOffWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
        XCTAssertNil(err);
        XCTAssertEqualObjects(value, @(0));
        [canReadExpectation2 fulfill];
    }];

    [self waitForExpectations:@[ canReadExpectation2 ] timeout:kTimeoutInSeconds];

    // Check that device1 can still read the On/Off attribute
    XCTestExpectation * canReadExpectation3 = [self expectationWithDescription:@"Initial commissioner can still read on/off"];
    [onOff1 readAttributeOnOffWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
        XCTAssertNil(err);
        XCTAssertEqualObjects(value, @(0));
        [canReadExpectation3 fulfill];
    }];

    [self waitForExpectations:@[ canReadExpectation3 ] timeout:kTimeoutInSeconds];

    // Check that the two devices are running on the same fabric.
    __auto_type * opCreds1 = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device1 endpoint:0 queue:queue];
    __auto_type * opCreds2 = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device2 endpoint:0 queue:queue];

    __block NSNumber * fabricIndex;
    XCTestExpectation * readFabricIndexExpectation1 =
        [self expectationWithDescription:@"Fabric index read by initial commissioner"];
    [opCreds1 readAttributeCurrentFabricIndexWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
        XCTAssertNil(readError);
        XCTAssertNotNil(value);
        fabricIndex = value;
        [readFabricIndexExpectation1 fulfill];
    }];

    [self waitForExpectations:@[ readFabricIndexExpectation1 ] timeout:kTimeoutInSeconds];

    XCTestExpectation * readFabricIndexExpectation2 = [self expectationWithDescription:@"Fabric index read by new node"];
    [opCreds2 readAttributeCurrentFabricIndexWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
        XCTAssertNil(readError);
        XCTAssertNotNil(value);
        XCTAssertEqualObjects(value, fabricIndex);
        [readFabricIndexExpectation2 fulfill];
    }];

    [self waitForExpectations:@[ readFabricIndexExpectation2 ] timeout:kTimeoutInSeconds];

    // Reset our commissionee.
    ResetCommissionee(device1, queue, self, kTimeoutInSeconds);

    [controller1 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    [controller2 shutdown];
    XCTAssertFalse([controller2 isRunning]);
    [controller3 shutdown];
    XCTAssertFalse([controller3 isRunning]);
}

- (BOOL)_array:(NSArray *)one containsSameElementsAsArray:(NSArray *)other
{
    for (id object in one) {
        if (![other containsObject:object]) {
            return NO;
        }
    }

    for (id object in other) {
        if (![one containsObject:object]) {
            return NO;
        }
    }

    return YES;
}

- (void)test008_TestDataStoreDirect
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    MTRDeviceClusterData * testClusterData1 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(1) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(111) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(112) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(113) },
    }];
    MTRDeviceClusterData * testClusterData2 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(2) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(121) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(122) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(123) },
    }];
    MTRDeviceClusterData * testClusterData3 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(3) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(211) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(212) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(213) },
    }];
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * testClusterData = @{
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(1)] : testClusterData1,
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)] : testClusterData2,
        [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(3)] : testClusterData3,
    };
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(1001)];
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(1002)];
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(1003)];

    for (NSNumber * nodeID in @[ @(1001), @(1002), @(1003) ]) {
        NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:nodeID];
        for (MTRClusterPath * path in testClusterData) {
            XCTAssertEqualObjects(testClusterData[path], dataStoreClusterData[path]);
        }
    }

    [controller.controllerDataStore clearStoredClusterDataForNodeID:@(1001)];
    XCTAssertNil([controller.controllerDataStore getStoredClusterDataForNodeID:@(1001)]);
    for (NSNumber * nodeID in @[ @(1002), @(1003) ]) {
        NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:nodeID];
        for (MTRClusterPath * path in testClusterData) {
            XCTAssertEqualObjects(testClusterData[path], dataStoreClusterData[path]);
        }
    }

    [controller.controllerDataStore clearAllStoredClusterData];
    for (NSNumber * nodeID in @[ @(1001), @(1002), @(1003) ]) {
        XCTAssertNil([controller.controllerDataStore getStoredClusterDataForNodeID:nodeID]);
    }

    // Test MTRDeviceControllerDataStore _pruneEmptyStoredAttributesBranches
    //  - Clear cache
    //  - Store some cluster data
    //  - Manually delete parts of the data from the test storage delegate
    //  - Call _pruneEmptyStoredAttributesBranches
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(2001)];
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(2002)];

    NSString * testClusterIndexKey1 = [controller.controllerDataStore _clusterIndexKeyForNodeID:@(2001) endpointID:@(1)];
    NSString * testClusterIndexKey2 = [controller.controllerDataStore _clusterIndexKeyForNodeID:@(2001) endpointID:@(2)];
    NSString * testClusterIndexKey3 = [controller.controllerDataStore _clusterIndexKeyForNodeID:@(2002) endpointID:@(1)];
    NSString * testClusterIndexKey4 = [controller.controllerDataStore _clusterIndexKeyForNodeID:@(2002) endpointID:@(2)];
    NSString * testEndpointIndexKey1 = [controller.controllerDataStore _endpointIndexKeyForNodeID:@(2001)];
    NSString * testEndpointIndexKey2 = [controller.controllerDataStore _endpointIndexKeyForNodeID:@(2002)];
    NSString * testNodeIndexKey = @"attrCacheNodeIndex";

    // store is async, so remove on the same queue to ensure order
    dispatch_sync(_storageQueue, ^{
        // Ensure that the indices we expect are populated.
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testClusterIndexKey1 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testClusterIndexKey2 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testClusterIndexKey3 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testClusterIndexKey4 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testEndpointIndexKey1 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testEndpointIndexKey2 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);
        XCTAssertNotNil([storageDelegate controller:controller valueForKey:testNodeIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared]);

        // Remove all three MTRDeviceClusterData for node 2001
        NSString * testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2001) endpointID:@(1) clusterID:@(1)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
        testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2001) endpointID:@(1) clusterID:@(2)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
        testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2001) endpointID:@(2) clusterID:@(3)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];

        // Remove the two MTRDeviceClusterData under endpoint 1 for node 2002
        testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2002) endpointID:@(1) clusterID:@(1)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
        testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2002) endpointID:@(1) clusterID:@(2)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    });

    [controller.controllerDataStore unitTestPruneEmptyStoredClusterDataBranches];

    // Now check the indexes are pruned.  There should be no more cluster
    // indices or endpoint indices for node 2001.
    id testClusterIndex1 = [storageDelegate controller:controller valueForKey:testClusterIndexKey1 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex1);
    id testClusterIndex2 = [storageDelegate controller:controller valueForKey:testClusterIndexKey2 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex2);
    id testEndpointIndex1 = [storageDelegate controller:controller valueForKey:testEndpointIndexKey1 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testEndpointIndex1);

    // There should be no more cluster index for endpoint 1 for node 2, but
    // we should still have a cluster index for endpoint 2, and an endpoint index.
    id testClusterIndex3 = [storageDelegate controller:controller valueForKey:testClusterIndexKey3 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex3);
    id testClusterIndex4 = [storageDelegate controller:controller valueForKey:testClusterIndexKey4 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNotNil(testClusterIndex4);
    id testEndpointIndex2 = [storageDelegate controller:controller valueForKey:testEndpointIndexKey2 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNotNil(testClusterIndex4);

    // We should still have a node index.
    id testNodeIndex = [storageDelegate controller:controller valueForKey:testNodeIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNotNil(testNodeIndex);

    // Again, remove on the storage queue to ensure order.
    dispatch_sync(_storageQueue, ^{
        NSString * testClusterDataKey = [controller.controllerDataStore _clusterDataKeyForNodeID:@(2002) endpointID:@(2) clusterID:@(3)];
        [storageDelegate controller:controller removeValueForKey:testClusterDataKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    });

    [controller.controllerDataStore unitTestPruneEmptyStoredClusterDataBranches];

    // All the indices should be pruned now.
    testClusterIndex4 = [storageDelegate controller:controller valueForKey:testClusterIndexKey4 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex4);
    testEndpointIndex2 = [storageDelegate controller:controller valueForKey:testEndpointIndexKey2 securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex4);
    testNodeIndex = [storageDelegate controller:controller valueForKey:testNodeIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testNodeIndex);

    // Now test bulk write
    MTRClusterPath * bulkTestClusterPath11 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(1)];
    MTRDeviceClusterData * bulkTestClusterData11 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(11) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(111) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(112) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(113) },
    }];
    MTRClusterPath * bulkTestClusterPath12 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)];
    MTRDeviceClusterData * bulkTestClusterData12 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(12) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(121) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(122) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(123) },
    }];
    MTRClusterPath * bulkTestClusterPath13 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(3)];
    MTRDeviceClusterData * bulkTestClusterData13 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(13) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(131) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(132) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(133) },
    }];
    MTRClusterPath * bulkTestClusterPath21 = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(1)];
    MTRDeviceClusterData * bulkTestClusterData21 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(21) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(211) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(212) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(213) },
    }];
    MTRClusterPath * bulkTestClusterPath22 = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(2)];
    MTRDeviceClusterData * bulkTestClusterData22 = [[MTRDeviceClusterData alloc] initWithDataVersion:@(22) attributes:@{
        @(1) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(221) },
        @(2) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(222) },
        @(3) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(223) },
    }];
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * bulkTestClusterDataDictionary = @{
        bulkTestClusterPath11 : bulkTestClusterData11,
        bulkTestClusterPath12 : bulkTestClusterData12,
        bulkTestClusterPath13 : bulkTestClusterData13,
        bulkTestClusterPath21 : bulkTestClusterData21,
        bulkTestClusterPath22 : bulkTestClusterData22,
    };

    // Manually construct what the total dictionary should look like
    NSDictionary<NSString *, id<NSSecureCoding>> * testBulkValues = @{
        @"attrCacheNodeIndex" : @[ @(3001) ],
        [controller.controllerDataStore _endpointIndexKeyForNodeID:@(3001)] : @[ @(1), @(2) ],
        [controller.controllerDataStore _clusterIndexKeyForNodeID:@(3001) endpointID:@(1)] : @[ @(1), @(2), @(3) ],
        [controller.controllerDataStore _clusterIndexKeyForNodeID:@(3001) endpointID:@(2)] : @[ @(1), @(2) ],
        [controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(1)] : bulkTestClusterData11,
        [controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(2)] : bulkTestClusterData12,
        [controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(3)] : bulkTestClusterData13,
        [controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(2) clusterID:@(1)] : bulkTestClusterData21,
        [controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(2) clusterID:@(2)] : bulkTestClusterData22,
    };
    // Bulk store with delegate
    dispatch_sync(_storageQueue, ^{
        [storageDelegate controller:controller storeValues:testBulkValues securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    });
    // Since we messed with the node index, tell the data store to re-sync it's cache.
    [controller.controllerDataStore unitTestRereadNodeIndex];
    // Verify that the store resulted in the correct values
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:@(3001)];
    XCTAssertEqualObjects(dataStoreClusterData, bulkTestClusterDataDictionary);

    // clear information before the next test
    [controller.controllerDataStore clearStoredClusterDataForNodeID:@(3001)];

    // Now test bulk store through data store
    [controller.controllerDataStore storeClusterData:bulkTestClusterDataDictionary forNodeID:@(3001)];
    dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:@(3001)];
    XCTAssertEqualObjects(dataStoreClusterData, bulkTestClusterDataDictionary);

    // Now test bulk read directly from storage delegate
    NSDictionary<NSString *, id<NSSecureCoding>> * dataStoreBulkValues = [storageDelegate valuesForController:controller securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    // Due to dictionary enumeration in storeClusterData:forNodeID:, the elements could be stored in a different order, but still be valid and equivalent
    XCTAssertTrue(([self _array:(NSArray *) dataStoreBulkValues[[controller.controllerDataStore _endpointIndexKeyForNodeID:@(3001)]] containsSameElementsAsArray:@[ @(1), @(2) ]]));
    XCTAssertTrue(([self _array:(NSArray *) dataStoreBulkValues[[controller.controllerDataStore _clusterIndexKeyForNodeID:@(3001) endpointID:@(1)]] containsSameElementsAsArray:@[ @(1), @(2), @(3) ]]));
    XCTAssertTrue(([self _array:(NSArray *) dataStoreBulkValues[[controller.controllerDataStore _clusterIndexKeyForNodeID:@(3001) endpointID:@(2)]] containsSameElementsAsArray:@[ @(1), @(2) ]]));
    XCTAssertEqualObjects(dataStoreBulkValues[[controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(1)]], bulkTestClusterData11);
    XCTAssertEqualObjects(dataStoreBulkValues[[controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(2)]], bulkTestClusterData12);
    XCTAssertEqualObjects(dataStoreBulkValues[[controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(1) clusterID:@(3)]], bulkTestClusterData13);
    XCTAssertEqualObjects(dataStoreBulkValues[[controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(2) clusterID:@(1)]], bulkTestClusterData21);
    XCTAssertEqualObjects(dataStoreBulkValues[[controller.controllerDataStore _clusterDataKeyForNodeID:@(3001) endpointID:@(2) clusterID:@(2)]], bulkTestClusterData22);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)doDataStoreMTRDeviceTestWithStorageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate disableStorageBehaviorOptimization:(BOOL)disableStorageBehaviorOptimization
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceStorageBehaviorConfiguration * storageBehaviorConfiguration = nil;
    if (disableStorageBehaviorOptimization) {
        storageBehaviorConfiguration = [MTRDeviceStorageBehaviorConfiguration configurationWithStorageBehaviorOptimizationDisabled];
    }
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer
                                            storageBehaviorConfiguration:storageBehaviorConfiguration];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    delegate.onReportEnd = ^{
        [subscriptionExpectation fulfill];
    };

    __block BOOL onDeviceCachePrimedCalled = NO;
    delegate.onDeviceCachePrimed = ^{
        onDeviceCachePrimedCalled = YES;
    };

    // Verify that initially (before we have ever subscribed while using this
    // datastore) the device has no estimate for subscription latency.
    XCTAssertNil(device.estimatedSubscriptionLatency);

    // And that the device cache is not primed.
    XCTAssertFalse(device.deviceCachePrimed);

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];

    XCTAssertTrue(device.deviceCachePrimed);
    XCTAssertTrue(onDeviceCachePrimedCalled);

    NSUInteger dataStoreValuesCount = 0;
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:deviceID];
    for (MTRClusterPath * path in dataStoreClusterData) {
        MTRDeviceClusterData * data = dataStoreClusterData[path];
        for (NSNumber * attributeID in data.attributes) {
            dataStoreValuesCount++;
            NSDictionary * dataValue = data.attributes[attributeID];
            NSDictionary * dataValueFromMTRDevice = [device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:attributeID params:nil];
            XCTAssertTrue([device _attributeDataValue:dataValue isEqualToDataValue:dataValueFromMTRDevice]);
        }
    }

    // Now force the removal of the object from controller to test reloading read cache from storage
    [controller removeDevice:device];

    // Verify the new device is initialized with the same values
    __auto_type * newDevice = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    NSUInteger storedAttributeDifferFromMTRDeviceCount = 0;
    for (MTRClusterPath * path in dataStoreClusterData) {
        MTRDeviceClusterData * data = dataStoreClusterData[path];
        for (NSNumber * attributeID in data.attributes) {
            NSDictionary * dataValue = data.attributes[attributeID];
            NSDictionary * dataValueFromMTRDevice = [newDevice readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:attributeID params:nil];
            if (![newDevice _attributeDataValue:dataValue isEqualToDataValue:dataValueFromMTRDevice]) {
                storedAttributeDifferFromMTRDeviceCount++;
            }
        }
    }

    // Only test that 90% of attributes are the same because there are some changing attributes each time (UTC time, for example)
    //   * With all-clusters-app as of 2024-02-10, about 1.476% of attributes change.
    double storedAttributeDifferFromMTRDevicePercentage = storedAttributeDifferFromMTRDeviceCount * 100.0 / dataStoreValuesCount;
    XCTAssertTrue(storedAttributeDifferFromMTRDevicePercentage < 10.0);

    // Check that the new device has an estimated subscription latency.
    XCTAssertNotNil(device.estimatedSubscriptionLatency);

    // And that it's already primed.
    XCTAssertTrue(device.deviceCachePrimed);

    // Check that this estimate is positive, since subscribing must have taken
    // some time.
    XCTAssertGreaterThan(device.estimatedSubscriptionLatency.doubleValue, 0);

    // Now set up new delegate for the new device and verify that once subscription reestablishes, the data version filter loaded from storage will work
    __auto_type * newDelegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * newDeviceSubscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up for new device"];
    XCTestExpectation * newDeviceGotClusterDataPersisted = nil;
    if (!disableStorageBehaviorOptimization) {
        newDeviceGotClusterDataPersisted = [self expectationWithDescription:@"Cluster data persisted on new device"];
    }

    newDelegate.onReportEnd = ^{
        [newDeviceSubscriptionExpectation fulfill];
    };
    newDelegate.onClusterDataPersisted = ^{
        [newDeviceGotClusterDataPersisted fulfill];
    };

    __block BOOL newOnDeviceCachePrimedCalled = NO;
    newDelegate.onDeviceCachePrimed = ^{
        newOnDeviceCachePrimedCalled = YES;
    };

    [newDevice setDelegate:newDelegate queue:queue];

    [self waitForExpectations:@[ newDeviceSubscriptionExpectation ] timeout:60];
    if (!disableStorageBehaviorOptimization) {
        [self waitForExpectations:@[ newDeviceGotClusterDataPersisted ] timeout:60];
        newDelegate.onClusterDataPersisted = nil;
    }
    newDelegate.onReportEnd = nil;

    XCTAssertFalse(newOnDeviceCachePrimedCalled);

    // 1) MTRDevice actually gets some attributes reported more than once
    // 2) Some attributes do change on resubscribe
    //   * With all-clusts-app as of 2024-02-10, out of 1287 persisted attributes, still 450 attributes were reported with filter
    // And so conservatively, assert that data version filters save at least 300 entries.
    NSUInteger storedAttributeCountDifferenceFromMTRDeviceReport = dataStoreValuesCount - [device unitTestAttributesReportedSinceLastCheck];
    XCTAssertTrue(storedAttributeCountDifferenceFromMTRDeviceReport > 300);

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test009_TestDataStoreMTRDevice
{
    [self doDataStoreMTRDeviceTestWithStorageDelegate:[[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]] disableStorageBehaviorOptimization:NO];
}

- (void)test010_TestDataStoreMTRDeviceWithBulkReadWrite
{
    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    // First do the same test as the above
    [self doDataStoreMTRDeviceTestWithStorageDelegate:storageDelegate disableStorageBehaviorOptimization:NO];

    // Then restart controller with same storage and see that bulk read through MTRDevice initialization works

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // No need to commission device - just look at device count
    NSDictionary<NSNumber *, NSNumber *> * deviceAttributeCounts = [controller unitTestGetDeviceAttributeCounts];
    XCTAssertTrue(deviceAttributeCounts.count > 0);
    NSUInteger totalAttributes = 0;
    for (NSNumber * nodeID in deviceAttributeCounts) {
        totalAttributes += deviceAttributeCounts[nodeID].unsignedIntegerValue;
    }
    XCTAssertTrue(totalAttributes > 300);

    // Now try forgetting this device and make sure all the info we had for it
    // goes away.
    NSNumber * deviceID = @(17);
    __auto_type * dataStore = controller.controllerDataStore;
    XCTAssertNotNil(deviceAttributeCounts[deviceID]);
    XCTAssertNotNil([dataStore findResumptionInfoByNodeID:deviceID]);
    XCTAssertNotNil([dataStore getStoredDeviceDataForNodeID:deviceID]);
    XCTAssertNotNil([dataStore getStoredClusterDataForNodeID:deviceID]);
    __auto_type * nodesWithStoredData = controller.nodesWithStoredData;
    XCTAssertTrue([nodesWithStoredData containsObject:deviceID]);
    XCTAssertEqualObjects(nodesWithStoredData, dataStore.nodesWithStoredData);
    XCTAssertEqualObjects(nodesWithStoredData, deviceAttributeCounts.allKeys);

    [controller forgetDeviceWithNodeID:deviceID];
    deviceAttributeCounts = [controller unitTestGetDeviceAttributeCounts];
    XCTAssertNil(deviceAttributeCounts[deviceID]);
    XCTAssertNil([dataStore findResumptionInfoByNodeID:deviceID]);
    XCTAssertNil([dataStore getStoredDeviceDataForNodeID:deviceID]);
    XCTAssertNil([dataStore getStoredClusterDataForNodeID:deviceID]);
    nodesWithStoredData = controller.nodesWithStoredData;
    XCTAssertFalse([nodesWithStoredData containsObject:deviceID]);
    XCTAssertEqualObjects(nodesWithStoredData, dataStore.nodesWithStoredData);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test011_TestDataStoreMTRDeviceWithStorageBehaviorOptimizationDisabled
{
    [self doDataStoreMTRDeviceTestWithStorageDelegate:[[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]] disableStorageBehaviorOptimization:YES];
}

// TODO: Factor out startControllerWithRootKeys into a test helper, move these
// suspension tests to a different file.
- (void)test012_startSuspended
{
    // Needs to match the 888 == 0x378 for the node ID below.
    __auto_type * operationalBrowser = [[MTRPerControllerStorageTestsOperationalBrowser alloc] initWithNodeID:@"0000000000000378"];
    XCTestExpectation * initialAdvertisingExpectation = [self expectationWithDescription:@"Controller advertising initially"];
    operationalBrowser.addedExpectation = initialAdvertisingExpectation;
    initialAdvertisingExpectation.inverted = YES; // We should not in fact advertise, since we are suspended.

    NSError * error;
    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    __auto_type * controller = [self startControllerWithRootKeys:[[MTRTestKeys alloc] init]
                                                 operationalKeys:[[MTRTestKeys alloc] init]
                                                        fabricID:@555
                                                          nodeID:@888
                                                         storage:storageDelegate
                                           caseAuthenticatedTags:nil
                                                  paramsModifier:^(MTRDeviceControllerExternalCertificateParameters * params) {
                                                      params.startSuspended = YES;
                                                      params.shouldAdvertiseOperational = YES;
                                                  }
                                                           error:&error];

    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue(controller.running);
    XCTAssertTrue(controller.suspended);

    // Test that a suspended controller can't set up a commissioning session.
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(payload);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(17) error:&error];
    XCTAssertNotNil(error);

    [self waitForExpectations:@[ initialAdvertisingExpectation ] timeout:kTimeoutInSeconds];

    [controller shutdown];

    [operationalBrowser shutdown];
}

- (void)test013_suspendDevices
{
    // getMTRDevice uses "123" for the node ID of the controller, which is hex 0x7B
    __auto_type * operationalBrowser = [[MTRPerControllerStorageTestsOperationalBrowser alloc] initWithNodeID:@"000000000000007B"];
    XCTestExpectation * initialAdvertisingExpectation = [self expectationWithDescription:@"Controller advertising initially"];
    operationalBrowser.addedExpectation = initialAdvertisingExpectation;

    NSNumber * deviceID = @(17);
    __auto_type * device = [self getMTRDevice:deviceID];
    __auto_type * controller = device.deviceController;

    XCTAssertFalse(controller.suspended);

    [self waitForExpectations:@[ initialAdvertisingExpectation ] timeout:kTimeoutInSeconds];

    __auto_type queue = dispatch_get_main_queue();
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * initialSubscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];
    XCTestExpectation * initialReachableExpectation = [self expectationWithDescription:@"Device initially became reachable"];
    XCTestExpectation * initialUnreachableExpectation = [self expectationWithDescription:@"Device initially became unreachable"];
    initialUnreachableExpectation.inverted = YES;

    delegate.onReachable = ^{
        [initialReachableExpectation fulfill];
    };

    delegate.onNotReachable = ^{
        // We do not expect to land here.
        [initialUnreachableExpectation fulfill];
    };

    delegate.onReportEnd = ^{
        [initialSubscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];
    [self waitForExpectations:@[ initialReachableExpectation, initialSubscriptionExpectation ] timeout:kSubscriptionTimeoutInSeconds];
    // Separately wait for the unreachable bit, so we don't end up waiting kSubscriptionTimeoutInSeconds
    // seconds for it.
    [self waitForExpectations:@[ initialUnreachableExpectation ] timeout:0];

    // Test that sending a command works.  Clear the delegate's onReportEnd
    // first, so reports from the command don't trigger it.
    delegate.onReportEnd = nil;
    XCTestExpectation * toggle1Expectation = [self expectationWithDescription:@"toggle 1"];
    __auto_type * cluster = [[MTRClusterOnOff alloc] initWithDevice:device endpointID:@(1) queue:queue];
    [cluster toggleWithExpectedValues:nil expectedValueInterval:nil completion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [toggle1Expectation fulfill];
    }];

    [self waitForExpectations:@[ toggle1Expectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * becameUnreachableExpectation = [self expectationWithDescription:@"Device became unreachable"];
    delegate.onNotReachable = ^{
        [becameUnreachableExpectation fulfill];
    };

    XCTestExpectation * suspendedExpectation = [self expectationWithDescription:@"Controller has been suspended"];
    __auto_type * suspensionDelegate = [[MTRPerControllerStorageTestsSuspensionDelegate alloc] initWithExpectation:suspendedExpectation expectedSuspensionState:YES];
    [controller addDeviceControllerDelegate:suspensionDelegate queue:queue];

    XCTestExpectation * browseStoppedExpectation = [self expectationWithDescription:@"Operational browse has stopped"];
    MTRSetLogCallback(MTRLogTypeProgress, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if ([message containsString:@"stopping persistent operational browse"]) {
            [browseStoppedExpectation fulfill];
        }
    });

    XCTestExpectation * advertisingStoppedExpectation = [self expectationWithDescription:@"Controller stopped advertising"];
    operationalBrowser.removedExpectation = advertisingStoppedExpectation;

    [controller suspend];
    XCTAssertTrue(controller.suspended);

    // Test that sending a command no longer works.
    XCTestExpectation * toggle2Expectation = [self expectationWithDescription:@"toggle 2"];
    [cluster toggleWithExpectedValues:nil expectedValueInterval:nil completion:^(NSError * _Nullable error) {
        XCTAssertNotNil(error);
        [toggle2Expectation fulfill];
    }];

    [self waitForExpectations:@[ becameUnreachableExpectation, toggle2Expectation, suspendedExpectation, browseStoppedExpectation, advertisingStoppedExpectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * newSubscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up again"];
    XCTestExpectation * newReachableExpectation = [self expectationWithDescription:@"Device became reachable again"];
    delegate.onReachable = ^{
        [newReachableExpectation fulfill];
    };

    delegate.onReportEnd = ^{
        [newSubscriptionExpectation fulfill];
    };

    XCTestExpectation * resumedExpectation = [self expectationWithDescription:@"Controller has been resumed"];
    suspensionDelegate.expectation = resumedExpectation;
    suspensionDelegate.expectedSuspensionState = NO;

    XCTestExpectation * browseRestartedExpectation = [self expectationWithDescription:@"Operational browse has re-started"];
    MTRSetLogCallback(MTRLogTypeProgress, ^(MTRLogType type, NSString * moduleName, NSString * message) {
        if ([message containsString:@"trying to start persistent operational browse"]) {
            [browseRestartedExpectation fulfill];
        }
    });

    XCTestExpectation * advertisingResumedExpectation = [self expectationWithDescription:@"Controller resumed advertising"];
    operationalBrowser.addedExpectation = advertisingResumedExpectation;

    [controller resume];
    XCTAssertFalse(controller.suspended);

    [self waitForExpectations:@[ newSubscriptionExpectation, newReachableExpectation, resumedExpectation, browseRestartedExpectation, advertisingResumedExpectation ] timeout:kSubscriptionTimeoutInSeconds];

    MTRSetLogCallback(MTRLogTypeProgress, nil);

    // Test that sending a command works again.  Clear the delegate's onReportEnd
    // first, so reports from the command don't trigger it.
    delegate.onReportEnd = nil;
    XCTestExpectation * toggle3Expectation = [self expectationWithDescription:@"toggle 3"];
    [cluster toggleWithExpectedValues:nil expectedValueInterval:nil completion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [toggle3Expectation fulfill];
    }];

    [self waitForExpectations:@[ toggle3Expectation ] timeout:kTimeoutInSeconds];

    [controller removeDevice:device];
    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];

    [operationalBrowser shutdown];
}

- (void)test014_TestDataStoreMTRDeviceInvalidateFlush
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;

    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceStorageBehaviorConfiguration * storageBehaviorConfiguration = [MTRDeviceStorageBehaviorConfiguration configurationWithDefaultStorageBehavior];
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer
                                            storageBehaviorConfiguration:storageBehaviorConfiguration];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(17);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];

    delegate.skipSetupSubscription = YES;

    // Read the base storage key count (case session resumption etc.)
    NSUInteger baseStorageKeyCount = storageDelegate.count;

    [device setDelegate:delegate queue:queue];

    NSArray<NSDictionary<NSString *, id> *> * attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(1) attributeID:@(1)],
        MTRDataKey : @ {
            MTRDataVersionKey : @(1),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(1),
        }
    } ];

    // Inject first report as priming report, which gets persisted immediately
    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    // No additional entries immediately after injected report
    XCTAssertEqual(storageDelegate.count, baseStorageKeyCount);

    sleep(1);

    // Verify priming report persisted before hitting storage delay
    XCTAssertGreaterThan(storageDelegate.count, baseStorageKeyCount);
    // Now set the base count to the after-priming number
    baseStorageKeyCount = storageDelegate.count;

    NSArray<NSDictionary<NSString *, id> *> * attributeReport2 = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(2) attributeID:@(2)],
        MTRDataKey : @ {
            MTRDataVersionKey : @(2),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(2),
        }
    } ];

    // Inject second report with different cluster
    [device unitTestInjectAttributeReport:attributeReport2 fromSubscription:YES];

    sleep(1);

    // No additional entries a second after report - under storage delay
    XCTAssertEqual(storageDelegate.count, baseStorageKeyCount);

    // Immediately shut down controller and force flush to storage
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Make sure there are more than base count entries
    XCTAssertGreaterThan(storageDelegate.count, baseStorageKeyCount);

    // Now restart controller to decommission the device
    controller = [self startControllerWithRootKeys:rootKeys
                                   operationalKeys:operationalKeys
                                          fabricID:fabricID
                                            nodeID:nodeID
                                           storage:storageDelegate
                                             error:&error
                                 certificateIssuer:&certificateIssuer
                      storageBehaviorConfiguration:storageBehaviorConfiguration];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
}

// TODO: This might want to go in a separate test file, with some shared setup
// across multiple tests, maybe.  Would need to factor out
// startControllerWithRootKeys into a test helper.
- (void)testControllerServer
{
#ifdef DEBUG
    // Force our controllers to only advertise on localhost, to avoid DNS-SD
    // crosstalk.
    [MTRDeviceController forceLocalhostAdvertisingOnly];
#endif // DEBUG

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    NSNumber * fabricID = @(456);

    __auto_type * operationalKeysServer = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeysServer);

    __auto_type * storageDelegateServer = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    XCTAssertNotNil(storageDelegateServer);

    NSNumber * nodeIDServer = @(123);

    NSError * error;
    MTRDeviceController * controllerServer = [self startControllerWithRootKeys:rootKeys
                                                               operationalKeys:operationalKeysServer
                                                                      fabricID:fabricID
                                                                        nodeID:nodeIDServer
                                                                       storage:storageDelegateServer
                                                                         error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controllerServer);
    XCTAssertTrue([controllerServer isRunning]);
    XCTAssertEqualObjects(controllerServer.controllerNodeID, nodeIDServer);

    __auto_type * endpointId1 = @(10);
    __auto_type * endpointId2 = @(20);
    __auto_type * endpointId3 = @(30);
    __auto_type * clusterId1 = @(0xFFF1FC02);
    __auto_type * clusterId2 = @(0xFFF1FC10);
    __auto_type * clusterRevision1 = @(3);
    __auto_type * clusterRevision2 = @(4);
    __auto_type * attributeId1 = @(0);
    __auto_type * attributeId2 = @(0xFFF10002);

    __auto_type * unsignedIntValue1 = @{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(5),
    };

    __auto_type * unsignedIntValue2 = @{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(7),
    };

    __auto_type * structValue1 = @{
        MTRTypeKey : MTRStructureValueType,
        MTRValueKey : @[
            @{
                MTRContextTagKey : @(1),
                MTRDataKey : @ {
                    MTRTypeKey : MTRUnsignedIntegerValueType,
                    MTRValueKey : @(1),
                },
            },
            @{
                MTRContextTagKey : @(2),
                MTRDataKey : @ {
                    MTRTypeKey : MTRUTF8StringValueType,
                    MTRValueKey : @"struct1",
                },
            },
        ],
    };

    __auto_type * structValue2 = @{
        MTRTypeKey : MTRStructureValueType,
        MTRValueKey : @[
            @{
                MTRContextTagKey : @(1),
                MTRDataKey : @ {
                    MTRTypeKey : MTRUnsignedIntegerValueType,
                    MTRValueKey : @(2),
                },
            },
            @{
                MTRContextTagKey : @(2),
                MTRDataKey : @ {
                    MTRTypeKey : MTRUTF8StringValueType,
                    MTRValueKey : @"struct2",
                },
            },
        ],
    };

    __auto_type * listOfStructsValue1 = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[
            @{
                MTRDataKey : structValue1,
            },
            @{
                MTRDataKey : structValue2,
            },
        ],
    };

    __auto_type responsePathFromRequestPath = ^(MTRAttributeRequestPath * path) {
        return [MTRAttributePath attributePathWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute];
    };

    // Set up an endpoint on the server.
    __auto_type * deviceType1 = [[MTRDeviceTypeRevision alloc] initWithDeviceTypeID:@(0xFFF10001) revision:@(1)];
    XCTAssertNotNil(deviceType1);

    __auto_type * endpoint1 = [[MTRServerEndpoint alloc] initWithEndpointID:endpointId1 deviceTypes:@[ deviceType1 ]];
    XCTAssertNotNil(endpoint1);

    __auto_type * cluster1 = [[MTRServerCluster alloc] initWithClusterID:clusterId1 revision:clusterRevision1];
    XCTAssertNotNil(cluster1);

    __auto_type * cluster2 = [[MTRServerCluster alloc] initWithClusterID:clusterId2 revision:clusterRevision2];
    XCTAssertNotNil(cluster1);

    __auto_type * attribute1 = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:attributeId1 initialValue:unsignedIntValue1 requiredPrivilege:MTRAccessControlEntryPrivilegeView];
    XCTAssertNotNil(attribute1);
    __auto_type * attribute1RequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:endpointId1
                                                                                   clusterID:clusterId1
                                                                                 attributeID:attributeId1];
    XCTAssertNotNil(attribute1RequestPath);
    __auto_type * attribute1ResponsePath = responsePathFromRequestPath(attribute1RequestPath);
    XCTAssertNotNil(attribute1ResponsePath);

    __auto_type * attribute2 = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:attributeId2 initialValue:listOfStructsValue1 requiredPrivilege:MTRAccessControlEntryPrivilegeManage];
    XCTAssertNotNil(attribute2);
    __auto_type * attribute2RequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:endpointId1
                                                                                   clusterID:clusterId2
                                                                                 attributeID:attributeId2];
    XCTAssertNotNil(attribute2RequestPath);
    __auto_type * attribute2ResponsePath = responsePathFromRequestPath(attribute2RequestPath);
    XCTAssertNotNil(attribute2ResponsePath);

    __auto_type * attribute3 = [[MTRServerAttribute alloc] initReadonlyAttributeWithID:attributeId2 initialValue:unsignedIntValue1 requiredPrivilege:MTRAccessControlEntryPrivilegeOperate];
    XCTAssertNotNil(attribute3);
    __auto_type * attribute3RequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:endpointId1
                                                                                   clusterID:clusterId1
                                                                                 attributeID:attributeId2];
    XCTAssertNotNil(attribute3RequestPath);
    __auto_type * attribute3ResponsePath = responsePathFromRequestPath(attribute3RequestPath);
    XCTAssertNotNil(attribute3ResponsePath);

    XCTAssertTrue([cluster1 addAttribute:attribute1]);
    XCTAssertTrue([cluster1 addAttribute:attribute3]);

    XCTAssertTrue([cluster2 addAttribute:attribute2]);

    XCTAssertTrue([endpoint1 addServerCluster:cluster1]);
    XCTAssertTrue([endpoint1 addServerCluster:cluster2]);

    [endpoint1 addAccessGrant:[MTRAccessGrant accessGrantForAllNodesWithPrivilege:MTRAccessControlEntryPrivilegeView]];

    XCTAssertTrue([controllerServer addServerEndpoint:endpoint1]);

    __auto_type * endpoint2 = [[MTRServerEndpoint alloc] initWithEndpointID:endpointId2 deviceTypes:@[ deviceType1 ]];
    XCTAssertNotNil(endpoint2);
    // Should be able to add this endpoint as well.
    XCTAssertTrue([controllerServer addServerEndpoint:endpoint2]);

    __auto_type * endpoint3 = [[MTRServerEndpoint alloc] initWithEndpointID:endpointId2 deviceTypes:@[ deviceType1 ]];
    XCTAssertNotNil(endpoint3);
    // Should not be able to add this endpoint, since it's got a duplicate
    // endpoint id.
    XCTAssertFalse([controllerServer addServerEndpoint:endpoint3]);

    __auto_type * operationalKeysClient = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeysClient);

    __auto_type * storageDelegateClient = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    XCTAssertNotNil(storageDelegateClient);

    NSNumber * nodeIDClient = @(789);

    MTRDeviceController * controllerClient = [self startControllerWithRootKeys:rootKeys
                                                               operationalKeys:operationalKeysClient
                                                                      fabricID:fabricID
                                                                        nodeID:nodeIDClient
                                                                       storage:storageDelegateClient
                                                                         error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controllerClient);
    XCTAssertTrue([controllerClient isRunning]);
    XCTAssertEqualObjects(controllerClient.controllerNodeID, nodeIDClient);

    __auto_type * endpoint4 = [[MTRServerEndpoint alloc] initWithEndpointID:endpointId2 deviceTypes:@[ deviceType1 ]];
    XCTAssertNotNil(endpoint4);
    // Should not be able to add this endpoint, since it's got a duplicate
    // endpoint id, even though we are adding on a different controller.
    XCTAssertFalse([controllerClient addServerEndpoint:endpoint4]);

    __auto_type * endpoint5 = [[MTRServerEndpoint alloc] initWithEndpointID:endpointId3 deviceTypes:@[ deviceType1 ]];
    XCTAssertNotNil(endpoint5);
    // Should be able to add this one, though; it's unrelated to any existing endpoints.
    XCTAssertTrue([controllerClient addServerEndpoint:endpoint5]);

    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:nodeIDServer controller:controllerClient];

    __auto_type * requestPath = attribute1RequestPath;
    __block __auto_type * responsePath = attribute1ResponsePath;

    __auto_type checkSingleValue = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error, NSDictionary<NSString *, id> * expectedValue) {
        // The overall interaction should succeed.
        XCTAssertNil(error);
        XCTAssertNotNil(values);

        // And we should get a value for our attribute.
        XCTAssertEqual(values.count, 1);

        NSDictionary<NSString *, id> * value = values[0];
        XCTAssertEqualObjects(value[MTRAttributePathKey], responsePath);

        XCTAssertNil(value[MTRErrorKey]);
        XCTAssertNotNil(value[MTRDataKey]);

        XCTAssertEqualObjects(value[MTRDataKey], expectedValue);
    };

    __auto_type checkSinglePathError = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error, MTRInteractionErrorCode expectedError) {
        // The overall interaction should succeed.
        XCTAssertNil(error);
        XCTAssertNotNil(values);

        // And we should get a value for our attribute.
        XCTAssertEqual(values.count, 1);

        NSDictionary<NSString *, id> * value = values[0];
        XCTAssertEqualObjects(value[MTRAttributePathKey], responsePath);

        XCTAssertNil(value[MTRDataKey]);
        XCTAssertNotNil(value[MTRErrorKey]);

        NSError * pathError = value[MTRErrorKey];
        XCTAssertEqual(pathError.domain, MTRInteractionErrorDomain);
        XCTAssertEqual(pathError.code, expectedError);
    };

    // First try a basic read.
    XCTestExpectation * readExpectation1 = [self expectationWithDescription:@"Read 1 of attribute complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSingleValue(values, error, unsignedIntValue1);
                            [readExpectation1 fulfill];
                        }];
    [self waitForExpectations:@[ readExpectation1 ] timeout:kTimeoutInSeconds];

    // Now try a basic subscribe.
    __block void (^reportHandler)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error);

    XCTestExpectation * initialValueExpectation = [self expectationWithDescription:@"Got initial value"];
    reportHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        checkSingleValue(values, error, unsignedIntValue1);
        [initialValueExpectation fulfill];
    };

    XCTestExpectation * subscriptionEstablishedExpectation = [self expectationWithDescription:@"Basic subscription established"];
    __auto_type * subscribeParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)];
    [baseDevice subscribeToAttributesWithEndpointID:requestPath.endpoint clusterID:requestPath.cluster attributeID:requestPath.attribute
        params:subscribeParams
        queue:queue
        reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
            reportHandler(values, error);
        }
        subscriptionEstablished:^() {
            [subscriptionEstablishedExpectation fulfill];
        }];
    [self waitForExpectations:@[ subscriptionEstablishedExpectation, initialValueExpectation ] timeout:kTimeoutInSeconds];

    // Now change the value and expect to see it on our subscription.
    XCTestExpectation * valueUpdateExpectation = [self expectationWithDescription:@"We see the new value"];
    reportHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        checkSingleValue(values, error, unsignedIntValue2);
        [valueUpdateExpectation fulfill];
    };

    [attribute1 setValue:unsignedIntValue2];

    [self waitForExpectations:@[ valueUpdateExpectation ] timeout:kTimeoutInSeconds];

    // Now try a read of an attribute we do not have permissions for.
    requestPath = attribute2RequestPath;
    responsePath = attribute2ResponsePath;
    XCTestExpectation * readNoPermissionsExpectation1 = [self expectationWithDescription:@"Read 1 of attribute with no permissions complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSinglePathError(values, error, MTRInteractionErrorCodeUnsupportedAccess);
                            [readNoPermissionsExpectation1 fulfill];
                        }];
    [self waitForExpectations:@[ readNoPermissionsExpectation1 ] timeout:kTimeoutInSeconds];

    // Change the permissions to give Manage access on the cluster to some
    // random node ID and try again.  Should still have no permissions.
    __auto_type * unrelatedGrant = [MTRAccessGrant accessGrantForNodeID:@(0xabc) privilege:MTRAccessControlEntryPrivilegeManage];
    XCTAssertNotNil(unrelatedGrant);
    [cluster2 addAccessGrant:unrelatedGrant];

    XCTestExpectation * readNoPermissionsExpectation2 = [self expectationWithDescription:@"Read 2 of attribute with no permissions complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSinglePathError(values, error, MTRInteractionErrorCodeUnsupportedAccess);
                            [readNoPermissionsExpectation2 fulfill];
                        }];
    [self waitForExpectations:@[ readNoPermissionsExpectation2 ] timeout:kTimeoutInSeconds];

    // Change the permissions to give Manage access on the cluster to our client
    // node ID and try again.  Should be able to read the attribute now.
    __auto_type * clientManageGrant = [MTRAccessGrant accessGrantForNodeID:nodeIDClient privilege:MTRAccessControlEntryPrivilegeManage];
    XCTAssertNotNil(clientManageGrant);
    [cluster2 addAccessGrant:clientManageGrant];

    XCTestExpectation * readExpectation2 = [self expectationWithDescription:@"Read 2 of attribute complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSingleValue(values, error, listOfStructsValue1);
                            [readExpectation2 fulfill];
                        }];
    [self waitForExpectations:@[ readExpectation2 ] timeout:kTimeoutInSeconds];

    // Adding Manage permissions to one cluster should not affect another one.
    requestPath = attribute3RequestPath;
    responsePath = attribute3ResponsePath;

    XCTestExpectation * readNoPermissionsExpectation3 = [self expectationWithDescription:@"Read 3 of attribute with no permissions complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSinglePathError(values, error, MTRInteractionErrorCodeUnsupportedAccess);
                            [readNoPermissionsExpectation3 fulfill];
                        }];
    [self waitForExpectations:@[ readNoPermissionsExpectation3 ] timeout:kTimeoutInSeconds];

    // But adding Manage permissions on the endpoint should grant Operate on
    // the cluster.
    [endpoint1 addAccessGrant:clientManageGrant];

    XCTestExpectation * readExpectation3 = [self expectationWithDescription:@"Read 3 of attribute complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSingleValue(values, error, unsignedIntValue1);
                            [readExpectation3 fulfill];
                        }];
    [self waitForExpectations:@[ readExpectation3 ] timeout:kTimeoutInSeconds];

    // And removing that grant should remove the permissions again.
    [endpoint1 removeAccessGrant:clientManageGrant];

    XCTestExpectation * readNoPermissionsExpectation4 = [self expectationWithDescription:@"Read 4 of attribute with no permissions complete"];
    [baseDevice readAttributePaths:@[ requestPath ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            checkSinglePathError(values, error, MTRInteractionErrorCodeUnsupportedAccess);
                            [readNoPermissionsExpectation4 fulfill];
                        }];
    [self waitForExpectations:@[ readNoPermissionsExpectation4 ] timeout:kTimeoutInSeconds];

    // Now do a wildcard read on the endpoint and check that this does the right
    // thing (gets the right things from descriptor, gets both clusters, etc).
    __auto_type globalAttributePath = ^(NSNumber * clusterID, MTRAttributeIDType attributeID) {
        return [MTRAttributePath attributePathWithEndpointID:endpointId1 clusterID:clusterID attributeID:@(attributeID)];
    };
    __auto_type descriptorAttributePath = ^(MTRAttributeIDType attributeID) {
        return [MTRAttributePath attributePathWithEndpointID:endpointId1 clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(attributeID)];
    };
    __auto_type unsignedIntValue = ^(NSUInteger value) {
        return @{
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(value),
        };
    };
    __auto_type arrayOfUnsignedIntegersValue = ^(NSArray<NSNumber *> * values) {
        __auto_type * mutableArray = [[NSMutableArray alloc] init];
        for (NSNumber * value in values) {
            [mutableArray addObject:@{
                MTRDataKey : @ {
                    MTRTypeKey : MTRUnsignedIntegerValueType,
                    MTRValueKey : value,
                },
            }];
        }
        return @{
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : [mutableArray copy],
        };
    };
    __auto_type endpoint1DeviceTypeValue = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[
            @{
                MTRDataKey : @ {
                    MTRTypeKey : MTRStructureValueType,
                    MTRValueKey : @[
                        @{
                            MTRContextTagKey : @(0),
                            MTRDataKey : @ {
                                MTRTypeKey : MTRUnsignedIntegerValueType,
                                MTRValueKey : deviceType1.deviceTypeID,
                            },
                        },
                        @{
                            MTRContextTagKey : @(1),
                            MTRDataKey : @ {
                                MTRTypeKey : MTRUnsignedIntegerValueType,
                                MTRValueKey : deviceType1.deviceTypeRevision,
                            },
                        },
                    ],
                }
            },
        ],
    };

    XCTestExpectation * wildcardReadExpectation = [self expectationWithDescription:@"Wildcard read of our endpoint"];
    [baseDevice readAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:endpointId1 clusterID:nil attributeID:nil] ]
                        eventPaths:nil
                            params:nil
                             queue:queue
                        completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                            XCTAssertNil(error);
                            XCTAssertNotNil(values);

                            for (NSDictionary<NSString *, id> * value in values) {
                                XCTAssertNotNil(value[MTRAttributePathKey]);
                                XCTAssertNil(value[MTRErrorKey]);
                                XCTAssertNotNil(value[MTRDataKey]);
                            }

                            NSSet<NSDictionary<NSString *, id> *> * receivedValues = [NSSet setWithArray:values];
                            NSSet<NSDictionary<NSString *, id> *> * expectedValues = [NSSet setWithArray:@[
                                // cluster1
                                @ {
                                    MTRAttributePathKey : attribute1ResponsePath,
                                    MTRDataKey : unsignedIntValue2,
                                },
                                // attribute3 requires Operate privileges to read, which we do not have
                                // for this cluster, so it will not be present here.
                                @ {
                                    MTRAttributePathKey : globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeFeatureMapID),
                                    MTRDataKey : unsignedIntValue(0),
                                },
                                @ {
                                    MTRAttributePathKey : globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeClusterRevisionID),
                                    MTRDataKey : unsignedIntValue(clusterRevision1.unsignedIntegerValue),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeAttributeListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[
                                        attributeId1, attributeId2, @(0xFFFC), @(0xFFFD), @(0xFFF8), @(0xFFF9), @(0xFFFB)
                                    ]),
                                },

                                // cluster2
                                @ {
                                    MTRAttributePathKey : attribute2ResponsePath,
                                    MTRDataKey : listOfStructsValue1,
                                },
                                @ {
                                    MTRAttributePathKey : globalAttributePath(clusterId2, MTRAttributeIDTypeGlobalAttributeFeatureMapID),
                                    MTRDataKey : unsignedIntValue(0),
                                },
                                @ {
                                    MTRAttributePathKey : globalAttributePath(clusterId2, MTRAttributeIDTypeGlobalAttributeClusterRevisionID),
                                    MTRDataKey : unsignedIntValue(clusterRevision2.unsignedIntegerValue),
                                },
                                @{MTRAttributePathKey : globalAttributePath(clusterId2, MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(clusterId2, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(clusterId2, MTRAttributeIDTypeGlobalAttributeAttributeListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[
                                        attributeId2, @(0xFFFC), @(0xFFFD), @0xFFF8, @(0xFFF9), @(0xFFFB)
                                    ]),
                                },

                                // descriptor
                                @ {
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID),
                                    MTRDataKey : endpoint1DeviceTypeValue,
                                },
                                @{
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeClusterDescriptorAttributeServerListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[ clusterId1, clusterId2, @(MTRClusterIDTypeDescriptorID) ]),
                                },
                                @{
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeClusterDescriptorAttributeClientListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeClusterDescriptorAttributePartsListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                // No TagList attribute on this descriptor.
                                @ {
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeGlobalAttributeFeatureMapID),
                                    MTRDataKey : unsignedIntValue(0),
                                },
                                @ {
                                    MTRAttributePathKey : descriptorAttributePath(MTRAttributeIDTypeGlobalAttributeClusterRevisionID),
                                    // Would be nice if we could get the Descriptor cluster revision
                                    // from somewhere intead of hardcoding it...
                                    MTRDataKey : unsignedIntValue(2),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(@(MTRClusterIDTypeDescriptorID), MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(@(MTRClusterIDTypeDescriptorID), MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[]),
                                },
                                @{
                                    MTRAttributePathKey : globalAttributePath(@(MTRClusterIDTypeDescriptorID), MTRAttributeIDTypeGlobalAttributeAttributeListID),
                                    MTRDataKey : arrayOfUnsignedIntegersValue(@[
                                        @(0), @(1), @(2), @(3), @(0xFFFC), @(0xFFFD), @(0xFFF8), @(0xFFF9), @(0xFFFB)
                                    ]),
                                },

                            ]];

                            XCTAssertEqualObjects(receivedValues, expectedValues);

                            [wildcardReadExpectation fulfill];
                        }];
    [self waitForExpectations:@[ wildcardReadExpectation ] timeout:kTimeoutInSeconds];

    // Do some MTRDevice testing against this convenient server we have that has
    // vendor-specific attributes.
    __auto_type * device = [MTRDevice deviceWithNodeID:nodeIDServer controller:controllerClient];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.forceAttributeReportsIfMatchingCache = YES;

    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"MTRDevice subscription established"];
    delegate.onReportEnd = ^() {
        [gotReportsExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ gotReportsExpectation ] timeout:kTimeoutInSeconds];

    delegate.onReportEnd = nil;

    // Test read-through behavior of non-standard (as in, not present in Matter XML) attributes.
    XCTestExpectation * nonStandardReadThroughExpectation = [self expectationWithDescription:@"Read-throughs of non-standard attributes complete"];

    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReports) {
        XCTAssertNotNil(attributeReports);

        for (NSDictionary<NSString *, id> * report in attributeReports) {
            XCTAssertNil(report[MTRErrorKey]);

            XCTAssertNotNil(report[MTRDataKey]);
            XCTAssertNotNil(report[MTRAttributePathKey]);

            // We only expect to get a report for the read that opted in to be
            // treated as "C"
            XCTAssertEqualObjects(report[MTRAttributePathKey], attribute2ResponsePath);

            // Strip out the DataVersion before comparing values, since our
            // local value does not have that.
            __auto_type * reportValue = [NSMutableDictionary dictionaryWithDictionary:report[MTRDataKey]];
            reportValue[MTRDataVersionKey] = nil;
            XCTAssertEqualObjects(reportValue, listOfStructsValue1);

            [nonStandardReadThroughExpectation fulfill];
        }
    };

    __auto_type * attrValue = [device readAttributeWithEndpointID:attribute1ResponsePath.endpoint
                                                        clusterID:attribute1ResponsePath.cluster
                                                      attributeID:attribute1ResponsePath.attribute
                                                           params:nil];
    XCTAssertNotNil(attrValue);
    XCTAssertEqualObjects(attrValue, unsignedIntValue2);

    __auto_type * params = [[MTRReadParams alloc] init];
    params.assumeUnknownAttributesReportable = NO;
    attrValue = [device readAttributeWithEndpointID:attribute2ResponsePath.endpoint
                                          clusterID:attribute2ResponsePath.cluster
                                        attributeID:attribute2ResponsePath.attribute
                                             params:params];
    XCTAssertNotNil(attrValue);
    XCTAssertEqualObjects(attrValue, listOfStructsValue1);

    [self waitForExpectations:@[ nonStandardReadThroughExpectation ] timeout:kTimeoutInSeconds];

    [controllerClient shutdown];
    [controllerServer shutdown];
}

- (void)testSetMRPParametersWithRunningController
{
    NSError * error;
    __auto_type * storageDelegate = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithRootKeys:[[MTRTestKeys alloc] init]
                                                         operationalKeys:[[MTRTestKeys alloc] init]
                                                                fabricID:@555
                                                                  nodeID:@888
                                                                 storage:storageDelegate
                                                                   error:&error];
    XCTAssertNotNil(controller);
    XCTAssertTrue(controller.running);
    MTRSetMessageReliabilityParameters(@2000, @2000, @2000, @2000);
    [controller shutdown];

    // Now reset back to the default state, so timings in other tests are not
    // affected.
    MTRSetMessageReliabilityParameters(nil, nil, nil, nil);
}

// TODO: This might also want to go in a separate test file, with some shared setup for commissioning devices per test
- (void)doTestSubscriptionPoolWithSize:(NSInteger)subscriptionPoolSize deviceOnboardingPayloads:(NSDictionary<NSNumber *, NSString *> *)deviceOnboardingPayloads
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    NSNumber * nodeID = @(555);
    NSNumber * fabricID = @(555);

    NSError * error;

    // Test DeviceController with a Subscription pool
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer
                                          concurrentSubscriptionPoolSize:subscriptionPoolSize];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    NSArray<NSNumber *> * orderedDeviceIDs = [deviceOnboardingPayloads allKeys];

    // Commission 5 devices
    for (NSNumber * deviceID in orderedDeviceIDs) {
        certificateIssuer.nextNodeID = deviceID;
        [self commissionWithController:controller newNodeID:deviceID onboardingPayload:deviceOnboardingPayloads[deviceID]];
    }

    // Shutdown and restart, to reset all existing sessions, so that the subscriptions and base device usage start after
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    controller = [self startControllerWithRootKeys:rootKeys
                                   operationalKeys:operationalKeys
                                          fabricID:fabricID
                                            nodeID:nodeID
                                           storage:storageDelegate
                                             error:&error
                                 certificateIssuer:&certificateIssuer
                    concurrentSubscriptionPoolSize:subscriptionPoolSize];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Set up expectations and delegates

    NSMutableDictionary<NSNumber *, XCTestExpectation *> * subscriptionExpectations = [NSMutableDictionary dictionary];
    for (NSNumber * deviceID in orderedDeviceIDs) {
        NSString * expectationDescription = [NSString stringWithFormat:@"Subscription 1 has been set up %@", deviceID];
        subscriptionExpectations[deviceID] = [self expectationWithDescription:expectationDescription];
    }

    NSMutableDictionary<NSNumber *, MTRDeviceTestDelegate *> * deviceDelegates = [NSMutableDictionary dictionary];
    for (NSNumber * deviceID in orderedDeviceIDs) {
        deviceDelegates[deviceID] = [[MTRDeviceTestDelegate alloc] init];
    }

    // Test with counters
    __block os_unfair_lock counterLock = OS_UNFAIR_LOCK_INIT;
    __block NSUInteger subscriptionRunningCount = 0;
    __block NSUInteger subscriptionDequeueCount = 0;
    __block BOOL baseDeviceReadCompleted = NO;

    for (NSNumber * deviceID in orderedDeviceIDs) {
        MTRDeviceTestDelegate * delegate = deviceDelegates[deviceID];
        delegate.pretendThreadEnabled = YES;

        delegate.onSubscriptionPoolDequeue = ^{
            // Count subscribing when dequeued from the subscription pool
            os_unfair_lock_lock(&counterLock);
            subscriptionRunningCount++;
            subscriptionDequeueCount++;
            // At any given moment, only up to subscriptionPoolSize subcriptions can be going on
            XCTAssertLessThanOrEqual(subscriptionRunningCount, subscriptionPoolSize);
            os_unfair_lock_unlock(&counterLock);
        };
        delegate.onSubscriptionPoolWorkComplete = ^{
            // Stop counting subscribing right before calling work item completion
            os_unfair_lock_lock(&counterLock);
            subscriptionRunningCount--;

            // Given the base device read is happening on the 5th device, at the completion
            // time of the first [pool size] subscriptions, the BaseDevice's request to
            // read can't have completed, as it should be gated on its call to
            // MTRDeviceController_Concrete's getSessionForNode:.
            if (subscriptionDequeueCount <= (orderedDeviceIDs.count - subscriptionPoolSize)) {
                XCTAssertFalse(baseDeviceReadCompleted);
            }
            os_unfair_lock_unlock(&counterLock);
        };
        __weak __auto_type weakDelegate = delegate;
        delegate.onReportEnd = ^{
            [subscriptionExpectations[deviceID] fulfill];
            // reset callback so expectation not fulfilled twice, given the run time of this can be long due to subscription pool
            __strong __auto_type strongDelegate = weakDelegate;
            strongDelegate.onReportEnd = nil;
        };
    }

    for (NSNumber * deviceID in orderedDeviceIDs) {
        __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
        [device setDelegate:deviceDelegates[deviceID] queue:queue];
    }

    // Create the base device to attempt to read from the 5th device
    __auto_type * baseDeviceReadExpectation = [self expectationWithDescription:@"BaseDevice read"];
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:@(105) controller:controller];
    __auto_type * onOffCluster = [[MTRBaseClusterOnOff alloc] initWithDevice:baseDevice endpointID:@(1) queue:queue];
    [onOffCluster readAttributeOnOffWithCompletion:^(NSNumber * value, NSError * _Nullable error) {
        XCTAssertNil(error);
        // We expect the device to be off.
        XCTAssertEqualObjects(value, @(0));
        [baseDeviceReadExpectation fulfill];
        os_unfair_lock_lock(&counterLock);
        baseDeviceReadCompleted = YES;
        os_unfair_lock_unlock(&counterLock);
    }];

    // Make the wait time depend on pool size and device count (can expand number of devices in the future)
    NSArray * expectationsToWait = [subscriptionExpectations.allValues arrayByAddingObject:baseDeviceReadExpectation];
    [self waitForExpectations:expectationsToWait timeout:(kSubscriptionPoolBaseTimeoutInSeconds * orderedDeviceIDs.count / subscriptionPoolSize)];

    XCTAssertEqual(subscriptionDequeueCount, orderedDeviceIDs.count);

    // Reset our commissionees.
    for (NSNumber * deviceID in orderedDeviceIDs) {
        __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
        ResetCommissioneeWithNodeID(baseDevice, queue, self, kTimeoutInSeconds, deviceID);
    }

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)testSubscriptionPool
{
    // QRCodes generated for discriminators 1111~1115 and passcodes 1001~1005
    NSDictionary<NSNumber *, NSString *> * deviceOnboardingPayloads = @{
        @(101) : @"MT:00000UZ427U0D900000",
        @(102) : @"MT:00000CQM00BED900000",
        @(103) : @"MT:00000K0V01TRD900000",
        @(104) : @"MT:00000SC11293E900000",
        @(105) : @"MT:00000-O913RGE900000",
    };

    // Start our helper apps.
    __auto_type * sortedKeys = [[deviceOnboardingPayloads allKeys] sortedArrayUsingSelector:@selector(compare:)];
    for (NSNumber * deviceID in sortedKeys) {
        BOOL started = [self startAppWithName:@"all-clusters"
                                    arguments:@[]
                                      payload:deviceOnboardingPayloads[deviceID]];
        XCTAssertTrue(started);
    }

    [self doTestSubscriptionPoolWithSize:1 deviceOnboardingPayloads:deviceOnboardingPayloads];
    [self doTestSubscriptionPoolWithSize:2 deviceOnboardingPayloads:deviceOnboardingPayloads];
}

- (void)testSubscriptionPoolManyDevices
{
    // QRCodes generated for discriminators 1111~1150 and passcodes 1001~1050
    NSDictionary<NSNumber *, NSString *> * deviceOnboardingPayloads = @{
        @(101) : @"MT:00000I9K17U0D900000",
        @(102) : @"MT:0000000000BED900000",
        @(103) : @"MT:000008C801TRD900000",
        @(104) : @"MT:00000GOG0293E900000",
        @(105) : @"MT:00000O-O03RGE900000",
        @(106) : @"MT:00000WAX047UE900000",
        @(107) : @"MT:000002N315P5F900000",
        @(108) : @"MT:00000AZB165JF900000",
        @(109) : @"MT:00000I9K17NWF900000",
        @(110) : @"MT:000000000048G900000",
        @(111) : @"MT:000008C801MLG900000",
        @(112) : @"MT:00000GOG022ZG900000",
        @(113) : @"MT:00000O-O03KAH900000",
        @(114) : @"MT:00000WAX040OH900000",
        @(115) : @"MT:000002N315I.H900000",
        @(116) : @"MT:00000AZB16-CI900000",
        @(117) : @"MT:00000I9K17GQI900000",
        @(118) : @"MT:0000000000Z1J900000",
        @(119) : @"MT:000008C801FFJ900000",
        @(120) : @"MT:00000GOG02XSJ900000",
        @(121) : @"MT:00000O-O03D4K900000",
        @(122) : @"MT:00000WAX04VHK900000",
        @(123) : @"MT:000002N315BVK900000",
        @(124) : @"MT:00000AZB16T6L900000",
        @(125) : @"MT:00000I9K179KL900000",
        @(126) : @"MT:0000000000SXL900000",
        @(127) : @"MT:000008C80189M900000",
        @(128) : @"MT:00000GOG02QMM900000",
        @(129) : @"MT:00000O-O036-M900000",
        @(130) : @"MT:00000WAX04OBN900000",
        @(131) : @"MT:000002N3154PN900000",
        @(132) : @"MT:00000AZB16M0O900000",
        @(133) : @"MT:00000I9K172EO900000",
        @(134) : @"MT:0000000000LRO900000",
        @(135) : @"MT:000008C80113P900000",
        @(136) : @"MT:00000GOG02JGP900000",
        @(137) : @"MT:00000O-O03.TP900000",
        @(138) : @"MT:00000WAX04H5Q900000",
        @(139) : @"MT:000002N315ZIQ900000",
        @(140) : @"MT:00000AZB16FWQ900000",
        @(141) : @"MT:00000I9K17X7R900000",
        @(142) : @"MT:0000000000ELR900000",
        @(143) : @"MT:000008C801WYR900000",
        @(144) : @"MT:00000GOG02CAS900000",
        @(145) : @"MT:00000O-O03UNS900000",
        @(146) : @"MT:00000WAX04A.S900000",
        @(147) : @"MT:000002N315SCT900000",
        @(148) : @"MT:00000AZB168QT900000",
        @(149) : @"MT:00000I9K17Q1U900000",
        @(150) : @"MT:00000000007FU900000",
    };

    // Start our helper apps.
    __auto_type * sortedKeys = [[deviceOnboardingPayloads allKeys] sortedArrayUsingSelector:@selector(compare:)];
    for (NSNumber * deviceID in sortedKeys) {
        BOOL started = [self startAppWithName:@"all-clusters"
                                    arguments:@[]
                                      payload:deviceOnboardingPayloads[deviceID]];
        XCTAssertTrue(started);
    }

    [self doTestSubscriptionPoolWithSize:3 deviceOnboardingPayloads:deviceOnboardingPayloads];
}

- (MTRDevice *)getMTRDevice:(NSNumber *)deviceID
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(123);
    NSNumber * fabricID = @(456);

    NSError * error;
    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];
    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    MTRDevice * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    return device;
}

- (NSMutableArray<NSNumber *> *)getEndpointArrayFromPartsList:(MTRDeviceDataValueDictionary)partsList forDevice:(MTRDevice *)device
{
    // Initialize the endpoint array with endpoint 0.
    NSMutableArray<NSNumber *> * endpoints = [NSMutableArray arrayWithObject:@0];

    [endpoints addObjectsFromArray:[device arrayOfNumbersFromAttributeValue:partsList]];
    return endpoints;
}

- (void)testDataStorageUpdatesWhenRemovingEndpoints
{
    NSNumber * deviceID = @(17);
    __auto_type * device = [self getMTRDevice:deviceID];
    __auto_type queue = dispatch_get_main_queue();
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    __auto_type * controller = device.deviceController;

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __block NSNumber * dataVersionForPartsList;
    __block NSNumber * rootEndpoint = @0;

    // This test will do the following -
    // 1. Get the data version and attribute value of the parts list for endpoint 0 to inject a fake report.
    //    The injected attribute report will delete endpoint 2.
    //    That should cause the endpoint and its corresponding clusters to be removed from data storage.
    // 2. The data store is populated with cluster index and cluster data for the set of endpoints in our test app initially.
    // 3. After the fake attribute report is injected with deleted endpoint 2, make sure the data store is still populated with cluster index and cluster data
    //    for all the other endpoints.
    __block MTRDeviceDataValueDictionary testDataForPartsList;
    __block id testClusterDataValueForPartsList;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);

        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);

            if ([attributePath.endpoint isEqualToNumber:rootEndpoint] && attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterDescriptorAttributePartsListID) {
                testDataForPartsList = attributeDict[MTRDataKey];
                XCTAssertNotNil(testDataForPartsList);
                dataVersionForPartsList = testDataForPartsList[MTRDataVersionKey];
                id dataValue = testDataForPartsList[MTRValueKey];
                XCTAssertNotNil(dataValue);
                testClusterDataValueForPartsList = [dataValue mutableCopy];
            }
        }
    };

    __block NSMutableDictionary<NSNumber *, NSArray<NSNumber *> *> * initialClusterIndex = [[NSMutableDictionary alloc] init];
    // Some of the places we get endpoint lists get them from enumerating dictionary keys, which
    // means order is not guaranteed.  Make sure we compare sets of endpoints, not arrays, to
    // account for that.
    __block NSSet<NSNumber *> * testEndpoints;

    delegate.onReportEnd = ^{
        XCTAssertNotNil(dataVersionForPartsList);
        XCTAssertNotNil(testClusterDataValueForPartsList);
        testEndpoints = [NSSet setWithArray:[self getEndpointArrayFromPartsList:testDataForPartsList forDevice:device]];

        // Make sure that the cluster data in the data storage is populated with cluster index and cluster data for our endpoints.
        // We do not need to check _persistedClusterData here. _persistedClusterData will be paged in from storage when needed so
        // just checking data storage should suffice here.
        dispatch_sync(self->_storageQueue, ^{
            XCTAssertEqualObjects([NSSet setWithArray:[controller.controllerDataStore _fetchEndpointIndexForNodeID:deviceID]], testEndpoints);

            // Populate the initialClusterIndex to use as a reference for all cluster paths later.
            for (NSNumber * endpoint in testEndpoints) {
                [initialClusterIndex setObject:[controller.controllerDataStore _fetchClusterIndexForNodeID:deviceID endpointID:endpoint] forKey:endpoint];
            }

            for (NSNumber * endpoint in testEndpoints) {
                for (NSNumber * cluster in [initialClusterIndex objectForKey:endpoint]) {
                    XCTAssertNotNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:endpoint clusterID:cluster]);
                }
            }
        });
        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];

    // Inject a fake attribute report deleting endpoint 2 from the parts list at the root endpoint.
    dataVersionForPartsList = [NSNumber numberWithUnsignedLongLong:(dataVersionForPartsList.unsignedLongLongValue + 1)];

    // Delete our to-be-deleted endpoint from the attribute value in parts list.  Make sure it's in the list to start with.
    NSNumber * toBeDeletedEndpoint = @2;
    XCTAssertTrue([testEndpoints containsObject:toBeDeletedEndpoint]);
    id endpointData =
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : toBeDeletedEndpoint,
            }
        };

    XCTAssertTrue([testClusterDataValueForPartsList containsObject:endpointData]);
    [testClusterDataValueForPartsList removeObject:endpointData];

    NSArray<NSDictionary<NSString *, id> *> * attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:rootEndpoint clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForPartsList,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : testClusterDataValueForPartsList,
        }
    } ];

    XCTestExpectation * attributeDataReceivedExpectation = [self expectationWithDescription:@"Injected Attribute data received"];
    XCTestExpectation * reportEndExpectation = [self expectationWithDescription:@"Injected Attribute data report ended"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);

        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);

            // Get the new updated parts list value to get the new test endpoints.
            if ([attributePath.endpoint isEqualToNumber:rootEndpoint] && attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterDescriptorAttributePartsListID) {
                testDataForPartsList = attributeDict[MTRDataKey];
                XCTAssertNotNil(testDataForPartsList);
                id dataValue = testDataForPartsList[MTRValueKey];
                XCTAssertNotNil(dataValue);
                testClusterDataValueForPartsList = [dataValue mutableCopy];
            }
        }
        [attributeDataReceivedExpectation fulfill];
    };

    delegate.onReportEnd = ^{
        XCTAssertNotNil(testClusterDataValueForPartsList);
        testEndpoints = [NSSet setWithArray:[self getEndpointArrayFromPartsList:testDataForPartsList forDevice:device]];

        // Make sure that the cluster data is removed from the data storage for the endpoint we deleted, but still there for the others.
        // We do not need to check _persistedClusterData here. _persistedClusterData will be paged in from storage when needed so
        // just checking data storage should suffice here.
        dispatch_sync(self->_storageQueue, ^{
            XCTAssertEqualObjects([NSSet setWithArray:[controller.controllerDataStore _fetchEndpointIndexForNodeID:deviceID]], testEndpoints);
            for (NSNumber * endpoint in testEndpoints) {
                XCTAssertNotNil(initialClusterIndex);
                for (NSNumber * cluster in [initialClusterIndex objectForKey:endpoint]) {
                    if ([endpoint isEqualToNumber:toBeDeletedEndpoint]) {
                        XCTAssertNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:endpoint clusterID:cluster]);
                    } else {
                        XCTAssertNotNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:endpoint clusterID:cluster]);
                    }
                }
            }
        });
        [reportEndExpectation fulfill];
    };

    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ attributeDataReceivedExpectation, reportEndExpectation ] timeout:60];

    [controller.controllerDataStore clearAllStoredClusterData];
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * storedClusterDataAfterClear = [controller.controllerDataStore getStoredClusterDataForNodeID:deviceID];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    [controller removeDevice:device];
    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)testDataStorageUpdatesWhenRemovingClusters
{
    NSNumber * deviceID = @(17);
    __auto_type * device = [self getMTRDevice:deviceID];
    __auto_type queue = dispatch_get_main_queue();
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    __auto_type * controller = device.deviceController;

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __block NSNumber * dataVersionForServerList;
    __block NSNumber * testEndpoint = @1;

    // This test will do the following -
    // 1. Get the data version and attribute value of the server list for endpoint 1 to inject a fake report. The attribute report will delete cluster ID - MTRClusterIDTypeIdentifyID.
    //    That should cause the cluster to be removed from cluster index for endpoint 1 and the cluster data for the removed cluster should be cleared from data storage.
    // 2. The data store is populated with MTRClusterIDTypeIdentifyID in the cluster index and cluster data for endpoint 1 initially.
    // 3. After the fake attribute report is injected with deleted cluster ID - MTRClusterIDTypeIdentifyID, make sure the data store is still populated with cluster index and
    //    cluster data for all other clusters at endpoint 1 but not the deleted cluster.
    __block id testClusterDataValue;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);

        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);

            if ([attributePath.endpoint isEqualToNumber:testEndpoint] && attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterDescriptorAttributeServerListID) {
                MTRDeviceDataValueDictionary data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForServerList = data[MTRDataVersionKey];
                id dataValue = data[MTRValueKey];
                XCTAssertNotNil(dataValue);
                testClusterDataValue = [dataValue mutableCopy];
            }
        }
    };

    __block NSMutableArray<NSNumber *> * initialClusterIndex = [[NSMutableArray alloc] init];
    __block NSNumber * toBeDeletedCluster = @(MTRClusterIDTypeIdentifyID);

    delegate.onReportEnd = ^{
        XCTAssertNotNil(dataVersionForServerList);
        XCTAssertNotNil(testClusterDataValue);

        // Make sure that the cluster data in the data storage has cluster ID - MTRClusterIDTypeIdentifyID in the cluster index for endpoint 1
        // and cluster data for MTRClusterIDTypeIdentifyID exists.
        // We do not need to check _persistedClusterData here. _persistedClusterData will be paged in from storage when needed so
        // just checking data storage should suffice here.
        dispatch_sync(self->_storageQueue, ^{
            initialClusterIndex = [[controller.controllerDataStore _fetchClusterIndexForNodeID:deviceID endpointID:testEndpoint] mutableCopy];
            XCTAssertTrue([initialClusterIndex containsObject:toBeDeletedCluster]);
            for (NSNumber * cluster in initialClusterIndex) {
                XCTAssertNotNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:testEndpoint clusterID:cluster]);
            }
        });
        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];

    // Inject a fake attribute report after removing cluster ID - MTRClusterIDTypeIdentifyID from endpoint 1 to the server list.
    dataVersionForServerList = [NSNumber numberWithUnsignedLongLong:(dataVersionForServerList.unsignedLongLongValue + 1)];
    id identifyClusterData =
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : toBeDeletedCluster,
            }
        };
    [testClusterDataValue removeObject:identifyClusterData];

    NSArray<NSDictionary<NSString *, id> *> * attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:testEndpoint clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeServerListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForServerList,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : testClusterDataValue,
        }
    } ];

    XCTestExpectation * attributeDataReceivedExpectation = [self expectationWithDescription:@"Injected Attribute data received"];
    XCTestExpectation * reportEndExpectation = [self expectationWithDescription:@"Injected Attribute data report ended"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);
        [attributeDataReceivedExpectation fulfill];
    };

    delegate.onReportEnd = ^{
        // Make sure that the cluster data does not have cluster ID - MTRClusterIDTypeIdentifyID in the cluster index for endpoint 1
        // and cluster data for MTRClusterIDTypeIdentifyID is nil.
        // We do not need to check _persistedClusterData here. _persistedClusterData will be paged in from storage when needed so
        // just checking data storage should suffice here.
        dispatch_sync(self->_storageQueue, ^{
            XCTAssertFalse([[controller.controllerDataStore _fetchClusterIndexForNodeID:deviceID endpointID:testEndpoint] containsObject:toBeDeletedCluster]);
            for (NSNumber * cluster in initialClusterIndex) {
                if ([cluster isEqualToNumber:toBeDeletedCluster]) {
                    XCTAssertNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:testEndpoint clusterID:cluster]);
                } else {
                    XCTAssertNotNil([controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:testEndpoint clusterID:cluster]);
                }
            }
        });
        [reportEndExpectation fulfill];
    };

    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ attributeDataReceivedExpectation, reportEndExpectation ] timeout:60];

    [controller.controllerDataStore clearAllStoredClusterData];
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * storedClusterDataAfterClear = [controller.controllerDataStore getStoredClusterDataForNodeID:deviceID];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    [controller removeDevice:device];
    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)testDataStorageUpdatesWhenRemovingAttributes
{
    NSNumber * deviceID = @(17);
    __auto_type * device = [self getMTRDevice:deviceID];
    __auto_type queue = dispatch_get_main_queue();
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    __auto_type * controller = device.deviceController;

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __block NSNumber * dataVersionForIdentify;
    __block NSNumber * testEndpoint = @(1);
    __block NSNumber * toBeDeletedAttribute = @(1);
    __block id testClusterDataValue;

    // This test will do the following -
    // 1. Get the data version and attribute value of the attribute list for endpoint 1 to inject a fake report with attribute 1 removed from MTRClusterIDTypeIdentifyID.
    // 2. The data store is populated with cluster data for MTRClusterIDTypeIdentifyID cluster and has all attributes including attribute 1.
    // 3. After the fake attribute report is injected, make sure the data store is populated with cluster data for all attributes in MTRClusterIDTypeIdentifyID
    //    cluster except for attribute 1 which has been deleted.
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);

        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);

            if ([attributePath.endpoint isEqualToNumber:testEndpoint] && attributePath.cluster.unsignedLongValue == MTRClusterIDTypeIdentifyID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeGlobalAttributeAttributeListID) {
                MTRDeviceDataValueDictionary data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForIdentify = data[MTRDataVersionKey];
                id dataValue = data[MTRValueKey];
                XCTAssertNotNil(dataValue);
                testClusterDataValue = [dataValue mutableCopy];
            }
        }
    };

    __block NSMutableArray<NSNumber *> * initialTestAttributes = [[NSMutableArray alloc] init];

    delegate.onReportEnd = ^{
        XCTAssertNotNil(dataVersionForIdentify);
        XCTAssertNotNil(testClusterDataValue);

        dispatch_sync(self->_storageQueue, ^{
            for (NSNumber * cluster in [controller.controllerDataStore _fetchClusterIndexForNodeID:deviceID endpointID:testEndpoint]) {

                // Make sure that the cluster data in the data storage is populated with cluster data for MTRClusterIDTypeIdentifyID cluster
                // and has all attributes including attribute 1.
                // We will page in the cluster data from storage to check the above.
                MTRClusterPath * path = [MTRClusterPath clusterPathWithEndpointID:testEndpoint clusterID:cluster];

                if ([cluster isEqualToNumber:@(MTRClusterIDTypeIdentifyID)]) {
                    MTRDeviceClusterData * data = [device unitTestGetClusterDataForPath:path];
                    XCTAssertNotNil(data);
                    XCTAssertNotNil(data.attributes);

                    MTRDeviceDataValueDictionary dict = [data.attributes objectForKey:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)];
                    XCTAssertNotNil(dict);

                    NSMutableArray<NSNumber *> * persistedAttributes = [device arrayOfNumbersFromAttributeValue:dict];
                    initialTestAttributes = [device arrayOfNumbersFromAttributeValue:@ { MTRTypeKey : MTRArrayValueType, MTRValueKey : testClusterDataValue }];
                    XCTAssertNotNil(persistedAttributes);
                    for (NSNumber * attribute in initialTestAttributes) {
                        XCTAssertTrue([persistedAttributes containsObject:attribute]);
                    }
                }
            }
        });

        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];

    dataVersionForIdentify = [NSNumber numberWithUnsignedLongLong:(dataVersionForIdentify.unsignedLongLongValue + 1)];
    id attributeData =
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : toBeDeletedAttribute,
            }
        };

    [testClusterDataValue removeObject:attributeData];

    NSArray<NSDictionary<NSString *, id> *> * attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:testEndpoint clusterID:@(MTRClusterIDTypeIdentifyID) attributeID:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForIdentify,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : testClusterDataValue,
        }
    } ];

    XCTestExpectation * attributeDataReceivedExpectation = [self expectationWithDescription:@"Injected Attribute data received"];
    XCTestExpectation * reportEndExpectation = [self expectationWithDescription:@"Injected Attribute data report ended"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        XCTAssertGreaterThan(attributeReport.count, 0);

        [attributeDataReceivedExpectation fulfill];
    };

    delegate.onReportEnd = ^{
        // Make sure that the cluster data in the data storage is populated with cluster data for MTRClusterIDTypeIdentifyID cluster
        // and has all attributes except attribute 1 which was deleted.
        // We will page in the cluster data from storage to check the above.
        dispatch_sync(self->_storageQueue, ^{
            for (NSNumber * cluster in [controller.controllerDataStore _fetchClusterIndexForNodeID:deviceID endpointID:testEndpoint]) {
                MTRDeviceClusterData * clusterData = [controller.controllerDataStore _fetchClusterDataForNodeID:deviceID endpointID:testEndpoint clusterID:cluster];
                XCTAssertNotNil(clusterData);
                MTRClusterPath * path = [MTRClusterPath clusterPathWithEndpointID:testEndpoint clusterID:cluster];

                if ([cluster isEqualToNumber:@(MTRClusterIDTypeIdentifyID)]) {
                    MTRDeviceClusterData * data = [device unitTestGetClusterDataForPath:path];
                    XCTAssertNotNil(data);
                    XCTAssertNotNil(data.attributes);

                    MTRDeviceDataValueDictionary attributeListValue = [data.attributes objectForKey:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)];
                    XCTAssertNotNil(attributeListValue);

                    NSMutableArray<NSNumber *> * persistedAttributes = [device arrayOfNumbersFromAttributeValue:attributeListValue];
                    XCTAssertNotNil(persistedAttributes);
                    for (NSNumber * attribute in initialTestAttributes) {
                        if ([attribute isEqualToNumber:toBeDeletedAttribute]) {
                            XCTAssertFalse([persistedAttributes containsObject:attribute]);
                        } else {
                            XCTAssertTrue([persistedAttributes containsObject:attribute]);
                        }
                    }
                }
            }
        });

        [reportEndExpectation fulfill];
    };

    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ attributeDataReceivedExpectation, reportEndExpectation ] timeout:60];

    [controller.controllerDataStore clearAllStoredClusterData];
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * storedClusterDataAfterClear = [controller.controllerDataStore getStoredClusterDataForNodeID:deviceID];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    [controller removeDevice:device];
    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

// Run the test here since detectLeaks is set, and subscription reset needs to not cause leaks
- (void)testMTRDeviceResetSubscription
{
    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(333);
    NSNumber * fabricID = @(444);

    NSError * error;

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(22);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * subscriptionExpectation1 = [self expectationWithDescription:@"Subscription has been set up 1"];

    delegate.onReportEnd = ^{
        [subscriptionExpectation1 fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation1 ] timeout:60];

    // Test 1: test that subscription reset works

    XCTestExpectation * subscriptionExpectation2 = [self expectationWithDescription:@"Subscription has been set up 2"];

    __weak __auto_type weakDelegate = delegate;
    delegate.onReportEnd = ^{
        [subscriptionExpectation2 fulfill];
        // reset callback so expectation not fulfilled twice
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };

    // clear cluster data before reset
    NSUInteger attributeCountBeforeReset = [device unitTestAttributeCount];
    [device unitTestClearClusterData];

    [device unitTestResetSubscription];

    [self waitForExpectations:@[ subscriptionExpectation2 ] timeout:60];

    // check that in-memory cache has recovered
    NSUInteger attributeCountAfterReset = [device unitTestAttributeCount];
    XCTAssertEqual(attributeCountBeforeReset, attributeCountAfterReset);

    // Test 2: simulate a cache purge and loss of storage, to see:
    //  * that subscription reestablishes
    //  * the cache is restored
    [device unitTestClearClusterData];
    [controller.controllerDataStore clearAllStoredClusterData];

    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * storedClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:deviceID];
    XCTAssertEqual(storedClusterData.count, 0);

    XCTestExpectation * subscriptionExpectation3 = [self expectationWithDescription:@"Subscription has been set up 3"];
    delegate.onReportEnd = ^{
        [subscriptionExpectation3 fulfill];
        // reset callback so expectation not fulfilled twice
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };

    // now get list of clusters, and call clusterDataForPath: to trigger the reset
    NSSet<MTRClusterPath *> * persistedClusters = [device unitTestGetPersistedClusters];
    MTRDeviceClusterData * data = [device unitTestGetClusterDataForPath:persistedClusters.anyObject];
    XCTAssertNil(data);

    // Also call clusterDataForPath: repeatedly to verify in logs that subscription is reset only once
    for (MTRClusterPath * path in persistedClusters) {
        MTRDeviceClusterData * data = [device unitTestGetClusterDataForPath:path];
        (void) data; // do not assert nil because subscription may happen during this time and already fill in the cache
    }

    [self waitForExpectations:@[ subscriptionExpectation3 ] timeout:60];

    // Verify that after report ends all the cluster data is back
    for (MTRClusterPath * path in persistedClusters) {
        MTRDeviceClusterData * data = [device unitTestGetClusterDataForPath:path];
        XCTAssertNotNil(data);
    }

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)testMTRDeviceDealloc
{
    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_queue_create("test.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(333);
    NSNumber * fabricID = @(444);

    NSError * error;

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(22);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    // Before the test, clear the device controller in-memory MapTable
    [controller forgetDeviceWithNodeID:deviceID];

    __block BOOL subscriptionReportEnd1 = NO;
    XCTestExpectation * subscriptionCallbackDeleted = [self expectationWithDescription:@"Subscription callback deleted"];
    XCTestExpectation * controllerAddedDevice = [self expectationWithDescription:@"Controller added device"];
    XCTestExpectation * controllerRemovedDevice = [self expectationWithDescription:@"Controller removed device"];
    @autoreleasepool {
        // Expected the test device was added and removed
        MTRPerControllerStorageTestsDeallocDelegate * controllerDelegate = [[MTRPerControllerStorageTestsDeallocDelegate alloc] init];
        __block NSUInteger lastDeviceCount = controller.devices.count;
        controllerDelegate.onDevicesChanged = ^{
            // Use self as lock for lastDeviceCount access, so sanitizer doesn't complain
            @synchronized(self) {
                NSArray<MTRDevice *> * devices = controller.devices;
                if (devices.count > lastDeviceCount) {
                    [controllerAddedDevice fulfill];
                } else if (devices.count < lastDeviceCount) {
                    [controllerRemovedDevice fulfill];
                }
                lastDeviceCount = devices.count;
            }
        };
        [controller addDeviceControllerDelegate:controllerDelegate queue:queue];

        __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
        __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

        XCTestExpectation * subscriptionReportBegin = [self expectationWithDescription:@"Subscription report begin"];

        delegate.onReportBegin = ^{
            [subscriptionReportBegin fulfill];
        };

        delegate.onReportEnd = ^{
            subscriptionReportEnd1 = YES;
        };

        delegate.onSubscriptionCallbackDelete = ^{
            [subscriptionCallbackDeleted fulfill];
        };

        [device setDelegate:delegate queue:queue];

        [self waitForExpectations:@[ subscriptionReportBegin ] timeout:60];

        XCTAssertEqual(controller.devices.count, 1);
    }

    // report should still be ongoing
    XCTAssertFalse(subscriptionReportEnd1);
    XCTAssertEqual(controller.devices.count, 0);

    // dealloc -> delete should be called soon after the autoreleasepool reaps
    [self waitForExpectations:@[ subscriptionCallbackDeleted, controllerAddedDevice, controllerRemovedDevice ] timeout:60];

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)testMTRDeviceMaybeUnreachableIgnoredIfReceivingFromDevice
{
    __auto_type * storageDelegate = [[MTRTestPerControllerStorageWithBulkReadWrite alloc] initWithControllerID:[NSUUID UUID]];

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_queue_create("test.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    NSNumber * nodeID = @(333);
    NSNumber * fabricID = @(444);

    NSError * error;

    MTRPerControllerStorageTestsCertificateIssuer * certificateIssuer;
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys
                                                         operationalKeys:operationalKeys
                                                                fabricID:fabricID
                                                                  nodeID:nodeID
                                                                 storage:storageDelegate
                                                                   error:&error
                                                       certificateIssuer:&certificateIssuer];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    // Now commission the device, to test that that works.
    NSNumber * deviceID = @(22);
    certificateIssuer.nextNodeID = deviceID;
    [self commissionWithController:controller newNodeID:deviceID];

    // We should have established CASE using our operational key.
    XCTAssertEqual(operationalKeys.signatureCount, 1);

    __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * subscriptionReportBegin = [self expectationWithDescription:@"Subscription report begin"];
    XCTestExpectation * subscriptionReportEnd = [self expectationWithDescription:@"Subscription report end"];

    __weak __auto_type weakDelegate = delegate;
    delegate.onReportBegin = ^{
        [subscriptionReportBegin fulfill];
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportBegin = nil;
    };

    delegate.onReportEnd = ^{
        [subscriptionReportEnd fulfill];
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };

    delegate.onSubscriptionReset = ^{
        XCTFail("Subscription should not be reset from calling _deviceMayBeReachable");
    };

    [device setDelegate:delegate queue:queue];

    // First wait for report to begin coming in
    [self waitForExpectations:@[ subscriptionReportBegin ] timeout:60];

    // Call _deviceMayBeReachable and expect it to be ignored
    [device _deviceMayBeReachable];

    [self waitForExpectations:@[ subscriptionReportEnd ] timeout:60];

    [device _deviceMayBeReachable];

    // Since subscription reset runs on the matter queue, synchronously run a block on the matter queue here to prove the reset did not happen
    [controller syncRunOnWorkQueue:^{
        ;
    } error:nil];

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

@end
