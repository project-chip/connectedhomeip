//
//  CHIPClustersTests.m
//  CHIPClustersTests
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

// module headers
#import <CHIP/CHIP.h>

// additional includes
#import "CHIPError.h"

// system dependencies
#import <XCTest/XCTest.h>

const uint16_t kPairingTimeoutInSeconds = 10;
const uint16_t kTimeoutInSeconds = 3;
const uint64_t kDeviceId = 1;
const uint16_t kDiscriminator = 3840;
const uint32_t kSetupPINCode = 20202021;
const uint16_t kRemotePort = 11097;
const uint16_t kLocalPort = 11098;
NSString * kAddress = @"::1";

CHIPDevice * GetPairedDevice(uint64_t deviceId)
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    NSError * pairingError;
    CHIPDevice * device = [controller getPairedDevice:deviceId error:&pairingError];
    XCTAssertEqual(pairingError.code, 0);
    XCTAssertNotNil(device);

    return device;
}

@interface CHIPToolPairingDelegate : NSObject <CHIPDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation CHIPToolPairingDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)onPairingComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}
@end

@interface CHIPClustersTests : XCTestCase
@end

@implementation CHIPClustersTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)testInitStack
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    CHIPToolPairingDelegate * pairing = [[CHIPToolPairingDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    [controller setListenPort:kLocalPort];
    [controller setPairingDelegate:pairing queue:callbackQueue];

    BOOL started = [controller startup:nil];
    XCTAssertTrue(started);

    NSError * error;
    [controller pairDevice:kDeviceId
                   address:kAddress
                      port:kRemotePort
             discriminator:kDiscriminator
              setupPINCode:kSetupPINCode
                     error:&error];
    XCTAssertEqual(error.code, 0);

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];
}

- (void)testShutdownStack
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    NSError * error;
    [controller unpairDevice:kDeviceId error:&error];
    XCTAssertEqual(error.code, 0);

    BOOL stopped = [controller shutdown];
    XCTAssertTrue(stopped);
}

- (void)testSendClusterTestCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterTestCommand"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster test:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testReuseChipClusterObject
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ReuseCHIPClusterObjectFirstCall"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster test:^(NSError * err, NSDictionary * values) {
        NSLog(@"ReuseCHIPClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    expectation = [self expectationWithDescription:@"ReuseCHIPClusterObjectSecondCall"];

    // Reuse the CHIPCluster Object for multiple times.

    [cluster test:^(NSError * err, NSDictionary * values) {
        NSLog(@"ReuseCHIPClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestNotHandledCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterTestNotHandledCommand"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster testNotHandled:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster testNotHandled Error: %@", err);
        XCTAssertEqual(err.code, 1);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestSpecificCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterTestSpecificCommand"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster testSpecific:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster testSpecific Error: %@", err);
        XCTAssertEqual(err.code, 0);
        XCTAssertEqual([values[@"returnValue"] intValue], 7);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterAccountLoginReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"AccountLoginReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPAccountLogin * cluster = [[CHIPAccountLogin alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"AccountLogin ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeVendorNameWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeVendorNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic VendorName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeVendorIdWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeVendorIdWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorIdWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic VendorId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationNameWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeProductIdWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeProductIdWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductIdWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ProductId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationIdWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationIdWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationIdWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeCatalogVendorIdWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeCatalogVendorIdWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCatalogVendorIdWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic CatalogVendorId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationSatusWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationSatusWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationSatusWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationSatus Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationBasicReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationLauncherReadAttributeApplicationLauncherListWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationLauncherReadAttributeApplicationLauncherListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationLauncher * cluster = [[CHIPApplicationLauncher alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationLauncherListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationLauncher ApplicationLauncherList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationLauncherReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ApplicationLauncherReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationLauncher * cluster = [[CHIPApplicationLauncher alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationLauncher ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterAudioOutputReadAttributeAudioOutputListWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"AudioOutputReadAttributeAudioOutputListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPAudioOutput * cluster = [[CHIPAudioOutput alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeAudioOutputListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"AudioOutput AudioOutputList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterAudioOutputReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"AudioOutputReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPAudioOutput * cluster = [[CHIPAudioOutput alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"AudioOutput ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierMovingStateWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BarrierControlReadAttributeBarrierMovingStateWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierMovingStateWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierMovingState Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierSafetyStatusWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BarrierControlReadAttributeBarrierSafetyStatusWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierSafetyStatusWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierSafetyStatus Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierCapabilitiesWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BarrierControlReadAttributeBarrierCapabilitiesWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierCapabilitiesWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierCapabilities Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierPositionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BarrierControlReadAttributeBarrierPositionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierPositionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierPosition Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BarrierControlReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeInteractionModelVersionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BasicReadAttributeInteractionModelVersionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInteractionModelVersionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic InteractionModelVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeVendorNameWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeVendorNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic VendorName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeVendorIDWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeVendorIDWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorIDWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic VendorID Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductNameWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductIDWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductIDWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductIDWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductID Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeUserLabelWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeUserLabelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeUserLabelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic UserLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeUserLabelWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeUserLabelWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    NSString * value = @"Test";
    [cluster writeAttributeUserLabelWithValue:value
                              responseHandler:^(NSError * err, NSDictionary * values) {
                                  NSLog(@"Basic UserLabel Error: %@", err);
                                  XCTAssertEqual(err.code, 0);
                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeLocationWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeLocationWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocationWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic Location Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeLocationWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeLocationWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    NSString * value = @"Test";
    [cluster writeAttributeLocationWithValue:value
                             responseHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"Basic Location Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeHardwareVersionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeHardwareVersionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic HardwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeHardwareVersionStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BasicReadAttributeHardwareVersionStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersionStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic HardwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSoftwareVersionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeSoftwareVersionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SoftwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSoftwareVersionStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BasicReadAttributeSoftwareVersionStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersionStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SoftwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeManufacturingDateWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeManufacturingDateWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeManufacturingDateWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ManufacturingDate Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributePartNumberWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributePartNumberWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePartNumberWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic PartNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductURLWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductURLWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductURLWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductURL Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductLabelWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductLabelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductLabelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSerialNumberWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeSerialNumberWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSerialNumberWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SerialNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeLocalConfigDisabledWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeLocalConfigDisabledWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocalConfigDisabledWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic LocalConfigDisabled Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeLocalConfigDisabledWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeLocalConfigDisabledWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeLocalConfigDisabledWithValue:value
                                        responseHandler:^(NSError * err, NSDictionary * values) {
                                            NSLog(@"Basic LocalConfigDisabled Error: %@", err);
                                            XCTAssertEqual(err.code, 0);
                                            [expectation fulfill];
                                        }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBinaryInputBasicReadAttributeOutOfServiceWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BinaryInputBasicReadAttributeOutOfServiceWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOutOfServiceWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BinaryInputBasic OutOfService Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBinaryInputBasicWriteAttributeOutOfServiceWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BinaryInputBasicWriteAttributeOutOfServiceWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeOutOfServiceWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"BinaryInputBasic OutOfService Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBinaryInputBasicReadAttributePresentValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BinaryInputBasicReadAttributePresentValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePresentValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BinaryInputBasic PresentValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBinaryInputBasicWriteAttributePresentValueWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BinaryInputBasicWriteAttributePresentValueWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0;
    [cluster writeAttributePresentValueWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"BinaryInputBasic PresentValue Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBinaryInputBasicReadAttributeStatusFlagsWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BinaryInputBasicReadAttributeStatusFlagsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeStatusFlagsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BinaryInputBasic StatusFlags Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBinaryInputBasicReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BinaryInputBasicReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinaryInputBasic * cluster = [[CHIPBinaryInputBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BinaryInputBasic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBindingReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BindingReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinding * cluster = [[CHIPBinding alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Binding ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeVendorNameWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeVendorNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic VendorName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeVendorIDWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeVendorIDWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorIDWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic VendorID Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeProductNameWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeProductNameWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductNameWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic ProductName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeUserLabelWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeUserLabelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeUserLabelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic UserLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicWriteAttributeUserLabelWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BridgedDeviceBasicWriteAttributeUserLabelWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    NSString * value = @"Test";
    [cluster writeAttributeUserLabelWithValue:value
                              responseHandler:^(NSError * err, NSDictionary * values) {
                                  NSLog(@"BridgedDeviceBasic UserLabel Error: %@", err);
                                  XCTAssertEqual(err.code, 0);
                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBridgedDeviceBasicReadAttributeHardwareVersionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeHardwareVersionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic HardwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeHardwareVersionStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeHardwareVersionStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersionStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic HardwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeSoftwareVersionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeSoftwareVersionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic SoftwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeSoftwareVersionStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeSoftwareVersionStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersionStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic SoftwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeManufacturingDateWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeManufacturingDateWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeManufacturingDateWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic ManufacturingDate Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributePartNumberWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributePartNumberWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePartNumberWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic PartNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeProductURLWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeProductURLWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductURLWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic ProductURL Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeProductLabelWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeProductLabelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductLabelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic ProductLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeSerialNumberWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeSerialNumberWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSerialNumberWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic SerialNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeReachableWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeReachableWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeReachableWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic Reachable Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBridgedDeviceBasicReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"BridgedDeviceBasicReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBridgedDeviceBasic * cluster = [[CHIPBridgedDeviceBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"BridgedDeviceBasic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentHueWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentHueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentHueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentHue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentSaturationWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeCurrentSaturationWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentSaturationWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentSaturation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeRemainingTimeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeRemainingTimeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeRemainingTimeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl RemainingTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentXWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentXWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentXWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentYWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentYWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentYWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeDriftCompensationWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeDriftCompensationWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeDriftCompensationWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl DriftCompensation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCompensationTextWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeCompensationTextWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCompensationTextWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CompensationText Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTemperatureWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorTemperatureWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTemperatureWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTemperature Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorModeWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorControlOptionsWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorControlOptionsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorControlOptionsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorControlOptions Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorControlOptionsWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorControlOptionsWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorControlOptionsWithValue:value
                                        responseHandler:^(NSError * err, NSDictionary * values) {
                                            NSLog(@"ColorControl ColorControlOptions Error: %@", err);
                                            XCTAssertEqual(err.code, 0);
                                            [expectation fulfill];
                                        }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeNumberOfPrimariesWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeNumberOfPrimariesWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNumberOfPrimariesWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl NumberOfPrimaries Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary1XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary1YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary1IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary2XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary2YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary2IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary3XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary3YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary3IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary4XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary4YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary4IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary5XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary5YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary5IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6XWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary6XWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6XWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6YWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary6YWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6YWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6IntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributePrimary6IntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6IntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeWhitePointXWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeWhitePointXWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWhitePointXWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl WhitePointX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeWhitePointXWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeWhitePointXWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeWhitePointXWithValue:value
                                responseHandler:^(NSError * err, NSDictionary * values) {
                                    NSLog(@"ColorControl WhitePointX Error: %@", err);
                                    XCTAssertEqual(err.code, 0);
                                    [expectation fulfill];
                                }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeWhitePointYWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeWhitePointYWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWhitePointYWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl WhitePointY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeWhitePointYWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeWhitePointYWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeWhitePointYWithValue:value
                                responseHandler:^(NSError * err, NSDictionary * values) {
                                    NSLog(@"ColorControl WhitePointY Error: %@", err);
                                    XCTAssertEqual(err.code, 0);
                                    [expectation fulfill];
                                }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRXWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointRXWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRXWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRXWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRXWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointRXWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointRX Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRYWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointRYWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRYWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRYWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRYWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointRYWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointRY Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRIntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorPointRIntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRIntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRIntensityWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRIntensityWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointRIntensityWithValue:value
                                         responseHandler:^(NSError * err, NSDictionary * values) {
                                             NSLog(@"ColorControl ColorPointRIntensity Error: %@", err);
                                             XCTAssertEqual(err.code, 0);
                                             [expectation fulfill];
                                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGXWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointGXWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGXWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGXWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGXWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointGXWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointGX Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGYWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointGYWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGYWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGYWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGYWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointGYWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointGY Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGIntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorPointGIntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGIntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGIntensityWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGIntensityWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointGIntensityWithValue:value
                                         responseHandler:^(NSError * err, NSDictionary * values) {
                                             NSLog(@"ColorControl ColorPointGIntensity Error: %@", err);
                                             XCTAssertEqual(err.code, 0);
                                             [expectation fulfill];
                                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBXWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointBXWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBXWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBXWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBXWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointBXWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointBX Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBYWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointBYWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBYWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBYWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBYWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointBYWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"ColorControl ColorPointBY Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBIntensityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorPointBIntensityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBIntensityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBIntensityWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBIntensityWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointBIntensityWithValue:value
                                         responseHandler:^(NSError * err, NSDictionary * values) {
                                             NSLog(@"ColorControl ColorPointBIntensity Error: %@", err);
                                             XCTAssertEqual(err.code, 0);
                                             [expectation fulfill];
                                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeEnhancedCurrentHueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeEnhancedCurrentHueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnhancedCurrentHueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl EnhancedCurrentHue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeEnhancedColorModeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeEnhancedColorModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnhancedColorModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl EnhancedColorMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopActiveWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorLoopActiveWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopActiveWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopActive Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopDirectionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorLoopDirectionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopDirectionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopDirection Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopTimeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorLoopTimeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopTimeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorCapabilitiesWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorCapabilitiesWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorCapabilitiesWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorCapabilities Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTempPhysicalMinWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorTempPhysicalMinWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTempPhysicalMinWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTempPhysicalMin Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTempPhysicalMaxWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeColorTempPhysicalMaxWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTempPhysicalMaxWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTempPhysicalMax Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCoupleColorTempToLevelMinMiredsWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeCoupleColorTempToLevelMinMiredsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCoupleColorTempToLevelMinMiredsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CoupleColorTempToLevelMinMireds Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeStartUpColorTemperatureMiredsWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeStartUpColorTemperatureMiredsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeStartUpColorTemperatureMiredsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl StartUpColorTemperatureMireds Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeStartUpColorTemperatureMiredsWithValue
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlWriteAttributeStartUpColorTemperatureMiredsWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeStartUpColorTemperatureMiredsWithValue:value
                                                  responseHandler:^(NSError * err, NSDictionary * values) {
                                                      NSLog(@"ColorControl StartUpColorTemperatureMireds Error: %@", err);
                                                      XCTAssertEqual(err.code, 0);
                                                      [expectation fulfill];
                                                  }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ColorControlReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterContentLaunchReadAttributeAcceptsHeaderListWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ContentLaunchReadAttributeAcceptsHeaderListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPContentLaunch * cluster = [[CHIPContentLaunch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeAcceptsHeaderListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ContentLaunch AcceptsHeaderList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterContentLaunchReadAttributeSupportedStreamingTypesWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ContentLaunchReadAttributeSupportedStreamingTypesWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPContentLaunch * cluster = [[CHIPContentLaunch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSupportedStreamingTypesWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ContentLaunch SupportedStreamingTypes Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterContentLaunchReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ContentLaunchReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPContentLaunch * cluster = [[CHIPContentLaunch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"ContentLaunch ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeDeviceListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeDeviceListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeDeviceListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor DeviceList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeServerListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeServerListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeServerListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ServerList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeClientListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeClientListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClientListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ClientList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributePartsListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributePartsListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePartsListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor PartsList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"DescriptorReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeLockStateWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeLockStateWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLockStateWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock LockState Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeLockTypeWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeLockTypeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLockTypeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock LockType Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeActuatorEnabledWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeActuatorEnabledWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeActuatorEnabledWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ActuatorEnabled Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributePacketRxCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributePacketRxCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePacketRxCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics PacketRxCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributePacketTxCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributePacketTxCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePacketTxCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics PacketTxCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributeTxErrCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributeTxErrCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeTxErrCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics TxErrCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributeCollisionCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributeCollisionCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCollisionCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics CollisionCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributeOverrunCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributeOverrunCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOverrunCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics OverrunCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterEthernetNetworkDiagnosticsReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"EthernetNetworkDiagnosticsReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPEthernetNetworkDiagnostics * cluster = [[CHIPEthernetNetworkDiagnostics alloc] initWithDevice:device
                                                                                             endpoint:0
                                                                                                queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"EthernetNetworkDiagnostics ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterFixedLabelReadAttributeLabelListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"FixedLabelReadAttributeLabelListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPFixedLabel * cluster = [[CHIPFixedLabel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLabelListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"FixedLabel LabelList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterFixedLabelReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"FixedLabelReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPFixedLabel * cluster = [[CHIPFixedLabel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"FixedLabel ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningReadAttributeFabricIdWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralCommissioningReadAttributeFabricIdWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeFabricIdWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning FabricId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningReadAttributeBreadcrumbWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralCommissioningReadAttributeBreadcrumbWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBreadcrumbWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning Breadcrumb Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningWriteAttributeBreadcrumbWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningWriteAttributeBreadcrumbWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    uint64_t value = 0x0000000000000000;
    [cluster writeAttributeBreadcrumbWithValue:value
                               responseHandler:^(NSError * err, NSDictionary * values) {
                                   NSLog(@"GeneralCommissioning Breadcrumb Error: %@", err);
                                   XCTAssertEqual(err.code, 0);
                                   [expectation fulfill];
                               }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterGeneralCommissioningReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralCommissioningReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralDiagnosticsReadAttributeNetworkInterfacesWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralDiagnosticsReadAttributeNetworkInterfacesWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralDiagnostics * cluster = [[CHIPGeneralDiagnostics alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNetworkInterfacesWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralDiagnostics NetworkInterfaces Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralDiagnosticsReadAttributeRebootCountWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralDiagnosticsReadAttributeRebootCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralDiagnostics * cluster = [[CHIPGeneralDiagnostics alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeRebootCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralDiagnostics RebootCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralDiagnosticsReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GeneralDiagnosticsReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralDiagnostics * cluster = [[CHIPGeneralDiagnostics alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralDiagnostics ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeGroupsWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupKeyManagementReadAttributeGroupsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeGroupsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement Groups Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeGroupKeysWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GroupKeyManagementReadAttributeGroupKeysWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeGroupKeysWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement GroupKeys Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"GroupKeyManagementReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupsReadAttributeNameSupportWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupsReadAttributeNameSupportWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroups * cluster = [[CHIPGroups alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNameSupportWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Groups NameSupport Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupsReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupsReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroups * cluster = [[CHIPGroups alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Groups ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterIdentifyReadAttributeIdentifyTimeWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyReadAttributeIdentifyTimeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeIdentifyTimeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Identify IdentifyTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterIdentifyWriteAttributeIdentifyTimeWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyWriteAttributeIdentifyTimeWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeIdentifyTimeWithValue:value
                                 responseHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"Identify IdentifyTime Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterIdentifyReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Identify ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterKeypadInputReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"KeypadInputReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPKeypadInput * cluster = [[CHIPKeypadInput alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"KeypadInput ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLevelControlReadAttributeCurrentLevelWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LevelControlReadAttributeCurrentLevelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLevelControl * cluster = [[CHIPLevelControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentLevelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"LevelControl CurrentLevel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLevelControlReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"LevelControlReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLevelControl * cluster = [[CHIPLevelControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"LevelControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLowPowerReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LowPowerReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLowPower * cluster = [[CHIPLowPower alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"LowPower ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterMediaInputReadAttributeMediaInputListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"MediaInputReadAttributeMediaInputListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPMediaInput * cluster = [[CHIPMediaInput alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMediaInputListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"MediaInput MediaInputList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterMediaInputReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"MediaInputReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPMediaInput * cluster = [[CHIPMediaInput alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"MediaInput ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterMediaPlaybackReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"MediaPlaybackReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPMediaPlayback * cluster = [[CHIPMediaPlayback alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"MediaPlayback ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterNetworkCommissioningReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"NetworkCommissioningReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPNetworkCommissioning * cluster = [[CHIPNetworkCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"NetworkCommissioning ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOnOffReadAttributeOnOffWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffReadAttributeOnOffWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOnOffWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff OnOff Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOnOffReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOperationalCredentialsReadAttributeFabricsListWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"OperationalCredentialsReadAttributeFabricsListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOperationalCredentials * cluster = [[CHIPOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeFabricsListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"OperationalCredentials FabricsList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOperationalCredentialsReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"OperationalCredentialsReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOperationalCredentials * cluster = [[CHIPOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"OperationalCredentials ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxPressureWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxPressureWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxPressureWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxPressure Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxSpeedWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxSpeedWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxSpeedWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxSpeed Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxFlowWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxFlowWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxFlowWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxFlow Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeEffectiveOperationModeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeEffectiveOperationModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEffectiveOperationModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl EffectiveOperationMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeEffectiveControlModeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeEffectiveControlModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEffectiveControlModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl EffectiveControlMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeCapacityWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeCapacityWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCapacityWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl Capacity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeOperationModeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeOperationModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOperationModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl OperationMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlWriteAttributeOperationModeWithValue
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlWriteAttributeOperationModeWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeOperationModeWithValue:value
                                  responseHandler:^(NSError * err, NSDictionary * values) {
                                      NSLog(@"PumpConfigurationAndControl OperationMode Error: %@", err);
                                      XCTAssertEqual(err.code, 0);
                                      [expectation fulfill];
                                  }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterPumpConfigurationAndControlReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterRelativeHumidityMeasurementReadAttributeMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"RelativeHumidityMeasurementReadAttributeMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPRelativeHumidityMeasurement * cluster = [[CHIPRelativeHumidityMeasurement alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"RelativeHumidityMeasurement MeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterRelativeHumidityMeasurementReadAttributeMinMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"RelativeHumidityMeasurementReadAttributeMinMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPRelativeHumidityMeasurement * cluster = [[CHIPRelativeHumidityMeasurement alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMinMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"RelativeHumidityMeasurement MinMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterRelativeHumidityMeasurementReadAttributeMaxMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"RelativeHumidityMeasurementReadAttributeMaxMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPRelativeHumidityMeasurement * cluster = [[CHIPRelativeHumidityMeasurement alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"RelativeHumidityMeasurement MaxMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterRelativeHumidityMeasurementReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"RelativeHumidityMeasurementReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPRelativeHumidityMeasurement * cluster = [[CHIPRelativeHumidityMeasurement alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"RelativeHumidityMeasurement ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeSceneCountWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeSceneCountWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSceneCountWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes SceneCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeCurrentSceneWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeCurrentSceneWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentSceneWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes CurrentScene Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeCurrentGroupWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeCurrentGroupWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentGroupWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes CurrentGroup Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeSceneValidWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeSceneValidWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSceneValidWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes SceneValid Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeNameSupportWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeNameSupportWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNameSupportWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes NameSupport Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSoftwareDiagnosticsReadAttributeCurrentHeapHighWatermarkWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"SoftwareDiagnosticsReadAttributeCurrentHeapHighWatermarkWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSoftwareDiagnostics * cluster = [[CHIPSoftwareDiagnostics alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentHeapHighWatermarkWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"SoftwareDiagnostics CurrentHeapHighWatermark Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSoftwareDiagnosticsReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"SoftwareDiagnosticsReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSoftwareDiagnostics * cluster = [[CHIPSoftwareDiagnostics alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"SoftwareDiagnostics ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeNumberOfPositionsWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeNumberOfPositionsWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNumberOfPositionsWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch NumberOfPositions Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeCurrentPositionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeCurrentPositionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentPositionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch CurrentPosition Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTvChannelReadAttributeTvChannelListWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TvChannelReadAttributeTvChannelListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTvChannel * cluster = [[CHIPTvChannel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeTvChannelListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TvChannel TvChannelList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTvChannelReadAttributeTvChannelLineupWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TvChannelReadAttributeTvChannelLineupWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTvChannel * cluster = [[CHIPTvChannel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeTvChannelLineupWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TvChannel TvChannelLineup Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTvChannelReadAttributeCurrentTvChannelWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TvChannelReadAttributeCurrentTvChannelWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTvChannel * cluster = [[CHIPTvChannel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentTvChannelWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TvChannel CurrentTvChannel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTvChannelReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TvChannelReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTvChannel * cluster = [[CHIPTvChannel alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TvChannel ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTargetNavigatorReadAttributeTargetNavigatorListWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TargetNavigatorReadAttributeTargetNavigatorListWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTargetNavigator * cluster = [[CHIPTargetNavigator alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeTargetNavigatorListWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TargetNavigator TargetNavigatorList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTargetNavigatorReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TargetNavigatorReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTargetNavigator * cluster = [[CHIPTargetNavigator alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TargetNavigator ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMinMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMinMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMinMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MinMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMaxMeasuredValueWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMaxMeasuredValueWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxMeasuredValueWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MaxMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TemperatureMeasurementReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterReadAttributeBooleanWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeBooleanWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBooleanWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Boolean Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeBooleanWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeBooleanWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0;
    [cluster writeAttributeBooleanWithValue:value
                            responseHandler:^(NSError * err, NSDictionary * values) {
                                NSLog(@"TestCluster Boolean Error: %@", err);
                                XCTAssertEqual(err.code, 0);
                                [expectation fulfill];
                            }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeBitmap8WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeBitmap8WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBitmap8WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Bitmap8 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeBitmap8WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeBitmap8WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0;
    [cluster writeAttributeBitmap8WithValue:value
                            responseHandler:^(NSError * err, NSDictionary * values) {
                                NSLog(@"TestCluster Bitmap8 Error: %@", err);
                                XCTAssertEqual(err.code, 0);
                                [expectation fulfill];
                            }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeBitmap16WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeBitmap16WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBitmap16WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Bitmap16 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeBitmap16WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeBitmap16WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0;
    [cluster writeAttributeBitmap16WithValue:value
                             responseHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"TestCluster Bitmap16 Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeBitmap32WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeBitmap32WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBitmap32WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Bitmap32 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeBitmap32WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeBitmap32WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint32_t value = 0;
    [cluster writeAttributeBitmap32WithValue:value
                             responseHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"TestCluster Bitmap32 Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeBitmap64WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeBitmap64WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBitmap64WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Bitmap64 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeBitmap64WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeBitmap64WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint64_t value = 0;
    [cluster writeAttributeBitmap64WithValue:value
                             responseHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"TestCluster Bitmap64 Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt8uWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt8uWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt8uWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int8u Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt8uWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt8uWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0;
    [cluster writeAttributeInt8uWithValue:value
                          responseHandler:^(NSError * err, NSDictionary * values) {
                              NSLog(@"TestCluster Int8u Error: %@", err);
                              XCTAssertEqual(err.code, 0);
                              [expectation fulfill];
                          }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt16uWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt16uWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt16uWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int16u Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt16uWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt16uWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0;
    [cluster writeAttributeInt16uWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int16u Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt32uWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt32uWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt32uWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int32u Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt32uWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt32uWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint32_t value = 0;
    [cluster writeAttributeInt32uWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int32u Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt64uWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt64uWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt64uWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int64u Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt64uWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt64uWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint64_t value = 0;
    [cluster writeAttributeInt64uWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int64u Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt8sWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt8sWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt8sWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int8s Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt8sWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt8sWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int8_t value = 0;
    [cluster writeAttributeInt8sWithValue:value
                          responseHandler:^(NSError * err, NSDictionary * values) {
                              NSLog(@"TestCluster Int8s Error: %@", err);
                              XCTAssertEqual(err.code, 0);
                              [expectation fulfill];
                          }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt16sWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt16sWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt16sWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int16s Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt16sWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt16sWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int16_t value = 0;
    [cluster writeAttributeInt16sWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int16s Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt32sWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt32sWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt32sWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int32s Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt32sWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt32sWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int32_t value = 0;
    [cluster writeAttributeInt32sWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int32s Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeInt64sWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeInt64sWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInt64sWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Int64s Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeInt64sWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeInt64sWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int64_t value = 0;
    [cluster writeAttributeInt64sWithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Int64s Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeEnum8WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeEnum8WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnum8WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Enum8 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeEnum8WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeEnum8WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0;
    [cluster writeAttributeEnum8WithValue:value
                          responseHandler:^(NSError * err, NSDictionary * values) {
                              NSLog(@"TestCluster Enum8 Error: %@", err);
                              XCTAssertEqual(err.code, 0);
                              [expectation fulfill];
                          }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeEnum16WithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeEnum16WithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnum16WithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster Enum16 Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeEnum16WithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeEnum16WithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0;
    [cluster writeAttributeEnum16WithValue:value
                           responseHandler:^(NSError * err, NSDictionary * values) {
                               NSLog(@"TestCluster Enum16 Error: %@", err);
                               XCTAssertEqual(err.code, 0);
                               [expectation fulfill];
                           }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeOctetStringWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeOctetStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOctetStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster OctetString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterWriteAttributeOctetStringWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterWriteAttributeOctetStringWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    NSData * value = [@"Test" dataUsingEncoding:NSUTF8StringEncoding];
    [cluster writeAttributeOctetStringWithValue:value
                                responseHandler:^(NSError * err, NSDictionary * values) {
                                    NSLog(@"TestCluster OctetString Error: %@", err);
                                    XCTAssertEqual(err.code, 0);
                                    [expectation fulfill];
                                }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterTestClusterReadAttributeListInt8uWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TestClusterReadAttributeListInt8uWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeListInt8uWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster ListInt8u Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterReadAttributeListOctetStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TestClusterReadAttributeListOctetStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeListOctetStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster ListOctetString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterReadAttributeListStructOctetStringWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TestClusterReadAttributeListStructOctetStringWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeListStructOctetStringWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster ListStructOctetString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTestClusterReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TestClusterReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTestCluster * cluster = [[CHIPTestCluster alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TestCluster ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatReadAttributeLocalTemperatureWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatReadAttributeLocalTemperatureWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocalTemperatureWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat LocalTemperature Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatReadAttributeOccupiedCoolingSetpointWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatReadAttributeOccupiedCoolingSetpointWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOccupiedCoolingSetpointWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat OccupiedCoolingSetpoint Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeOccupiedCoolingSetpointWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeOccupiedCoolingSetpointWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int16_t value = 0;
    [cluster writeAttributeOccupiedCoolingSetpointWithValue:value
                                            responseHandler:^(NSError * err, NSDictionary * values) {
                                                NSLog(@"Thermostat OccupiedCoolingSetpoint Error: %@", err);
                                                XCTAssertEqual(err.code, 0);
                                                [expectation fulfill];
                                            }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeOccupiedHeatingSetpointWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatReadAttributeOccupiedHeatingSetpointWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOccupiedHeatingSetpointWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat OccupiedHeatingSetpoint Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeOccupiedHeatingSetpointWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeOccupiedHeatingSetpointWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int16_t value = 0;
    [cluster writeAttributeOccupiedHeatingSetpointWithValue:value
                                            responseHandler:^(NSError * err, NSDictionary * values) {
                                                NSLog(@"Thermostat OccupiedHeatingSetpoint Error: %@", err);
                                                XCTAssertEqual(err.code, 0);
                                                [expectation fulfill];
                                            }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeControlSequenceOfOperationWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatReadAttributeControlSequenceOfOperationWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeControlSequenceOfOperationWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat ControlSequenceOfOperation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeControlSequenceOfOperationWithValue
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatWriteAttributeControlSequenceOfOperationWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeControlSequenceOfOperationWithValue:value
                                               responseHandler:^(NSError * err, NSDictionary * values) {
                                                   NSLog(@"Thermostat ControlSequenceOfOperation Error: %@", err);
                                                   XCTAssertEqual(err.code, 0);
                                                   [expectation fulfill];
                                               }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeSystemModeWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeSystemModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSystemModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat SystemMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeSystemModeWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeSystemModeWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeSystemModeWithValue:value
                               responseHandler:^(NSError * err, NSDictionary * values) {
                                   NSLog(@"Thermostat SystemMode Error: %@", err);
                                   XCTAssertEqual(err.code, 0);
                                   [expectation fulfill];
                               }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"ThermostatReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTrustedRootCertificatesReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"TrustedRootCertificatesReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTrustedRootCertificates * cluster = [[CHIPTrustedRootCertificates alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"TrustedRootCertificates ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWakeOnLanReadAttributeWakeOnLanMacAddressWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WakeOnLanReadAttributeWakeOnLanMacAddressWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWakeOnLan * cluster = [[CHIPWakeOnLan alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWakeOnLanMacAddressWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WakeOnLan WakeOnLanMacAddress Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWakeOnLanReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"WakeOnLanReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWakeOnLan * cluster = [[CHIPWakeOnLan alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WakeOnLan ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeWindowCoveringTypeWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeWindowCoveringTypeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWindowCoveringTypeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering WindowCoveringType Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeCurrentPositionLiftWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeCurrentPositionLiftWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentPositionLiftWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering CurrentPositionLift Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeCurrentPositionTiltWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeCurrentPositionTiltWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentPositionTiltWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering CurrentPositionTilt Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeConfigStatusWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeConfigStatusWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeConfigStatusWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering ConfigStatus Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeInstalledOpenLimitLiftWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeInstalledOpenLimitLiftWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInstalledOpenLimitLiftWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering InstalledOpenLimitLift Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeInstalledClosedLimitLiftWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeInstalledClosedLimitLiftWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInstalledClosedLimitLiftWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering InstalledClosedLimitLift Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeInstalledOpenLimitTiltWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeInstalledOpenLimitTiltWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInstalledOpenLimitTiltWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering InstalledOpenLimitTilt Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeInstalledClosedLimitTiltWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeInstalledClosedLimitTiltWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInstalledClosedLimitTiltWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering InstalledClosedLimitTilt Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringReadAttributeModeWithResponseHandler
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"WindowCoveringReadAttributeModeWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeModeWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering Mode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterWindowCoveringWriteAttributeModeWithValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"WindowCoveringWriteAttributeModeWithValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeModeWithValue:value
                         responseHandler:^(NSError * err, NSDictionary * values) {
                             NSLog(@"WindowCovering Mode Error: %@", err);
                             XCTAssertEqual(err.code, 0);
                             [expectation fulfill];
                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterWindowCoveringReadAttributeClusterRevisionWithResponseHandler
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"WindowCoveringReadAttributeClusterRevisionWithResponseHandler"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPWindowCovering * cluster = [[CHIPWindowCovering alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevisionWithResponseHandler:^(NSError * err, NSDictionary * values) {
        NSLog(@"WindowCovering ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

@end
