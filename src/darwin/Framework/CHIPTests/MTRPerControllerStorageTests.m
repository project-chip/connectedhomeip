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

#import "MTRErrorTestUtils.h"
#import "MTRFabricInfoChecker.h"
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

+ (void)tearDown
{
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
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:err], MTRInteractionErrorCodeUnsupportedAccess);
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

@end

#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED
