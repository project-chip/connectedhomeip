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
#import <dns_sd.h>

#import "MTRFabricInfoChecker.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTestVendorId = 0xFFF1u;
static const uint16_t kTimeoutInSeconds = 3;

static NSString * NodeIDAsString(NSNumber * nodeID) { return [NSString stringWithFormat:@"%016llX", nodeID.unsignedLongLongValue]; }

@interface MTRControllerAdvertisingTestsOperationalBrowser : NSObject
@property NSSet<NSString *> * discoveredNodes;

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation nodeIDToExpect:(NSNumber *)nodeIDToExpect;
- (void)discoveredNodeID:(NSString *)nodeID onCompressedFabricID:(NSString *)compressedFabricID;
@end

static void OnBrowse(DNSServiceRef aServiceRef, DNSServiceFlags aFlags, uint32_t aInterfaceId, DNSServiceErrorType aError,
    const char * aName, const char * aType, const char * aDomain, void * aContext)
{
    XCTAssertTrue(aError == kDNSServiceErr_NoError);

    if (!(aFlags & kDNSServiceFlagsAdd)) {
        return;
    }

    // 16 chars for compressed fabric id, 16 chars for node id, and the dash.
    XCTAssertTrue(strlen(aName) == 33);

    NSString * compressedFabricID = [[NSString alloc] initWithBytes:aName length:16 encoding:NSUTF8StringEncoding];
    NSString * nodeID = [[NSString alloc] initWithBytes:aName + 17 length:16 encoding:NSUTF8StringEncoding];

    __auto_type * self = (__bridge MTRControllerAdvertisingTestsOperationalBrowser *) aContext;
    [self discoveredNodeID:nodeID onCompressedFabricID:compressedFabricID];
}

static const char kLocalDot[] = "local.";
static const char kOperationalType[] = "_matter._tcp";
static const DNSServiceFlags kBrowseFlags = 0;

@implementation MTRControllerAdvertisingTestsOperationalBrowser {
    DNSServiceRef _browseRef;
    // Key is compressed fabric id, value is the set of discovered node IDs.
    NSMutableDictionary<NSString *, NSMutableSet<NSString *> *> * _allDiscoveredNodes;

    XCTestExpectation * _expectation;
    NSString * _nodeIDToExpect;
}

- (instancetype)initWithExpectation:(XCTestExpectation *)expectation nodeIDToExpect:(NSNumber *)nodeIDToExpect
{
    XCTAssertNotNil([super init]);

    _allDiscoveredNodes = [[NSMutableDictionary alloc] init];
    _expectation = expectation;
    _nodeIDToExpect = NodeIDAsString(nodeIDToExpect);

    __auto_type err = DNSServiceBrowse(
        &_browseRef, kBrowseFlags, kDNSServiceInterfaceIndexAny, kOperationalType, kLocalDot, OnBrowse, (__bridge void *) self);
    XCTAssertTrue(err == kDNSServiceErr_NoError);

    err = DNSServiceSetDispatchQueue(_browseRef, dispatch_get_main_queue());
    XCTAssertTrue(err == kDNSServiceErr_NoError);

    return self;
}

- (void)discoveredNodeID:(NSString *)nodeID onCompressedFabricID:(NSString *)compressedFabricID
{
    if (_allDiscoveredNodes[compressedFabricID] == nil) {
        _allDiscoveredNodes[compressedFabricID] = [[NSMutableSet alloc] init];
    }
    [_allDiscoveredNodes[compressedFabricID] addObject:nodeID];

    // It would be nice to check the compressedFabricID, but computing the right
    // expected value for it is a pain.
    if ([nodeID isEqualToString:_nodeIDToExpect]) {
        _discoveredNodes = [NSSet setWithSet:_allDiscoveredNodes[compressedFabricID]];
        // Stop our browse so we get no more notifications.
        DNSServiceRefDeallocate(_browseRef);
        _browseRef = NULL;
        [_expectation fulfill];
    }
}

- (void)dealloc
{
    if (_browseRef) {
        DNSServiceRefDeallocate(_browseRef);
    }
}

@end

@interface MTRControllerAdvertisingTests : XCTestCase
@end

@implementation MTRControllerAdvertisingTests {
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

- (nullable MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys
                                              operationalKeys:(MTRTestKeys *)operationalKeys
                                                     fabricID:(NSNumber *)fabricID
                                                       nodeID:(NSNumber *)nodeID
                                                      storage:(MTRTestPerControllerStorage *)storage
                                         advertiseOperational:(BOOL)advertiseOperational
                                                        error:(NSError * __autoreleasing *)error
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
                                                        caseAuthenticatedTags:nil
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

    params.shouldAdvertiseOperational = advertiseOperational;

    return [[MTRDeviceController alloc] initWithParameters:params error:error];
}

- (void)test001_CheckAdvertisingAsExpected
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(operationalKeys);

    // Pick some ids that no other test will be using.
    NSNumber * nodeID1 = @(0x1827364554637281);
    NSNumber * nodeID2 = @(0x8172635445362718);
    NSNumber * nodeID3 = @(0x8811772266335544);
    NSNumber * fabricID = @(0x1122334455667788);

    __auto_type * browseExpectation = [self expectationWithDescription:@"Discovered our last controller"];
    // Assume that since we start the controller with nodeID3 last, by the
    // time we see its advertisements we will have seen the ones for the one
    // with nodeID1 too, if it had any.
    __auto_type * operationalBrowser =
        [[MTRControllerAdvertisingTestsOperationalBrowser alloc] initWithExpectation:browseExpectation nodeIDToExpect:nodeID3];
    XCTAssertNotNil(operationalBrowser);

    __auto_type * storageDelegate1 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    NSError * error;
    MTRDeviceController * controller1 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID
                                                                   nodeID:nodeID1
                                                                  storage:storageDelegate1
                                                     advertiseOperational:NO
                                                                    error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller1);
    XCTAssertTrue([controller1 isRunning]);
    XCTAssertEqualObjects(controller1.controllerNodeID, nodeID1);

    __auto_type * storageDelegate2 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    MTRDeviceController * controller2 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID
                                                                   nodeID:nodeID2
                                                                  storage:storageDelegate2
                                                     advertiseOperational:YES
                                                                    error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller2);
    XCTAssertTrue([controller2 isRunning]);
    XCTAssertEqualObjects(controller2.controllerNodeID, nodeID2);

    __auto_type * storageDelegate3 = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];

    MTRDeviceController * controller3 = [self startControllerWithRootKeys:rootKeys
                                                          operationalKeys:operationalKeys
                                                                 fabricID:fabricID
                                                                   nodeID:nodeID3
                                                                  storage:storageDelegate3
                                                     advertiseOperational:YES
                                                                    error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller3);
    XCTAssertTrue([controller3 isRunning]);
    XCTAssertEqualObjects(controller3.controllerNodeID, nodeID3);

    [self waitForExpectations:@[ browseExpectation ] timeout:kTimeoutInSeconds];

    __auto_type * expectedDiscoveredNodes = [NSSet setWithArray:@[ NodeIDAsString(nodeID2), NodeIDAsString(nodeID3) ]];
    XCTAssertEqualObjects(operationalBrowser.discoveredNodes, expectedDiscoveredNodes);

    [controller1 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    [controller2 shutdown];
    XCTAssertFalse([controller2 isRunning]);
    [controller3 shutdown];
    XCTAssertFalse([controller3 isRunning]);
}

@end
