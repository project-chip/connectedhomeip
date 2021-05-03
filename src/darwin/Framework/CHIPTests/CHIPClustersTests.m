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

    BOOL started = [controller startup:nil queue:nil];
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

- (void)testSendClusterBarrierControlBarrierControlStopCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlBarrierControlStop"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster barrierControlStop:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierControlStop Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicMfgSpecificPingCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicMfgSpecificPing"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster mfgSpecificPing:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic MfgSpecificPing Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterDoorLockClearAllPinsCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockClearAllPins"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster clearAllPins:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ClearAllPins Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterDoorLockClearAllRfidsCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockClearAllRfids"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster clearAllRfids:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ClearAllRfids Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterGeneralCommissioningCommissioningCompleteCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningCommissioningComplete"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster commissioningComplete:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning CommissioningComplete Error: %@", err);
        XCTAssertEqual(err.code, 1);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterGroupsRemoveAllGroupsCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupsRemoveAllGroups"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroups * cluster = [[CHIPGroups alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster removeAllGroups:^(NSError * err, NSDictionary * values) {
        NSLog(@"Groups RemoveAllGroups Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterIdentifyIdentifyQueryCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyIdentifyQuery"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster identifyQuery:^(NSError * err, NSDictionary * values) {
        NSLog(@"Identify IdentifyQuery Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterLevelControlStopWithOnOffCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LevelControlStopWithOnOff"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLevelControl * cluster = [[CHIPLevelControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster stopWithOnOff:^(NSError * err, NSDictionary * values) {
        NSLog(@"LevelControl StopWithOnOff Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterLowPowerSleepCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LowPowerSleep"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLowPower * cluster = [[CHIPLowPower alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster sleep:^(NSError * err, NSDictionary * values) {
        NSLog(@"LowPower Sleep Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterOnOffOffCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffOff"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster off:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff Off Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterOnOffOnCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffOn"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster on:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff On Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterOnOffToggleCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffToggle"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster toggle:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff Toggle Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterOperationalCredentialsGetFabricIdCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OperationalCredentialsGetFabricId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOperationalCredentials * cluster = [[CHIPOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster getFabricId:^(NSError * err, NSDictionary * values) {
        NSLog(@"OperationalCredentials GetFabricId Error: %@", err);
        XCTAssertEqual(err.code, 1);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatClearWeeklyScheduleCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatClearWeeklySchedule"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster clearWeeklySchedule:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat ClearWeeklySchedule Error: %@", err);
        XCTAssertEqual(err.code, 1);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatGetRelayStatusLogCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatGetRelayStatusLog"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster getRelayStatusLog:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat GetRelayStatusLog Error: %@", err);
        XCTAssertEqual(err.code, 1);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeVendorName
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeVendorName"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorName:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic VendorName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeVendorId
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeVendorId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorId:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic VendorId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationName
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationName"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationName:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeProductId
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeProductId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductId:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ProductId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationId
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationId:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeCatalogVendorId
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeCatalogVendorId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCatalogVendorId:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic CatalogVendorId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeApplicationSatus
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeApplicationSatus"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeApplicationSatus:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ApplicationSatus Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterApplicationBasicReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ApplicationBasicReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPApplicationBasic * cluster = [[CHIPApplicationBasic alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"ApplicationBasic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierMovingState
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlReadAttributeBarrierMovingState"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierMovingState:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierMovingState Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierSafetyStatus
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlReadAttributeBarrierSafetyStatus"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierSafetyStatus:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierSafetyStatus Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierCapabilities
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlReadAttributeBarrierCapabilities"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierCapabilities:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierCapabilities Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeBarrierPosition
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlReadAttributeBarrierPosition"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBarrierPosition:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl BarrierPosition Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBarrierControlReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BarrierControlReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBarrierControl * cluster = [[CHIPBarrierControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"BarrierControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeInteractionModelVersion
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeInteractionModelVersion"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeInteractionModelVersion:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic InteractionModelVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeVendorName
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeVendorName"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorName:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic VendorName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeVendorID
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeVendorID"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeVendorID:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic VendorID Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductName
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductName"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductName:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductName Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductID
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductID"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductID:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductID Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeUserLabel
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeUserLabel"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeUserLabel:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic UserLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeUserLabel
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeUserLabel"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    NSString * value = @"Test";
    [cluster writeAttributeUserLabel:value
                   completionHandler:^(NSError * err, NSDictionary * values) {
                       NSLog(@"Basic UserLabel Error: %@", err);
                       XCTAssertEqual(err.code, 0);
                       [expectation fulfill];
                   }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeLocation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeLocation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocation:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic Location Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeLocation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeLocation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    NSString * value = @"Test";
    [cluster writeAttributeLocation:value
                  completionHandler:^(NSError * err, NSDictionary * values) {
                      NSLog(@"Basic Location Error: %@", err);
                      XCTAssertEqual(err.code, 0);
                      [expectation fulfill];
                  }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeHardwareVersion
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeHardwareVersion"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersion:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic HardwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeHardwareVersionString
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeHardwareVersionString"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeHardwareVersionString:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic HardwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSoftwareVersion
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeSoftwareVersion"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersion:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SoftwareVersion Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSoftwareVersionString
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeSoftwareVersionString"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSoftwareVersionString:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SoftwareVersionString Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeManufacturingDate
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeManufacturingDate"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeManufacturingDate:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ManufacturingDate Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributePartNumber
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributePartNumber"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePartNumber:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic PartNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductURL
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductURL"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductURL:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductURL Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeProductLabel
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeProductLabel"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeProductLabel:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ProductLabel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeSerialNumber
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeSerialNumber"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSerialNumber:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic SerialNumber Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicReadAttributeLocalConfigDisabled
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeLocalConfigDisabled"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocalConfigDisabled:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic LocalConfigDisabled Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBasicWriteAttributeLocalConfigDisabled
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicWriteAttributeLocalConfigDisabled"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeLocalConfigDisabled:value
                             completionHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"Basic LocalConfigDisabled Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterBasicReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BasicReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBasic * cluster = [[CHIPBasic alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Basic ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterBindingReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"BindingReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPBinding * cluster = [[CHIPBinding alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Binding ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentHue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentHue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentHue:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentHue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentSaturation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentSaturation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentSaturation:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentSaturation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeRemainingTime
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeRemainingTime"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeRemainingTime:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl RemainingTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentX:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCurrentY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCurrentY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentY:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CurrentY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeDriftCompensation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeDriftCompensation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeDriftCompensation:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl DriftCompensation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCompensationText
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCompensationText"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCompensationText:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CompensationText Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTemperature
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorTemperature"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTemperature:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTemperature Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorControlOptions
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorControlOptions"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorControlOptions:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorControlOptions Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorControlOptions
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorControlOptions"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorControlOptions:value
                             completionHandler:^(NSError * err, NSDictionary * values) {
                                 NSLog(@"ColorControl ColorControlOptions Error: %@", err);
                                 XCTAssertEqual(err.code, 0);
                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeNumberOfPrimaries
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeNumberOfPrimaries"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNumberOfPrimaries:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl NumberOfPrimaries Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary1X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary1Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary1Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary1Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary1Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary1Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary2X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary2Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary2Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary2Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary2Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary2Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary3X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary3Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary3Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary3Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary3Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary3Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary4X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary4Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary4Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary4Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary4Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary4Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary5X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary5Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary5Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary5Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary5Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary5Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6X
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary6X"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6X:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6X Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6Y
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary6Y"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6Y:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6Y Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributePrimary6Intensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributePrimary6Intensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePrimary6Intensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl Primary6Intensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeWhitePointX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeWhitePointX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWhitePointX:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl WhitePointX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeWhitePointX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeWhitePointX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeWhitePointX:value
                     completionHandler:^(NSError * err, NSDictionary * values) {
                         NSLog(@"ColorControl WhitePointX Error: %@", err);
                         XCTAssertEqual(err.code, 0);
                         [expectation fulfill];
                     }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeWhitePointY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeWhitePointY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeWhitePointY:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl WhitePointY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeWhitePointY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeWhitePointY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeWhitePointY:value
                     completionHandler:^(NSError * err, NSDictionary * values) {
                         NSLog(@"ColorControl WhitePointY Error: %@", err);
                         XCTAssertEqual(err.code, 0);
                         [expectation fulfill];
                     }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointRX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRX:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointRX:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointRX Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointRY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRY:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointRY:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointRY Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointRIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointRIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointRIntensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointRIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointRIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointRIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointRIntensity:value
                              completionHandler:^(NSError * err, NSDictionary * values) {
                                  NSLog(@"ColorControl ColorPointRIntensity Error: %@", err);
                                  XCTAssertEqual(err.code, 0);
                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointGX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGX:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointGX:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointGX Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointGY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGY:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointGY:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointGY Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointGIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointGIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointGIntensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointGIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointGIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointGIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointGIntensity:value
                              completionHandler:^(NSError * err, NSDictionary * values) {
                                  NSLog(@"ColorControl ColorPointGIntensity Error: %@", err);
                                  XCTAssertEqual(err.code, 0);
                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointBX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBX:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBX Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBX
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBX"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointBX:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointBX Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointBY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBY:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBY Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBY
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBY"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeColorPointBY:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"ColorControl ColorPointBY Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeColorPointBIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorPointBIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorPointBIntensity:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorPointBIntensity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeColorPointBIntensity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeColorPointBIntensity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeColorPointBIntensity:value
                              completionHandler:^(NSError * err, NSDictionary * values) {
                                  NSLog(@"ColorControl ColorPointBIntensity Error: %@", err);
                                  XCTAssertEqual(err.code, 0);
                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeEnhancedCurrentHue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeEnhancedCurrentHue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnhancedCurrentHue:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl EnhancedCurrentHue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeEnhancedColorMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeEnhancedColorMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEnhancedColorMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl EnhancedColorMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopActive
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorLoopActive"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopActive:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopActive Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopDirection
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorLoopDirection"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopDirection:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopDirection Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorLoopTime
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorLoopTime"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorLoopTime:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorLoopTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorCapabilities
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorCapabilities"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorCapabilities:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorCapabilities Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTempPhysicalMin
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorTempPhysicalMin"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTempPhysicalMin:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTempPhysicalMin Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeColorTempPhysicalMax
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeColorTempPhysicalMax"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeColorTempPhysicalMax:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ColorTempPhysicalMax Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeCoupleColorTempToLevelMinMireds
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeCoupleColorTempToLevelMinMireds"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCoupleColorTempToLevelMinMireds:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl CoupleColorTempToLevelMinMireds Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlReadAttributeStartUpColorTemperatureMireds
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeStartUpColorTemperatureMireds"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeStartUpColorTemperatureMireds:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl StartUpColorTemperatureMireds Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterColorControlWriteAttributeStartUpColorTemperatureMireds
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlWriteAttributeStartUpColorTemperatureMireds"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeStartUpColorTemperatureMireds:value
                                       completionHandler:^(NSError * err, NSDictionary * values) {
                                           NSLog(@"ColorControl StartUpColorTemperatureMireds Error: %@", err);
                                           XCTAssertEqual(err.code, 0);
                                           [expectation fulfill];
                                       }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterColorControlReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ColorControlReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPColorControl * cluster = [[CHIPColorControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"ColorControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeDeviceList
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeDeviceList"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeDeviceList:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor DeviceList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeServerList
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeServerList"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeServerList:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ServerList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeClientList
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeClientList"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClientList:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ClientList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributePartsList
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributePartsList"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributePartsList:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor PartsList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDescriptorReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DescriptorReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDescriptor * cluster = [[CHIPDescriptor alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Descriptor ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeLockState
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeLockState"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLockState:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock LockState Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeLockType
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeLockType"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLockType:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock LockType Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeActuatorEnabled
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeActuatorEnabled"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeActuatorEnabled:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ActuatorEnabled Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterDoorLockReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"DoorLockReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPDoorLock * cluster = [[CHIPDoorLock alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"DoorLock ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningReadAttributeFabricId
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningReadAttributeFabricId"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeFabricId:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning FabricId Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningReadAttributeBreadcrumb
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningReadAttributeBreadcrumb"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeBreadcrumb:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning Breadcrumb Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGeneralCommissioningWriteAttributeBreadcrumb
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningWriteAttributeBreadcrumb"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    uint64_t value = 0x0000000000000000;
    [cluster writeAttributeBreadcrumb:value
                    completionHandler:^(NSError * err, NSDictionary * values) {
                        NSLog(@"GeneralCommissioning Breadcrumb Error: %@", err);
                        XCTAssertEqual(err.code, 0);
                        [expectation fulfill];
                    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterGeneralCommissioningReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GeneralCommissioningReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGeneralCommissioning * cluster = [[CHIPGeneralCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"GeneralCommissioning ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeGroups
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupKeyManagementReadAttributeGroups"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeGroups:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement Groups Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeGroupKeys
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupKeyManagementReadAttributeGroupKeys"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeGroupKeys:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement GroupKeys Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupKeyManagementReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupKeyManagementReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroupKeyManagement * cluster = [[CHIPGroupKeyManagement alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"GroupKeyManagement ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupsReadAttributeNameSupport
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupsReadAttributeNameSupport"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroups * cluster = [[CHIPGroups alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNameSupport:^(NSError * err, NSDictionary * values) {
        NSLog(@"Groups NameSupport Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterGroupsReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"GroupsReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPGroups * cluster = [[CHIPGroups alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Groups ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterIdentifyReadAttributeIdentifyTime
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyReadAttributeIdentifyTime"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeIdentifyTime:^(NSError * err, NSDictionary * values) {
        NSLog(@"Identify IdentifyTime Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterIdentifyWriteAttributeIdentifyTime
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyWriteAttributeIdentifyTime"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint16_t value = 0x0000;
    [cluster writeAttributeIdentifyTime:value
                      completionHandler:^(NSError * err, NSDictionary * values) {
                          NSLog(@"Identify IdentifyTime Error: %@", err);
                          XCTAssertEqual(err.code, 0);
                          [expectation fulfill];
                      }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterIdentifyReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"IdentifyReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPIdentify * cluster = [[CHIPIdentify alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Identify ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLevelControlReadAttributeCurrentLevel
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LevelControlReadAttributeCurrentLevel"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLevelControl * cluster = [[CHIPLevelControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentLevel:^(NSError * err, NSDictionary * values) {
        NSLog(@"LevelControl CurrentLevel Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLevelControlReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LevelControlReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLevelControl * cluster = [[CHIPLevelControl alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"LevelControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterLowPowerReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"LowPowerReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPLowPower * cluster = [[CHIPLowPower alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"LowPower ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterNetworkCommissioningReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"NetworkCommissioningReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPNetworkCommissioning * cluster = [[CHIPNetworkCommissioning alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"NetworkCommissioning ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOnOffReadAttributeOnOff
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffReadAttributeOnOff"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOnOff:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff OnOff Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOnOffReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OnOffReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOnOff * cluster = [[CHIPOnOff alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"OnOff ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOperationalCredentialsReadAttributeFabricsList
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OperationalCredentialsReadAttributeFabricsList"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOperationalCredentials * cluster = [[CHIPOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeFabricsList:^(NSError * err, NSDictionary * values) {
        NSLog(@"OperationalCredentials FabricsList Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterOperationalCredentialsReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"OperationalCredentialsReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPOperationalCredentials * cluster = [[CHIPOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"OperationalCredentials ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxPressure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxPressure"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxPressure:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxPressure Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxSpeed
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxSpeed"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxSpeed:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxSpeed Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeMaxFlow
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeMaxFlow"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxFlow:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl MaxFlow Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeEffectiveOperationMode
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeEffectiveOperationMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEffectiveOperationMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl EffectiveOperationMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeEffectiveControlMode
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeEffectiveControlMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeEffectiveControlMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl EffectiveControlMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeCapacity
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeCapacity"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCapacity:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl Capacity Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlReadAttributeOperationMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeOperationMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOperationMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl OperationMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterPumpConfigurationAndControlWriteAttributeOperationMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlWriteAttributeOperationMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeOperationMode:value
                       completionHandler:^(NSError * err, NSDictionary * values) {
                           NSLog(@"PumpConfigurationAndControl OperationMode Error: %@", err);
                           XCTAssertEqual(err.code, 0);
                           [expectation fulfill];
                       }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterPumpConfigurationAndControlReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"PumpConfigurationAndControlReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPPumpConfigurationAndControl * cluster = [[CHIPPumpConfigurationAndControl alloc] initWithDevice:device
                                                                                               endpoint:1
                                                                                                  queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"PumpConfigurationAndControl ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeSceneCount
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeSceneCount"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSceneCount:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes SceneCount Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeCurrentScene
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeCurrentScene"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentScene:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes CurrentScene Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeCurrentGroup
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeCurrentGroup"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentGroup:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes CurrentGroup Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeSceneValid
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeSceneValid"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSceneValid:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes SceneValid Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeNameSupport
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeNameSupport"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNameSupport:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes NameSupport Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterScenesReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ScenesReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPScenes * cluster = [[CHIPScenes alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Scenes ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeNumberOfPositions
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeNumberOfPositions"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeNumberOfPositions:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch NumberOfPositions Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeCurrentPosition
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeCurrentPosition"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeCurrentPosition:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch CurrentPosition Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterSwitchReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"SwitchReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPSwitch * cluster = [[CHIPSwitch alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Switch ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMeasuredValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMeasuredValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMeasuredValue:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMinMeasuredValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMinMeasuredValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMinMeasuredValue:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MinMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeMaxMeasuredValue
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TemperatureMeasurementReadAttributeMaxMeasuredValue"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeMaxMeasuredValue:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement MaxMeasuredValue Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterTemperatureMeasurementReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"TemperatureMeasurementReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPTemperatureMeasurement * cluster = [[CHIPTemperatureMeasurement alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"TemperatureMeasurement ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatReadAttributeLocalTemperature
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeLocalTemperature"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeLocalTemperature:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat LocalTemperature Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatReadAttributeOccupiedCoolingSetpoint
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeOccupiedCoolingSetpoint"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOccupiedCoolingSetpoint:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat OccupiedCoolingSetpoint Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeOccupiedCoolingSetpoint
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeOccupiedCoolingSetpoint"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int16_t value = 0;
    [cluster writeAttributeOccupiedCoolingSetpoint:value
                                 completionHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"Thermostat OccupiedCoolingSetpoint Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeOccupiedHeatingSetpoint
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeOccupiedHeatingSetpoint"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeOccupiedHeatingSetpoint:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat OccupiedHeatingSetpoint Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeOccupiedHeatingSetpoint
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeOccupiedHeatingSetpoint"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    int16_t value = 0;
    [cluster writeAttributeOccupiedHeatingSetpoint:value
                                 completionHandler:^(NSError * err, NSDictionary * values) {
                                     NSLog(@"Thermostat OccupiedHeatingSetpoint Error: %@", err);
                                     XCTAssertEqual(err.code, 0);
                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeControlSequenceOfOperation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeControlSequenceOfOperation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeControlSequenceOfOperation:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat ControlSequenceOfOperation Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeControlSequenceOfOperation
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeControlSequenceOfOperation"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeControlSequenceOfOperation:value
                                    completionHandler:^(NSError * err, NSDictionary * values) {
                                        NSLog(@"Thermostat ControlSequenceOfOperation Error: %@", err);
                                        XCTAssertEqual(err.code, 0);
                                        [expectation fulfill];
                                    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeSystemMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeSystemMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeSystemMode:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat SystemMode Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)testSendClusterThermostatWriteAttributeSystemMode
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatWriteAttributeSystemMode"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    uint8_t value = 0x00;
    [cluster writeAttributeSystemMode:value
                    completionHandler:^(NSError * err, NSDictionary * values) {
                        NSLog(@"Thermostat SystemMode Error: %@", err);
                        XCTAssertEqual(err.code, 0);
                        [expectation fulfill];
                    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
- (void)testSendClusterThermostatReadAttributeClusterRevision
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"ThermostatReadAttributeClusterRevision"];

    CHIPDevice * device = GetPairedDevice(kDeviceId);
    dispatch_queue_t queue = dispatch_get_main_queue();
    CHIPThermostat * cluster = [[CHIPThermostat alloc] initWithDevice:device endpoint:1 queue:queue];
    XCTAssertNotNil(cluster);

    [cluster readAttributeClusterRevision:^(NSError * err, NSDictionary * values) {
        NSLog(@"Thermostat ClusterRevision Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

@end
