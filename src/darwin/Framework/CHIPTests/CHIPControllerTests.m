//
//  CHIPControllerTests.m
//  CHIPControllerTests
/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import <CHIP/CHIP.h>

// system dependencies
#import <XCTest/XCTest.h>

#import "CHIPTestKeys.h"
#import "CHIPTestStorage.h"

static uint16_t kTestVendorId = 0xFFF1u;

@interface CHIPControllerTests : XCTestCase

@end

@implementation CHIPControllerTests

- (void)testFactoryLifecycle
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);
    XCTAssertFalse([factory isRunning]);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);

    // Now try to restart the factory.
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerLifecycle
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // now try to restart the controller
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // now try to restart the controller without providing a vendor id.
    params.vendorId = nil;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testFactoryShutdownShutsDownController
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
    XCTAssertFalse([controller isRunning]);
}

- (void)testControllerMultipleShutdown
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    for (int i = 0; i < 5; i++) {
        [controller shutdown];
        XCTAssertFalse([controller isRunning]);
    }

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerInvalidAccess
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertTrue([controller isRunning]);
    [controller shutdown];

    XCTAssertFalse([controller isRunning]);
    XCTAssertFalse([controller getConnectedDevice:1234
                                            queue:dispatch_get_main_queue()
                                completionHandler:^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                                    XCTAssertEqual(error.code, CHIPErrorCodeInvalidState);
                                }]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerNewFabricMatchesOldFabric
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // now try to start a new controller on a new fabric but using the
    // same params; this should fail.
    XCTAssertNil([factory startControllerOnNewFabric:params]);

    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerExistingFabricMatchesRunningController
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    // Now try to start a new controller on the same fabric.  This should fail.
    XCTAssertNil([factory startControllerOnExistingFabric:params]);

    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartControllersOnTwoFabricIds
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params1 = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params1.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller1 = [factory startControllerOnNewFabric:params1];
    XCTAssertNotNil(controller1);
    XCTAssertTrue([controller1 isRunning]);

    // Now try to start a new controller with the same root but a
    // different fabric id.
    __auto_type * params2 = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:2 ipk:testKeys.ipk];
    params2.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller2 = [factory startControllerOnNewFabric:params2];
    XCTAssertNotNil(controller2);
    XCTAssertTrue([controller2 isRunning]);

    XCTAssertNil([factory startControllerOnExistingFabric:params2]);

    [controller1 shutdown];
    XCTAssertFalse([controller1 isRunning]);

    [controller2 shutdown];
    XCTAssertFalse([controller2 isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartControllerSameFabricWrongSubject
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * root1 = [MTRCertificates generateRootCertificate:testKeys issuerId:@1 fabricId:@1 error:nil];
    XCTAssertNotNil(root1);

    __auto_type * root2 = [MTRCertificates generateRootCertificate:testKeys issuerId:@1 fabricId:@1 error:nil];
    XCTAssertNotNil(root2);

    __auto_type * root3 = [MTRCertificates generateRootCertificate:testKeys issuerId:@2 fabricId:@1 error:nil];
    XCTAssertNotNil(root3);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root1;

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now try to start a new controller on the same fabric with what should be
    // a compatible root certificate.
    params.rootCertificate = root2;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    NSNumber * nodeId = [controller controllerNodeId];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now try to start a new controller on the same fabric but with a root
    // certificate that has a different subject.  This should fail for multipler
    // reasons, including our existing operational certificate not matching this
    // root.
    params.rootCertificate = root3;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNil(controller);

    // Now try to start a new controller on the same fabric but with a root
    // certificate that has a different subject while also rotating the
    // operational certificate.  This should fail because our root of trust for
    // the fabric would change if we allowed this.
    params.rootCertificate = root3;
    params.nodeId = nodeId;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNil(controller);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerFabricIdRootCertMismatch
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * testKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * root1 = [MTRCertificates generateRootCertificate:testKeys issuerId:@1 fabricId:@1 error:nil];
    XCTAssertNotNil(root1);

    __auto_type * root2 = [MTRCertificates generateRootCertificate:testKeys issuerId:@1 fabricId:@2 error:nil];
    XCTAssertNotNil(root2);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:testKeys fabricId:1 ipk:testKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Try to start controller when fabric id in root cert subject does not match provided fabric id.
    params.rootCertificate = root2;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    // Start controller when the fabric ids do match.
    params.rootCertificate = root1;
    controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Re-start controller on the new fabric.
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now try to restart controller on the fabric, but with the wrong fabric id
    // in the root cert.
    params.rootCertificate = root2;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNil(controller);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerSignerDoesNotMatchRoot
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * signerKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(signerKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:signerKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root;

    // Try to start controller when there is no ICA and root cert does not match signing key.
    params.rootCertificate = root;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerSignerKeyWithIntermediate
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediate = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                  rootCertificate:root
                                                            intermediatePublicKey:intermediateKeys.pubkey
                                                                         issuerId:nil
                                                                         fabricId:nil
                                                                            error:nil];
    XCTAssertNotNil(intermediate);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Try to start controller when there is an ICA and the ICA cert does not match signing key.
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    // Now start controller with the signing key matching the intermediate cert.
    params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate;
    controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartupParamsInvalidFabric
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    // Invalid fabric ID.
    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:0 ipk:rootKeys.ipk];
    XCTAssertNil(params);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartupParamsInvalidVendor
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    // Invalid vendor ID ("standard").
    params.vendorId = @(0);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartupNodeIdPreserved
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    NSNumber * nodeId = [controller controllerNodeId];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], nodeId);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartupNodeIdUsed
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Bring up with node id 17.
    params.nodeId = @17;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], @17);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Bring up with a different node id (18).
    params.nodeId = @18;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], @18);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Verify the new node id has been stored.
    params.nodeId = nil;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], @18);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerStartupNodeIdValidation
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Try to bring up with node id 0.
    params.nodeId = @0;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    // Try to bring up with node id that is outside of the operational range.
    params.nodeId = @(0xFFFFFFFF00000000ULL);
    controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    // Verify that we can indeed bring up a controller for this fabric, with a valid node id.
    params.nodeId = @17;
    controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], @17);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerRotateToICA
{
    // Tests that we can switch a fabric from not using an ICA to using an ICA.
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediate = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                  rootCertificate:root
                                                            intermediatePublicKey:intermediateKeys.pubkey
                                                                         issuerId:nil
                                                                         fabricId:nil
                                                                            error:nil];
    XCTAssertNotNil(intermediate);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Create a new fabric without the ICA.
    params.rootCertificate = root;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    NSNumber * nodeId = [controller controllerNodeId];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now start controller on the same fabric but using the ICA.
    params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], nodeId);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerRotateFromICA
{
    // Tests that we can switch a fabric from using an ICA to not using an ICA.
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediate = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                  rootCertificate:root
                                                            intermediatePublicKey:intermediateKeys.pubkey
                                                                         issuerId:nil
                                                                         fabricId:nil
                                                                            error:nil];
    XCTAssertNotNil(intermediate);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Create a new fabric without the ICA.
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    NSNumber * nodeId = [controller controllerNodeId];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now start controller on the same fabric but without using the ICA.
    params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:rootKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], nodeId);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerRotateICA
{
    // Tests that we can change the ICA being used for a fabric.
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys1 = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys1);

    __auto_type * intermediate1 = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                   rootCertificate:root
                                                             intermediatePublicKey:intermediateKeys1.pubkey
                                                                          issuerId:nil
                                                                          fabricId:nil
                                                                             error:nil];
    XCTAssertNotNil(intermediate1);

    __auto_type * intermediateKeys2 = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys2);

    __auto_type * intermediate2 = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                   rootCertificate:root
                                                             intermediatePublicKey:intermediateKeys2.pubkey
                                                                          issuerId:nil
                                                                          fabricId:nil
                                                                             error:nil];
    XCTAssertNotNil(intermediate2);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys1
                                                                             fabricId:1
                                                                                  ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Create a new fabric without the first ICA.
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate1;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    NSNumber * nodeId = [controller controllerNodeId];

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    // Now start controller on the same fabric but using the second ICA.
    params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys2 fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);
    params.rootCertificate = root;
    params.intermediateCertificate = intermediate2;
    controller = [factory startControllerOnExistingFabric:params];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);

    XCTAssertEqualObjects([controller controllerNodeId], nodeId);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

- (void)testControllerICAWithoutRoot
{
    __auto_type * factory = [MatterControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[CHIPTestStorage alloc] init];
    __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startup:factoryParams]);
    XCTAssertTrue([factory isRunning]);

    __auto_type * rootKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(rootKeys);

    __auto_type * root = [MTRCertificates generateRootCertificate:rootKeys issuerId:nil fabricId:nil error:nil];
    XCTAssertNotNil(root);

    __auto_type * intermediateKeys = [[CHIPTestKeys alloc] init];
    XCTAssertNotNil(intermediateKeys);

    __auto_type * intermediate = [MTRCertificates generateIntermediateCertificate:rootKeys
                                                                  rootCertificate:root
                                                            intermediatePublicKey:intermediateKeys.pubkey
                                                                         issuerId:nil
                                                                         fabricId:nil
                                                                            error:nil];
    XCTAssertNotNil(intermediate);

    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:intermediateKeys fabricId:1 ipk:rootKeys.ipk];
    params.vendorId = @(kTestVendorId);

    // Pass in an intermediate but no root.  Should fail.
    params.intermediateCertificate = intermediate;
    CHIPDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNil(controller);

    [factory shutdown];
    XCTAssertFalse([factory isRunning]);
}

@end
