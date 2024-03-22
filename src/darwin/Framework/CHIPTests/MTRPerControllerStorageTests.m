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

// system dependencies
#import <XCTest/XCTest.h>

#import "MTRDeviceTestDelegate.h"
#import "MTRDevice_Internal.h"
#import "MTRErrorTestUtils.h"
#import "MTRFabricInfoChecker.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"
#import "MTRTestResetCommissioneeHelper.h"

#if MTR_PER_CONTROLLER_STORAGE_ENABLED

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kTestVendorId = 0xFFF1u;

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

@interface MTRPerControllerStorageTests : XCTestCase
@end

@implementation MTRPerControllerStorageTests {
    dispatch_queue_t _storageQueue;
}

- (void)setUp
{
    // Per-test setup, runs before each test.
    [super setUp];
    [self setContinueAfterFailure:NO];

    _storageQueue = dispatch_queue_create("test.storage.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
}

- (void)tearDown
{
    // Per-test teardown, runs after each test.
    [self stopFactory];
    _storageQueue = nil;
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
    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    __auto_type * deviceControllerDelegate = [[MTRPerControllerStorageTestsControllerDelegate alloc] initWithExpectation:expectation
                                                                                                               newNodeID:newNodeID];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
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
                                        caseAuthenticatedTags:(nullable NSSet *)caseAuthenticatedTags
                                                        error:(NSError * __autoreleasing *)error
                                            certificateIssuer:
                                                (MTRPerControllerStorageTestsCertificateIssuer * __autoreleasing *)certificateIssuer
{
    XCTAssertTrue(error != NULL);

    // Specify a fixed issuerID, so we get the same cert if we use the same keys.
    __auto_type * root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:error];
    XCTAssertNil(*error);
    XCTAssertNotNil(root);

    __auto_type * operational = [MTRCertificates createOperationalCertificate:rootKeys
                                                           signingCertificate:root
                                                         operationalPublicKey:operationalKeys.publicKey
                                                                     fabricID:fabricID
                                                                       nodeID:nodeID
                                                        caseAuthenticatedTags:caseAuthenticatedTags
                                                                        error:error];
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
    // TODO: This is only used by testControllerServer.  If that moves
    // elsewhere, take this back out again.
    params.shouldAdvertiseOperational = YES;

    __auto_type * ourCertificateIssuer = [[MTRPerControllerStorageTestsCertificateIssuer alloc] initWithRootCertificate:root
                                                                                                intermediateCertificate:nil
                                                                                                             signingKey:rootKeys
                                                                                                               fabricID:fabricID];
    XCTAssertNotNil(ourCertificateIssuer);

    if (certificateIssuer) {
        *certificateIssuer = ourCertificateIssuer;
    }

    [params setOperationalCertificateIssuer:ourCertificateIssuer queue:dispatch_get_main_queue()];

    return [[MTRDeviceController alloc] initWithParameters:params error:error];
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
                           certificateIssuer:certificateIssuer];
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
                           certificateIssuer:nil];
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

- (void)test008_TestDataStoreDirect
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

    XCTAssertEqualObjects(controller.controllerNodeID, nodeID);

    NSArray * testAttributes = @[
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(1)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(111) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(2)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(112) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(3)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(113) } },

        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(1)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(121) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(2)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(122) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(3)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(123) } },

        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(1) attributeID:@(1)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(211) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(1) attributeID:@(2)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(212) } },
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(1) attributeID:@(3)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(213) } },
    ];
    [controller.controllerDataStore storeAttributeValues:testAttributes forNodeID:@(1001)];
    [controller.controllerDataStore storeAttributeValues:testAttributes forNodeID:@(1002)];
    [controller.controllerDataStore storeAttributeValues:testAttributes forNodeID:@(1003)];

    MTRDeviceClusterData * testClusterData1 = [[MTRDeviceClusterData alloc] init];
    testClusterData1.dataVersion = @(1);
    MTRDeviceClusterData * testClusterData2 = [[MTRDeviceClusterData alloc] init];
    testClusterData2.dataVersion = @(2);
    MTRDeviceClusterData * testClusterData3 = [[MTRDeviceClusterData alloc] init];
    testClusterData3.dataVersion = @(3);
    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * testClusterData = @{
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(1)] : testClusterData1,
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)] : testClusterData2,
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(3)] : testClusterData3,
    };
    [controller.controllerDataStore storeClusterData:testClusterData forNodeID:@(1001)];

    // Check values are written and can be fetched
    NSArray * dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1001)];
    XCTAssertEqual(dataStoreValues.count, 9);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1002)];
    XCTAssertEqual(dataStoreValues.count, 9);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1003)];
    XCTAssertEqual(dataStoreValues.count, 9);

    // Check values
    for (NSDictionary * responseValue in dataStoreValues) {
        MTRAttributePath * path = responseValue[MTRAttributePathKey];
        XCTAssertNotNil(path);
        NSDictionary * dataValue = responseValue[MTRDataKey];
        XCTAssertNotNil(dataValue);
        NSString * type = dataValue[MTRTypeKey];
        XCTAssertNotNil(type);
        XCTAssertEqualObjects(type, MTRUnsignedIntegerValueType);
        NSNumber * value = dataValue[MTRValueKey];
        XCTAssertNotNil(value);

        if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(1)]) {
            XCTAssertEqualObjects(value, @(111));
        } else if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(2)]) {
            XCTAssertEqualObjects(value, @(112));
        } else if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(3)]) {
            XCTAssertEqualObjects(value, @(113));
        } else if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(2)] && [path.attribute isEqualToNumber:@(1)]) {
            XCTAssertEqualObjects(value, @(121));
        } else if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(2)] && [path.attribute isEqualToNumber:@(2)]) {
            XCTAssertEqualObjects(value, @(122));
        } else if ([path.endpoint isEqualToNumber:@(1)] && [path.cluster isEqualToNumber:@(2)] && [path.attribute isEqualToNumber:@(3)]) {
            XCTAssertEqualObjects(value, @(123));
        } else if ([path.endpoint isEqualToNumber:@(2)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(1)]) {
            XCTAssertEqualObjects(value, @(211));
        } else if ([path.endpoint isEqualToNumber:@(2)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(2)]) {
            XCTAssertEqualObjects(value, @(212));
        } else if ([path.endpoint isEqualToNumber:@(2)] && [path.cluster isEqualToNumber:@(1)] && [path.attribute isEqualToNumber:@(3)]) {
            XCTAssertEqualObjects(value, @(213));
        }
    }

    NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> * dataStoreClusterData = [controller.controllerDataStore getStoredClusterDataForNodeID:@(1001)];
    for (MTRClusterPath * path in testClusterData) {
        XCTAssertEqualObjects(testClusterData[path].dataVersion, dataStoreClusterData[path].dataVersion);
    }

    [controller.controllerDataStore clearStoredAttributesForNodeID:@(1001)];
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1001)];
    XCTAssertEqual(dataStoreValues.count, 0);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1002)];
    XCTAssertEqual(dataStoreValues.count, 9);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1003)];
    XCTAssertEqual(dataStoreValues.count, 9);

    [controller.controllerDataStore clearAllStoredAttributes];
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1001)];
    XCTAssertEqual(dataStoreValues.count, 0);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1002)];
    XCTAssertEqual(dataStoreValues.count, 0);
    dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:@(1003)];
    XCTAssertEqual(dataStoreValues.count, 0);

    // Test MTRDeviceControllerDataStore _pruneEmptyStoredAttributesBranches
    //  - Clear cache
    //  - Store an attribute
    //  - Manually delete it from the test storage delegate
    //  - Call _pruneEmptyStoredAttributesBranches
    [controller.controllerDataStore clearAllStoredAttributes];

    NSArray * testAttribute = @[
        @{ MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(1)], MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(111) } },
    ];
    [controller.controllerDataStore storeAttributeValues:testAttribute forNodeID:@(2001)];

    // store is async, so remove on the same queue to ensure order
    dispatch_sync(_storageQueue, ^{
        NSString * testAttributeValueKey = [controller.controllerDataStore _attributeValueKeyForNodeID:@(2001) endpointID:@(1) clusterID:@(1) attributeID:@(1)];
        [storageDelegate controller:controller removeValueForKey:testAttributeValueKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    });
    [controller.controllerDataStore unitTestPruneEmptyStoredAttributesBranches];

    // Now check the indexes are pruned
    NSString * testAttributeIndexKey = [controller.controllerDataStore _attributeIndexKeyForNodeID:@(2001) endpointID:@(1) clusterID:@(1)];
    id testAttributeIndex = [storageDelegate controller:controller valueForKey:testAttributeIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testAttributeIndex);
    NSString * testClusterIndexKey = [controller.controllerDataStore _clusterIndexKeyForNodeID:@(2001) endpointID:@(1)];
    id testClusterIndex = [storageDelegate controller:controller valueForKey:testClusterIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testClusterIndex);
    NSString * testEndpointIndexKey = [controller.controllerDataStore _endpointIndexKeyForNodeID:@(2001)];
    id testEndpointIndex = [storageDelegate controller:controller valueForKey:testEndpointIndexKey securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testEndpointIndex);
    id testNodeIndex = [storageDelegate controller:controller valueForKey:@"attrCacheNodeIndex" securityLevel:MTRStorageSecurityLevelSecure sharingType:MTRStorageSharingTypeNotShared];
    XCTAssertNil(testNodeIndex);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
}

- (void)test009_TestDataStoreMTRDevice
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type queue = dispatch_get_main_queue();

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

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

    __auto_type * device = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    delegate.onReportEnd = ^{
        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];

    NSArray * dataStoreValues = [controller.controllerDataStore getStoredAttributesForNodeID:deviceID];

    // Verify all values are stored into storage
    for (NSDictionary * responseValue in dataStoreValues) {
        MTRAttributePath * path = responseValue[MTRAttributePathKey];
        XCTAssertNotNil(path);
        NSDictionary * dataValue = responseValue[MTRDataKey];
        XCTAssertNotNil(dataValue);

        NSDictionary * dataValueFromMTRDevice = [device readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
        XCTAssertTrue([device _attributeDataValue:dataValue isEqualToDataValue:dataValueFromMTRDevice]);
    }

    // Now force the removal of the object from controller to test reloading read cache from storage
    [controller removeDevice:device];

    // Verify the new device is initialized with the same values
    __auto_type * newDevice = [MTRDevice deviceWithNodeID:deviceID controller:controller];
    NSUInteger storedAttributeDifferFromMTRDeviceCount = 0;
    for (NSDictionary * responseValue in dataStoreValues) {
        MTRAttributePath * path = responseValue[MTRAttributePathKey];
        XCTAssertNotNil(path);
        NSDictionary * dataValue = responseValue[MTRDataKey];
        XCTAssertNotNil(dataValue);

        NSDictionary * dataValueFromMTRDevice = [newDevice readAttributeWithEndpointID:path.endpoint clusterID:path.cluster attributeID:path.attribute params:nil];
        if (![newDevice _attributeDataValue:dataValue isEqualToDataValue:dataValueFromMTRDevice]) {
            storedAttributeDifferFromMTRDeviceCount++;
        }
    }

    // Only test that 90% of attributes are the same because there are some changing attributes each time (UTC time, for example)
    //   * With all-clusters-app as of 2024-02-10, about 1.476% of attributes change.
    double storedAttributeDifferFromMTRDevicePercentage = storedAttributeDifferFromMTRDeviceCount * 100.0 / dataStoreValues.count;
    XCTAssertTrue(storedAttributeDifferFromMTRDevicePercentage < 10.0);

    // Now
    __auto_type * newDelegate = [[MTRDeviceTestDelegate alloc] init];

    XCTestExpectation * newDeviceSubscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up for new device"];

    newDelegate.onReportEnd = ^{
        [newDeviceSubscriptionExpectation fulfill];
    };

    [newDevice setDelegate:newDelegate queue:queue];

    [self waitForExpectations:@[ newDeviceSubscriptionExpectation ] timeout:60];
    newDelegate.onReportEnd = nil;

    // 1) MTRDevice actually gets some attributes reported more than once
    // 2) Some attributes do change on resubscribe
    //   * With all-clusts-app as of 2024-02-10, out of 1287 persisted attributes, still 450 attributes were reported with filter
    // And so conservatively, assert that data version filters save at least 300 entries.
    NSUInteger storedAttributeCountDifferenceFromMTRDeviceReport = dataStoreValues.count - [device unitTestAttributesReportedSinceLastCheck];
    XCTAssertTrue(storedAttributeCountDifferenceFromMTRDeviceReport > 300);

    // Reset our commissionee.
    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:deviceID controller:controller];
    ResetCommissionee(baseDevice, queue, self, kTimeoutInSeconds);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
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

#if 0
    __auto_type * listOfStructsValue2 = @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : @[
                        @{ MTRDataKey: structValue2, },
                        ],
    };
#endif

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

    __auto_type * device = [MTRBaseDevice deviceWithNodeID:nodeIDServer controller:controllerClient];

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
    [device readAttributePaths:@[ requestPath ]
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
    [device subscribeToAttributesWithEndpointID:requestPath.endpoint clusterID:requestPath.cluster attributeID:requestPath.attribute
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
    [device readAttributePaths:@[ requestPath ]
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
    [device readAttributePaths:@[ requestPath ]
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
    [device readAttributePaths:@[ requestPath ]
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
    [device readAttributePaths:@[ requestPath ]
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
    [device readAttributePaths:@[ requestPath ]
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
    [device readAttributePaths:@[ requestPath ]
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
#if 0
    // Unused bits ifdefed out until we doing more testing on the actual values
    // we get back.
    __auto_type globalAttributePath = ^(NSNumber * clusterID, MTRAttributeIDType attributeID) {
        return [MTRAttributePath attributePathWithEndpointID:endpointId1 clusterID:clusterID attributeID:@(attributeID)];
    };
    __auto_type unsignedIntValue = ^(NSUInteger value) {
        return @{
        MTRTypeKey: MTRUnsignedIntegerValueType,
        MTRValueKey: @(value),
        };
    };
    __auto_type arrayOfUnsignedIntegersValue = ^(NSArray<NSNumber *> * values) {
        __auto_type * mutableArray = [[NSMutableArray alloc] init];
        for (NSNumber * value in values) {
            [mutableArray addObject:@{ MTRDataKey: @{
                    MTRTypeKey: MTRUnsignedIntegerValueType,
                            MTRValueKey: value,
                            }, }];
        }
        return @{
        MTRTypeKey: MTRArrayValueType,
                MTRValueKey: [mutableArray copy],
                };
    };
#endif
    XCTestExpectation * wildcardReadExpectation = [self expectationWithDescription:@"Wildcard read of our endpoint"];
    [device readAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:endpointId1 clusterID:nil attributeID:nil] ]
                    eventPaths:nil
                        params:nil
                         queue:queue
                    completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                        XCTAssertNil(error);
                        XCTAssertNotNil(values);

                        // TODO: Figure out how to test that values is correct that's not
                        // too fragile if things get returned in different valid order.
                        // For now just check that every path we got has a value, not an
                        // error.
                        for (NSDictionary<NSString *, id> * value in values) {
                            XCTAssertNotNil(value[MTRAttributePathKey]);
                            XCTAssertNil(value[MTRErrorKey]);
                            XCTAssertNotNil(value[MTRDataKey]);
                        }
#if 0
            XCTAssertEqualObjects(values, @[
                                            // cluster1
                                            @{ MTRAttributePathKey: attribute1ResponsePath,
                                                    MTRDataKey: unsignedIntValue2, },
                                               @{ MTRAttributePathKey: globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeFeatureMapID),
                                                    MTRDataKey: unsignedIntValue(0), },
                                               @{ MTRAttributePathKey: globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeClusterRevisionID),
                                                    MTRDataKey: clusterRevision1, },
                                               @{ MTRAttributePathKey: globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID),
                                                    MTRDataKey: arrayOfUnsignedIntegersValue(@[]), },
                                               @{ MTRAttributePathKey: globalAttributePath(clusterId1, MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID),
                                                    MTRDataKey: arrayOfUnsignedIntegersValue(@[]), },
                                             // etc

                                            ]);
#endif
                        [wildcardReadExpectation fulfill];
                    }];
    [self waitForExpectations:@[ wildcardReadExpectation ] timeout:kTimeoutInSeconds];

    [controllerClient shutdown];
    [controllerServer shutdown];
}

@end

#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED
