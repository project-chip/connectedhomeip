/**
 *
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

#import "MTRFabricInfoChecker.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static uint16_t kTestVendorId1 = 0xFFF1u;
static uint16_t kTestVendorId2 = 0xFFF2u;

@interface MTRFabricInfoTests : XCTestCase

@end

@implementation MTRFabricInfoTests

/**
 * Override recordIssue to log things better.
 */
- (void)recordIssue:(XCTIssue *)issue
{
    for (XCTSourceCodeFrame * stackFrame in issue.sourceCodeContext.callStack) {
        __auto_type * location = stackFrame.symbolInfo.location;
        if (location != nil) {
            fprintf(stderr, "  %s:%llu\n", location.fileURL.path.UTF8String, (unsigned long long) location.lineNumber);
        }
    }

    [super recordIssue:issue];
}

- (void)testFabricInfoNoFabrics
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * fabricInfoList = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList, [NSMutableSet set]);

    [factory stopControllerFactory];
    XCTAssertFalse([factory isRunning]);
}

- (void)testFabricInfoSingleFabric
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    const unsigned fabricID = 1;
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk
                                                                        fabricID:@(fabricID)
                                                                       nocSigner:testKeys];
    XCTAssertNotNil(params);

    params.vendorID = @(kTestVendorId1);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    __auto_type * nodeID = [controller controllerNodeID];

    __auto_type * fabricInfoList = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[ @{
        @"rootPublicKey" : [testKeys publicKeyData],
        @"vendorID" : @(kTestVendorId1),
        @"fabricID" : @(fabricID),
        @"nodeID" : nodeID,
        @"label" : @"",
        @"hasIntermediateCertificate" : @(NO),
        @"fabricIndex" : @(1)
    } ]]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Should still know about the fabric.
    __auto_type * fabricInfoList2 = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList2, [NSMutableSet setWithArray:@[ @{
        @"rootPublicKey" : [testKeys publicKeyData],
        @"vendorID" : @(kTestVendorId1),
        @"fabricID" : @(fabricID),
        @"nodeID" : nodeID,
        @"label" : @"",
        @"hasIntermediateCertificate" : @(NO),
        @"fabricIndex" : @(1)
    } ]]);

    [factory stopControllerFactory];
    XCTAssertFalse([factory isRunning]);
}

- (void)testFabricInfoTwoFabrics
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startControllerFactory:factoryParams error:nil]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys1 = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys1);

    const unsigned fabricID1 = 8;
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys1.ipk
                                                                        fabricID:@(fabricID1)
                                                                       nocSigner:testKeys1];
    XCTAssertNotNil(params);

    params.vendorID = @(kTestVendorId1);

    MTRDeviceController * controller1 = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller1);
    XCTAssertTrue([controller1 isRunning]);

    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates createRootCertificate:rootKeys issuerID:nil fabricID:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediate = [MTRCertificates createIntermediateCertificate:rootKeys
                                                                rootCertificate:root
                                                          intermediatePublicKey:intermediateKeys.publicKey
                                                                       issuerID:nil
                                                                       fabricID:nil
                                                                          error:nil];
    XCTAssertNotNil(intermediate);

    const unsigned fabricID2 = 17;
    params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:rootKeys.ipk fabricID:@(fabricID2) nocSigner:intermediateKeys];
    XCTAssertNotNil(params);

    params.vendorID = @(kTestVendorId2);
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate;

    MTRDeviceController * controller2 = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller2);
    XCTAssertTrue([controller2 isRunning]);

    __auto_type * nodeID1 = [controller1 controllerNodeID];
    __auto_type * nodeID2 = [controller2 controllerNodeID];

    __auto_type * fabricInfoList = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : [testKeys1 publicKeyData],
            @"vendorID" : @(kTestVendorId1),
            @"fabricID" : @(fabricID1),
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1)
        },
        @{
            @"rootPublicKey" : [rootKeys publicKeyData],
            @"vendorID" : @(kTestVendorId2),
            @"fabricID" : @(fabricID2),
            @"nodeID" : nodeID2,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(YES),
            @"fabricIndex" : @(2)
        }
    ]]);

    [controller1 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    XCTAssertTrue([controller2 isRunning]);

    // Should still know about the fabrics.
    __auto_type * fabricInfoList2 = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList2, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : [testKeys1 publicKeyData],
            @"vendorID" : @(kTestVendorId1),
            @"fabricID" : @(fabricID1),
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1)
        },
        @{
            @"rootPublicKey" : [rootKeys publicKeyData],
            @"vendorID" : @(kTestVendorId2),
            @"fabricID" : @(fabricID2),
            @"nodeID" : nodeID2,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(YES),
            @"fabricIndex" : @(2)
        }
    ]]);

    [controller2 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    XCTAssertFalse([controller2 isRunning]);

    // Should still know about the fabrics.
    __auto_type * fabricInfoList3 = [factory knownFabrics];
    CheckFabricInfo(fabricInfoList3, [NSMutableSet setWithArray:@[
        @{
            @"rootPublicKey" : [testKeys1 publicKeyData],
            @"vendorID" : @(kTestVendorId1),
            @"fabricID" : @(fabricID1),
            @"nodeID" : nodeID1,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(NO),
            @"fabricIndex" : @(1)
        },
        @{
            @"rootPublicKey" : [rootKeys publicKeyData],
            @"vendorID" : @(kTestVendorId2),
            @"fabricID" : @(fabricID2),
            @"nodeID" : nodeID2,
            @"label" : @"",
            @"hasIntermediateCertificate" : @(YES),
            @"fabricIndex" : @(2)
        }
    ]]);

    [factory stopControllerFactory];
    XCTAssertFalse([factory isRunning]);
}

@end
