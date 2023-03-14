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

#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

static uint16_t kTestVendorId1 = 0xFFF1u;
static uint16_t kTestVendorId2 = 0xFFF2u;

// Macro so that we get useful line numbers
#define CHECK_FABRIC_INFO(fabricInfoList, expected)                                                                                \
    do {                                                                                                                           \
        XCTAssertEqual([fabricInfoList count], [expected count]);                                                                  \
        for (size_t i = 0; i < [fabricInfoList count]; ++i) {                                                                      \
            __auto_type * info = fabricInfoList[i];                                                                                \
            XCTAssertEqualObjects(info.rootPublicKey, expected[i][@"rootPublicKey"]);                                              \
            XCTAssertEqualObjects(info.vendorID, expected[i][@"vendorID"]);                                                        \
            XCTAssertEqualObjects(info.fabricID, expected[i][@"fabricID"]);                                                        \
            XCTAssertEqualObjects(info.nodeID, expected[i][@"nodeID"]);                                                            \
            XCTAssertEqualObjects(info.label, expected[i][@"label"]);                                                              \
            XCTAssertNotNil(info.rootCertificate);                                                                                 \
            XCTAssertNotNil(info.rootCertificateTLV);                                                                              \
            XCTAssertEqualObjects([MTRCertificates convertX509Certificate:info.rootCertificate], info.rootCertificateTLV);         \
            XCTAssertEqualObjects([MTRCertificates convertMatterCertificate:info.rootCertificateTLV], info.rootCertificate);       \
            XCTAssertEqual((info.intermediateCertificate == nil), (info.intermediateCertificateTLV == nil));                       \
            XCTAssertEqualObjects(@(info.intermediateCertificate != nil), expected[i][@"hasIntermediateCertificate"]);             \
            XCTAssertEqualObjects(                                                                                                 \
                [MTRCertificates convertX509Certificate:info.intermediateCertificate], info.intermediateCertificateTLV);           \
            XCTAssertEqualObjects(                                                                                                 \
                [MTRCertificates convertMatterCertificate:info.intermediateCertificateTLV], info.intermediateCertificate);         \
            XCTAssertNotNil(info.operationalCertificate);                                                                          \
            XCTAssertNotNil(info.operationalCertificateTLV);                                                                       \
            XCTAssertEqualObjects(                                                                                                 \
                [MTRCertificates convertX509Certificate:info.operationalCertificate], info.operationalCertificateTLV);             \
            XCTAssertEqualObjects(                                                                                                 \
                [MTRCertificates convertMatterCertificate:info.operationalCertificateTLV], info.operationalCertificate);           \
            __auto_type * certInfo = [[MTRCertificateInfo alloc] initWithTLVBytes:info.operationalCertificateTLV];                 \
            XCTAssertNotNil(certInfo);                                                                                             \
            XCTAssertEqualObjects(certInfo.subject.nodeID, info.nodeID);                                                           \
            XCTAssertEqualObjects(certInfo.subject.fabricID, info.fabricID);                                                       \
            XCTAssertEqualObjects(info.fabricIndex, expected[i][@"fabricIndex"]);                                                  \
        }                                                                                                                          \
    } while (0)

@interface MTRFabricInfoTests : XCTestCase

@end

@implementation MTRFabricInfoTests

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
    CHECK_FABRIC_INFO(fabricInfoList, @[]);

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
    CHECK_FABRIC_INFO(fabricInfoList, (@[ @{
        @"rootPublicKey" : [testKeys publicKeyData],
        @"vendorID" : @(kTestVendorId1),
        @"fabricID" : @(fabricID),
        @"nodeID" : nodeID,
        @"label" : @"",
        @"hasIntermediateCertificate" : @(NO),
        @"fabricIndex" : @(1)
    } ]));

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Should still know about the fabric.
    __auto_type * fabricInfoList2 = [factory knownFabrics];
    CHECK_FABRIC_INFO(fabricInfoList2, (@[ @{
        @"rootPublicKey" : [testKeys publicKeyData],
        @"vendorID" : @(kTestVendorId1),
        @"fabricID" : @(fabricID),
        @"nodeID" : nodeID,
        @"label" : @"",
        @"hasIntermediateCertificate" : @(NO),
        @"fabricIndex" : @(1)
    } ]));

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
    CHECK_FABRIC_INFO(fabricInfoList, (@[
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
    ]));

    [controller1 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    XCTAssertTrue([controller2 isRunning]);

    // Should still know about the fabrics.
    __auto_type * fabricInfoList2 = [factory knownFabrics];
    CHECK_FABRIC_INFO(fabricInfoList2, (@[
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
    ]));

    [controller2 shutdown];
    XCTAssertFalse([controller1 isRunning]);
    XCTAssertFalse([controller2 isRunning]);

    // Should still know about the fabrics.
    __auto_type * fabricInfoList3 = [factory knownFabrics];
    CHECK_FABRIC_INFO(fabricInfoList3, (@[
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
    ]));

    [factory stopControllerFactory];
    XCTAssertFalse([factory isRunning]);
}

@end
