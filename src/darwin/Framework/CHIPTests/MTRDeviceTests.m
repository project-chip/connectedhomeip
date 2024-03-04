//
//  MTRDeviceTests.m
//  MTRDeviceTests
/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import <Matter/MTRBaseClusters.h>
#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRClusterStateCacheContainer.h>
#import <Matter/Matter.h>

#import "MTRCommandPayloadExtensions_Internal.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRErrorTestUtils.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestResetCommissioneeHelper.h"
#import "MTRTestStorage.h"

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

// Fixture: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static MTRBaseDevice * mConnectedDevice;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

static void WaitForCommissionee(XCTestExpectation * expectation)
{
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    // For now keep the async dispatch, but could we just
    // synchronously fulfill the expectation here?
    dispatch_async(dispatch_get_main_queue(), ^{
        [expectation fulfill];
        mConnectedDevice = [MTRBaseDevice deviceWithNodeID:@(kDeviceId) controller:controller];
    });
}

static MTRBaseDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(mConnectedDevice);
    return mConnectedDevice;
}

@interface MTRDeviceTestDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTRDeviceTestDeviceControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);

    __auto_type * params = [[MTRCommissioningParameters alloc] init];
    params.countryCode = @("au");

    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId) commissioningParams:params error:&commissionError];
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

@interface MTRDeviceTests : XCTestCase
@end

@implementation MTRDeviceTests

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
static BOOL slocalTestStorageEnabledBeforeUnitTest;
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

+ (void)setUp
{
    XCTestExpectation * pairingExpectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete"];

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
    slocalTestStorageEnabledBeforeUnitTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = YES;
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startControllerFactory:factoryParams error:nil];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:nil];
    XCTAssertNotNil(controller);

    sController = controller;

    MTRDeviceTestDeviceControllerDelegate * deviceControllerDelegate =
        [[MTRDeviceTestDeviceControllerDelegate alloc] initWithExpectation:pairingExpectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    XCTAssertTrue([controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error]);
    XCTAssertNil(error);
    XCTAssertEqual([XCTWaiter waitForExpectations:@[ pairingExpectation ] timeout:kPairingTimeoutInSeconds], XCTWaiterResultCompleted);

    XCTestExpectation * expectation = [[XCTestExpectation alloc] initWithDescription:@"Wait for the commissioned device to be retrieved"];
    WaitForCommissionee(expectation);
    XCTAssertEqual([XCTWaiter waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds], XCTWaiterResultCompleted);
}

+ (void)tearDown
{
    ResetCommissionee(GetConnectedDevice(), dispatch_get_main_queue(), nil, kTimeoutInSeconds);

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
    // Restore testing setting to previous state, and remove all persisted attributes
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = slocalTestStorageEnabledBeforeUnitTest;
    [sController.controllerDataStore clearAllStoredAttributes];
    NSArray * storedAttributesAfterClear = [sController.controllerDataStore getStoredAttributesForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedAttributesAfterClear.count, 0);
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];

    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)test001_ReadAttribute
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:nil
                               clusterID:@29
                             attributeID:@0
                                  params:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 29);
                                          XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
                                          XCTAssertNotNil(result[@"data"]);
                                          XCTAssertNil(result[@"error"]);
                                          XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                          XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Array"]);

                                          __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:result
                                                                                                             error:nil];
                                          XCTAssertNotNil(report);
                                          XCTAssertEqualObjects(report.path.cluster, @(29));
                                          XCTAssertEqualObjects(report.path.attribute, @(0));
                                          XCTAssertNil(report.error);
                                          XCTAssertNotNil(report.value);
                                          XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);

                                          for (id entry in report.value) {
                                              XCTAssertTrue([entry isKindOfClass:[MTRDescriptorClusterDeviceTypeStruct class]]);
                                          }
                                      }
                                      XCTAssertTrue([resultArray count] > 0);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test002_WriteAttribute
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"write LevelControl Brightness attribute"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointID:@1
                               clusterID:@8
                             attributeID:@17
                                   value:writeValue
                       timedWriteTimeout:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertNil(error);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                          XCTAssertEqual([path.attribute unsignedIntegerValue], 17);
                                          XCTAssertNil(result[@"error"]);
                                      }
                                      XCTAssertEqual([resultArray count], 1);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test003_InvokeCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[
            @{ @"contextTag" : @0, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @0 } },
            @{ @"contextTag" : @1, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @10 } }
        ]
    };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@8
                              commandID:@4
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 4);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test004_InvokeTimedCommand
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke Off command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[],
    };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:@10000
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: Off values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

static void (^globalReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

- (void)test005_Subscribe
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];

        __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:result error:nil];
        XCTAssertNotNil(report);
        XCTAssertEqualObjects(report.path.endpoint, @(1));
        XCTAssertEqualObjects(report.path.cluster, @(6));
        XCTAssertEqualObjects(report.path.attribute, @(0));
        XCTAssertNil(report.error);
        XCTAssertNotNil(report.value);
        XCTAssertEqualObjects(report.value, @(YES));

        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        XCTAssertTrue([result[@"data"][@"value"] boolValue]);
        [reportExpectation fulfill];
        globalReportHandler = nil;
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : [NSArray array] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Set up expectation for 2nd report
    reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];

        __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:result error:nil];
        XCTAssertNotNil(report);
        XCTAssertEqualObjects(report.path.endpoint, @(1));
        XCTAssertEqualObjects(report.path.cluster, @(6));
        XCTAssertEqualObjects(report.path.attribute, @(0));
        XCTAssertNil(report.error);
        XCTAssertNotNil(report.value);
        XCTAssertEqualObjects(report.value, @(NO));

        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        XCTAssertFalse([result[@"data"][@"value"] boolValue]);
        [reportExpectation fulfill];
        globalReportHandler = nil;
    };

    // Send command to trigger attribute change
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    expectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [expectation fulfill];
                                   }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test006_ReadAttributeFailure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:@0
                               clusterID:@10000
                             attributeID:@0
                                  params:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);
                                  XCTAssertNotNil(values);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      XCTAssertEqual([resultArray count], 1);
                                      NSDictionary * result = resultArray[0];

                                      __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:result error:nil];
                                      XCTAssertNotNil(report);
                                      XCTAssertEqualObjects(report.path.endpoint, @(0));
                                      XCTAssertEqualObjects(report.path.cluster, @(10000));
                                      XCTAssertEqualObjects(report.path.attribute, @(0));
                                      XCTAssertNotNil(report.error);
                                      XCTAssertNil(report.value);
                                      XCTAssertTrue([MTRErrorTestUtils error:report.error isInteractionModelError:MTRInteractionErrorCodeUnsupportedCluster]);

                                      MTRAttributePath * path = result[@"attributePath"];
                                      XCTAssertEqual(path.endpoint.unsignedIntegerValue, 0);
                                      XCTAssertEqual(path.cluster.unsignedIntegerValue, 10000);
                                      XCTAssertEqual(path.attribute.unsignedIntegerValue, 0);
                                      XCTAssertNotNil(result[@"error"]);
                                      XCTAssertNil(result[@"data"]);
                                      XCTAssertTrue([result[@"error"] isKindOfClass:[NSError class]]);
                                      XCTAssertTrue([MTRErrorTestUtils error:result[@"error"] isInteractionModelError:MTRInteractionErrorCodeUnsupportedCluster]);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test007_WriteAttributeFailure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"write failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device
        writeAttributeWithEndpointID:@1
                           clusterID:@8
                         attributeID:@10000
                               value:writeValue
                   timedWriteTimeout:nil
                               queue:queue
                          completion:^(id _Nullable values, NSError * _Nullable error) {
                              NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                              XCTAssertNil(values);
                              XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeUnsupportedAttribute]);

                              [expectation fulfill];
                          }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test008_InvokeCommandFailure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[
            @{ @"contextTag" : @0, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @0 } },
            @{ @"contextTag" : @1, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @10 } }
        ]
    };
    [device
        invokeCommandWithEndpointID:@1
                          clusterID:@8
                          commandID:@(0xff)
                      commandFields:fields
                 timedInvokeTimeout:nil
                              queue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                             XCTAssertNil(values);
                             XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeUnsupportedCommand]);

                             [expectation fulfill];
                         }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test009_SubscribeFailure
{
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Set up expectation for report
    XCTestExpectation * errorReportExpectation = [self expectationWithDescription:@"receive subscription error"];
    reportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        // Because our subscription has no existent paths, it gets an
        // InvalidAction response.
        XCTAssertNil(value);
        XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeInvalidAction]);
        [errorReportExpectation fulfill];
    };

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    [device subscribeToAttributesWithEndpointID:@10000
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (reportHandler) {
                __auto_type callback = reportHandler;
                reportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            XCTFail("Should not get subscriptionEstablished in the error case");
        }];

    // Wait till establishment and error report
    [self waitForExpectations:@[ errorReportExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test010_ReadAllAttribute
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:@1
                               clusterID:@29
                             attributeID:nil
                                  params:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 29);
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertNotNil(result[@"data"]);
                                          XCTAssertNil(result[@"error"]);
                                          XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                      }
                                      XCTAssertTrue([resultArray count] > 0);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test011_ReadCachedAttribute
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    __auto_type clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"Subscription complete"];

    NSLog(@"Subscribing...");
    // reportHandler returns TRUE if it got the things it was looking for or if there's an error.
    __block BOOL (^reportHandler)(NSArray * _Nullable value, NSError * _Nullable error);
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(60)];
    params.resubscribeAutomatically = NO;
    [device subscribeWithQueue:queue
        params:params
        clusterStateCacheContainer:clusterStateCacheContainer
        attributeReportHandler:^(NSArray * value) {
            NSLog(@"Received report: %@", value);
            if (reportHandler) {
                __auto_type handler = reportHandler;
                reportHandler = nil;
                BOOL done = handler(value, nil);
                if (done == NO) {
                    // Keep waiting.
                    reportHandler = handler;
                }
            }
        }
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);
            if (reportHandler) {
                __auto_type handler = reportHandler;
                reportHandler = nil;
                handler(nil, error);
            }
        }
        subscriptionEstablished:^() {
            [subscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ subscribeExpectation ] timeout:60];

    // Invoke command to set the attribute to a known state
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"Command invoked"];
    MTRBaseClusterOnOff * cluster = [[MTRBaseClusterOnOff alloc] initWithDevice:device endpointID:@(1) queue:queue];
    XCTAssertNotNil(cluster);

    NSLog(@"Invoking command...");
    [cluster onWithCompletion:^(NSError * _Nullable err) {
        NSLog(@"Invoked command with error: %@", err);
        XCTAssertNil(err);
        [commandExpectation fulfill];
    }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait till reports arrive from accessory.
    NSLog(@"Waiting for reports from accessory...");
    sleep(5);

    // Read cache
    NSLog(@"Reading from cache...");
    XCTestExpectation * cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [MTRBaseClusterOnOff readAttributeOnOffWithClusterStateCache:clusterStateCacheContainer
                                                        endpoint:@1
                                                           queue:queue
                                                      completion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
                                                          NSLog(@"Read attribute cache value: %@, error: %@", value, err);
                                                          XCTAssertNil(err);
                                                          XCTAssertTrue([value isEqualToNumber:[NSNumber numberWithBool:YES]]);
                                                          [cacheExpectation fulfill];
                                                      }];
    [self waitForExpectations:[NSArray arrayWithObject:cacheExpectation] timeout:kTimeoutInSeconds];

    // Add another subscriber of the attribute to verify that attribute cache still works when there are other subscribers.
    NSLog(@"New subscription...");
    XCTestExpectation * newSubscriptionEstablished = [self expectationWithDescription:@"New subscription established"];
    MTRSubscribeParams * newParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(60)];
    newParams.replaceExistingSubscriptions = NO;
    newParams.resubscribeAutomatically = NO;
    [cluster subscribeAttributeOnOffWithParams:newParams
        subscriptionEstablished:^{
            NSLog(@"New subscription was established");
            [newSubscriptionEstablished fulfill];
        }
        reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"New subscriber received a report: %@, error: %@", value, error);
        }];
    [self waitForExpectations:[NSArray arrayWithObject:newSubscriptionEstablished] timeout:kTimeoutInSeconds];

    __auto_type reportExpectation = [self expectationWithDescription:@"Report handler called"];
    reportHandler = ^(NSArray * _Nullable value, NSError * _Nullable error) {
        if (error != nil) {
            return YES;
        }
        NSLog(@"Report received: %@, error: %@", value, error);
        for (MTRAttributeReport * report in value) {
            if ([report.path.endpoint isEqualToNumber:@1] && [report.path.cluster isEqualToNumber:@6] &&
                [report.path.attribute isEqualToNumber:@0]) {
                NSLog(@"Report value for OnOff: %@", report.value);
                XCTAssertNotNil(report.value);
                XCTAssertTrue([report.value isKindOfClass:[NSNumber class]]);
                XCTAssertEqual([report.value boolValue], NO);
                [reportExpectation fulfill];
                return YES;
            }
        }

        return NO;
    };

    NSLog(@"Invoking another command...");
    commandExpectation = [self expectationWithDescription:@"Command invoked"];
    [cluster offWithCompletion:^(NSError * _Nullable err) {
        NSLog(@"Invoked command with error: %@", err);
        XCTAssertNil(err);
        [commandExpectation fulfill];
    }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait till reports arrive from accessory.
    NSLog(@"Waiting for reports from accessory...");
    [self waitForExpectations:@[ reportExpectation ] timeout:kTimeoutInSeconds];

    NSLog(@"Disconnect accessory to test cache...");
    __auto_type idleExpectation = [self expectationWithDescription:@"Must not break out of idle"];
    idleExpectation.inverted = YES;
    [self waitForExpectations:[NSArray arrayWithObject:idleExpectation] timeout:10];

    // Read cache
    NSLog(@"Reading from cache...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [MTRBaseClusterOnOff readAttributeOnOffWithClusterStateCache:clusterStateCacheContainer
                                                        endpoint:@1
                                                           queue:queue
                                                      completion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
                                                          NSLog(@"Read attribute cache value: %@, error: %@", value, err);
                                                          XCTAssertNil(err);
                                                          XCTAssertTrue([value isEqualToNumber:[NSNumber numberWithBool:NO]]);
                                                          [cacheExpectation fulfill];
                                                      }];
    [self waitForExpectations:[NSArray arrayWithObject:cacheExpectation] timeout:kTimeoutInSeconds];

    // Read from cache using generic path
    NSLog(@"Reading from cache using generic path...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [clusterStateCacheContainer
        readAttributesWithEndpointID:@1
                           clusterID:@6
                         attributeID:@0
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Read attribute cache value: %@, error %@", values, error);
                              XCTAssertNil(error);
                              XCTAssertEqual([values count], 1);

                              __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:values[0] error:nil];
                              XCTAssertNotNil(report);
                              XCTAssertEqualObjects(report.path.endpoint, @(1));
                              XCTAssertEqualObjects(report.path.cluster, @(6));
                              XCTAssertEqualObjects(report.path.attribute, @(0));
                              XCTAssertNil(report.error);
                              XCTAssertNotNil(report.value);
                              XCTAssertEqualObjects(report.value, @(NO));

                              MTRAttributePath * path = values[0][@"attributePath"];
                              XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                              XCTAssertEqual([path.cluster unsignedLongValue], 6);
                              XCTAssertEqual([path.attribute unsignedLongValue], 0);
                              XCTAssertNotNil(values[0][@"data"]);
                              XCTAssertNil(values[0][@"error"]);
                              XCTAssertTrue([values[0][@"data"][@"type"] isEqualToString:@"Boolean"]);
                              XCTAssertEqual([values[0][@"data"][@"value"] boolValue], NO);
                              [cacheExpectation fulfill];
                          }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard endpoint...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [clusterStateCacheContainer
        readAttributesWithEndpointID:nil
                           clusterID:@6
                         attributeID:@0
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Read attribute cache value: %@, error %@", values, error);
                              XCTAssertNil(error);
                              XCTAssertTrue([values count] > 0);
                              for (NSDictionary<NSString *, id> * value in values) {
                                  __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:value error:nil];
                                  XCTAssertNotNil(report);
                                  XCTAssertEqualObjects(report.path.cluster, @(6));
                                  XCTAssertEqualObjects(report.path.attribute, @(0));
                                  XCTAssertNil(report.error);
                                  XCTAssertNotNil(report.value);
                                  XCTAssertTrue([report.value isKindOfClass:[NSNumber class]]);

                                  MTRAttributePath * path = value[@"attributePath"];
                                  XCTAssertEqual([path.cluster unsignedLongValue], 6);
                                  XCTAssertEqual([path.attribute unsignedLongValue], 0);
                                  XCTAssertNil(value[@"error"]);
                                  XCTAssertNotNil(value[@"data"]);
                              }
                              [cacheExpectation fulfill];
                          }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard cluster ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [clusterStateCacheContainer
        readAttributesWithEndpointID:@1
                           clusterID:nil
                         attributeID:@0
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Read attribute cache value: %@, error %@", values, error);
                              XCTAssertNil(error);
                              XCTAssertTrue([values count] > 0);
                              for (NSDictionary<NSString *, id> * value in values) {
                                  MTRAttributePath * path = value[@"attributePath"];
                                  XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                                  XCTAssertEqual([path.attribute unsignedLongValue], 0);
                                  XCTAssertNil(value[@"error"]);
                                  XCTAssertNotNil(value[@"data"]);
                              }
                              [cacheExpectation fulfill];
                          }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard attribute ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [clusterStateCacheContainer
        readAttributesWithEndpointID:@1
                           clusterID:@6
                         attributeID:nil
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Read attribute cache value: %@, error %@", values, error);
                              XCTAssertNil(error);
                              XCTAssertTrue([values count] > 0);
                              for (NSDictionary<NSString *, id> * value in values) {
                                  MTRAttributePath * path = value[@"attributePath"];
                                  XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                                  XCTAssertEqual([path.cluster unsignedLongValue], 6);
                                  XCTAssertNil(value[@"error"]);
                                  XCTAssertNotNil(value[@"data"]);
                              }
                              [cacheExpectation fulfill];
                          }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard endpoint ID and cluster ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [clusterStateCacheContainer
        readAttributesWithEndpointID:nil
                           clusterID:nil
                         attributeID:@0
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Read attribute cache value: %@, error %@", values, error);
                              XCTAssertNotNil(error);
                              [cacheExpectation fulfill];
                          }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];
}

#ifdef DEBUG
// Test an error to subscription
- (void)test012_SubscriptionError
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * deregisterExpectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [deregisterExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ deregisterExpectation ] timeout:kTimeoutInSeconds];

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([value isKindOfClass:[NSArray class]]);
        NSDictionary * result = value[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"data"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : [NSArray array] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Trigger reader failure
    XCTestExpectation * failureExpectation = [self expectationWithDescription:@"failed on purpose"];
    [device failSubscribers:queue
                 completion:^{
                     [failureExpectation fulfill];
                 }];
    [self waitForExpectations:@[ failureExpectation ] timeout:kTimeoutInSeconds];

    deregisterExpectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [deregisterExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ deregisterExpectation ] timeout:kTimeoutInSeconds];
}
#endif

- (void)test013_ReuseChipClusterObject
{
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    MTRBaseDevice * device = [MTRBaseDevice deviceWithNodeID:@(kDeviceId) controller:controller];

    XCTestExpectation * expectation = [self expectationWithDescription:@"ReuseMTRClusterObjectFirstCall"];

    dispatch_queue_t queue = dispatch_get_main_queue();
    MTRBaseClusterUnitTesting * cluster = [[MTRBaseClusterUnitTesting alloc] initWithDevice:device endpointID:@(1) queue:queue];
    XCTAssertNotNil(cluster);

    [cluster testWithCompletion:^(NSError * err) {
        NSLog(@"ReuseMTRClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    expectation = [self expectationWithDescription:@"ReuseMTRClusterObjectSecondCall"];

    // Reuse the MTRCluster Object for multiple times.

    [cluster testWithCompletion:^(NSError * err) {
        NSLog(@"ReuseMTRClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test014_InvokeCommandWithDifferentIdResponse
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke Off command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[],
    };
    // KeySetReadAllIndices in the Group Key Management has id 4 and a data response with id 5
    [device
        invokeCommandWithEndpointID:@0
                          clusterID:@(0x003F)
                          commandID:@4
                      commandFields:fields
                 timedInvokeTimeout:nil
                              queue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"invoke command: KeySetReadAllIndices values: %@, error: %@", values, error);

                             XCTAssertNil(error);

                             {
                                 XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                 NSArray * resultArray = values;
                                 for (NSDictionary * result in resultArray) {
                                     MTRCommandPath * path = result[MTRCommandPathKey];
                                     XCTAssertEqual([path.endpoint unsignedIntegerValue], 0);
                                     XCTAssertEqual([path.cluster unsignedIntegerValue], 0x003F);
                                     XCTAssertEqual([path.command unsignedIntegerValue], 5);
                                     // We expect a KeySetReadAllIndicesResponse struct,
                                     // which has context tag 0 pointing to a list with one
                                     // item: 0 (the IPK's keyset id).
                                     NSDictionary * expectedResult = @{
                                         MTRTypeKey : MTRStructureValueType,
                                         MTRValueKey : @[ @{
                                             MTRContextTagKey : @0,
                                             MTRDataKey : @ {
                                                 MTRTypeKey : MTRArrayValueType,
                                                 MTRValueKey : @[ @{
                                                     MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @0 }
                                                 } ]
                                             }
                                         } ],
                                     };
                                     XCTAssertEqualObjects(result[MTRDataKey], expectedResult);
                                     XCTAssertNil(result[MTRErrorKey]);

                                     // Now check our strong-typed parsing bits.
                                     NSError * parseError;
                                     __auto_type * response =
                                         [[MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams alloc]
                                             initWithResponseValue:result
                                                             error:&parseError];
                                     XCTAssertNil(parseError);
                                     XCTAssertNotNil(response);
                                     XCTAssertEqual(response.groupKeySetIDs.count, 1);
                                     XCTAssertEqualObjects(response.groupKeySetIDs[0], @(0));
                                 }
                                 XCTAssertEqual([resultArray count], 1);
                             }

                             [expectation fulfill];
                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test015_FailedSubscribeWithQueueAcrossShutdown
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * firstSubscribeExpectation = [self expectationWithDescription:@"First subscription complete"];
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"First subscription errored out"];

    // Create first subscription.  It needs to be using subscribeWithQueue and
    // must have a clusterStateCacheContainer to exercise the onDone case.
    NSLog(@"Subscribing...");
    __auto_type clusterStateCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    __auto_type * params = [[MTRSubscribeParams alloc] init];
    params.resubscribeAutomatically = NO;
    params.replaceExistingSubscriptions = NO; // Not strictly needed, but checking that doing this does not
    // affect this subscription erroring out correctly.
    [device subscribeWithQueue:queue
        minInterval:1
        maxInterval:2
        params:params
        cacheContainer:clusterStateCacheContainer
        attributeReportHandler:nil
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);

            // Restart the controller here, to exercise our various event queue bits.
            [controller shutdown];

            // Wait a bit before restart, to allow whatever async things are going on after this is called to try to happen.
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), queue, ^{
                __auto_type * factory = [MTRControllerFactory sharedInstance];
                XCTAssertNotNil(factory);

                // Needs to match what initStack does.
                __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:sTestKeys
                                                                                               fabricId:1
                                                                                                    ipk:sTestKeys.ipk];
                __auto_type * newController = [factory startControllerOnExistingFabric:params];
                XCTAssertNotNil(newController);

                sController = newController;

                WaitForCommissionee(errorExpectation);
            });
        }
        subscriptionEstablished:^() {
            [firstSubscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ firstSubscribeExpectation ] timeout:60];

    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    params.replaceExistingSubscriptions = YES;
    [device subscribeAttributeWithEndpointId:@10000
                                   clusterId:@6
                                 attributeId:@0
                                 minInterval:@(1)
                                 maxInterval:@(2)
                                      params:params
                                 clientQueue:queue
                               reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                               }
                     subscriptionEstablished:^() {}];
    [self waitForExpectations:@[ errorExpectation ] timeout:60];
}

- (void)test016_FailedSubscribeWithCacheReadDuringFailure
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * firstSubscribeExpectation = [self expectationWithDescription:@"First subscription complete"];
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"First subscription errored out"];

    // Create first subscription.  It needs to be using subscribeWithQueue and
    // must have a clusterStateCacheContainer to exercise the onDone case.
    NSLog(@"Subscribing...");
    __auto_type clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(2)];
    params.resubscribeAutomatically = NO;
    [device subscribeWithQueue:queue
        params:params
        clusterStateCacheContainer:clusterStateCacheContainer
        attributeReportHandler:nil
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);

            [MTRBaseClusterOnOff readAttributeOnOffWithClusterStateCache:clusterStateCacheContainer
                                                                endpoint:@1
                                                                   queue:queue
                                                              completion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                                  [errorExpectation fulfill];
                                                              }];
        }
        subscriptionEstablished:^() {
            [firstSubscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ firstSubscribeExpectation ] timeout:60];

    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    params.replaceExistingSubscriptions = YES;
    [device subscribeAttributeWithEndpointId:@10000
                                   clusterId:@6
                                 attributeId:@0
                                 minInterval:@(1)
                                 maxInterval:@(2)
                                      params:params
                                 clientQueue:queue
                               reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                               }
                     subscriptionEstablished:^() {}];
    [self waitForExpectations:@[ errorExpectation ] timeout:60];
}

- (void)test017_TestMTRDeviceBasics
{
    // Ensure the test starts with clean slate, even with MTRDeviceControllerLocalTestStorage enabled
    [sController.controllerDataStore clearAllStoredAttributes];
    NSArray * storedAttributesAfterClear = [sController.controllerDataStore getStoredAttributesForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedAttributesAfterClear.count, 0);

    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Given reachable state becomes true before underlying OnSubscriptionEstablished callback, this expectation is necessary but
    // not sufficient as a mark to the end of reports
    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onReachable = ^() {
        [subscriptionExpectation fulfill];
    };

    __block unsigned attributeReportsReceived = 0;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributeReportsReceived += data.count;
    };

    // This is dependent on current implementation that priming reports send attributes and events in that order, and also that
    // events in this test would fit in one report. So receiving events would mean all attributes and events have been received, and
    // can satisfy the test below.
    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received"];
    __block unsigned eventReportsReceived = 0;
    __block BOOL reportEnded = NO;
    __block BOOL gotOneNonPrimingEvent = NO;
    XCTestExpectation * gotNonPrimingEventExpectation = [self expectationWithDescription:@"Received event outside of priming report"];
    delegate.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * eventReport) {
        eventReportsReceived += eventReport.count;

        for (NSDictionary<NSString *, id> * eventDict in eventReport) {
            NSNumber * eventTimeTypeNumber = eventDict[MTREventTimeTypeKey];
            XCTAssertNotNil(eventTimeTypeNumber);
            MTREventTimeType eventTimeType = (MTREventTimeType) eventTimeTypeNumber.unsignedIntegerValue;
            XCTAssert((eventTimeType == MTREventTimeTypeSystemUpTime) || (eventTimeType == MTREventTimeTypeTimestampDate));
            if (eventTimeType == MTREventTimeTypeSystemUpTime) {
                XCTAssertNotNil(eventDict[MTREventSystemUpTimeKey]);
                XCTAssertNotNil(device.estimatedStartTime);
            } else if (eventTimeType == MTREventTimeTypeTimestampDate) {
                XCTAssertNotNil(eventDict[MTREventTimestampDateKey]);
            }

            if (!reportEnded) {
                NSNumber * reportIsHistorical = eventDict[MTREventIsHistoricalKey];
                XCTAssertTrue(reportIsHistorical.boolValue);
            } else {
                if (!gotOneNonPrimingEvent) {
                    NSNumber * reportIsHistorical = eventDict[MTREventIsHistoricalKey];
                    XCTAssertFalse(reportIsHistorical.boolValue);
                    gotOneNonPrimingEvent = YES;
                    [gotNonPrimingEventExpectation fulfill];
                }
            }
        }
    };
    delegate.onReportEnd = ^() {
        reportEnded = YES;
        [gotReportsExpectation fulfill];
#ifdef DEBUG
        [device unitTestInjectEventReport:@[ @{
            MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(1) eventID:@(1)],
            MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
            MTREventTimestampDateKey : [NSDate date]
        } ]];
#endif
    };

    [device setDelegate:delegate queue:queue];

    // Test batching and duplicate check
    //   - Read 13 different attributes in a row, expect that the 1st to go out by itself, the next 9 batch, and then the 3 after
    //     are correctly queued in one batch
    //   - Then read 3 duplicates and expect them to be filtered
    //   - Note that these tests can only be verified via logs
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(0) params:nil];

    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(1) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(2) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(3) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(4) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(5) params:nil];

    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(6) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeColorControlID) attributeID:@(7) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(0) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(1) params:nil];

    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(2) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(3) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(4) params:nil];

    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(4) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(4) params:nil];
    [device readAttributeWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(4) params:nil];

    [self waitForExpectations:@[ subscriptionExpectation, gotReportsExpectation, gotNonPrimingEventExpectation ] timeout:60];

    delegate.onReportEnd = nil;

    XCTAssertNotEqual(attributeReportsReceived, 0);
    XCTAssertNotEqual(eventReportsReceived, 0);

    // Before resubscribe, first test write failure and expected value effects
    NSNumber * testEndpointID = @(1);
    NSNumber * testClusterID = @(8);
    NSNumber * testAttributeID = @(10000); // choose a nonexistent attribute to cause a failure
    XCTestExpectation * expectedValueReportedExpectation = [self expectationWithDescription:@"Expected value reported"];
    XCTestExpectation * expectedValueRemovedExpectation = [self expectationWithDescription:@"Expected value removed"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);
            if ([attributePath.endpoint isEqualToNumber:testEndpointID] && [attributePath.cluster isEqualToNumber:testClusterID] &&
                [attributePath.attribute isEqualToNumber:testAttributeID]) {
                id data = attributeDict[MTRDataKey];
                if (data) {
                    [expectedValueReportedExpectation fulfill];
                } else {
                    [expectedValueRemovedExpectation fulfill];
                }
            }
        }
    };

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointID:testEndpointID
                               clusterID:testClusterID
                             attributeID:testAttributeID
                                   value:writeValue
                   expectedValueInterval:@(20000)
                       timedWriteTimeout:nil];

    // expected value interval is 20s but expect it get reverted immediately as the write fails because it's writing to a
    // nonexistent attribute
    [self waitForExpectations:@[ expectedValueReportedExpectation, expectedValueRemovedExpectation ] timeout:5 enforceOrder:YES];

    // Test if previous value is reported on a write
    uint16_t testOnTimeValue = 10;
    XCTestExpectation * onTimeWriteSuccess = [self expectationWithDescription:@"OnTime write success"];
    XCTestExpectation * onTimePreviousValue = [self expectationWithDescription:@"OnTime previous value"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * attributeReponseValue in data) {
            MTRAttributePath * path = attributeReponseValue[MTRAttributePathKey];
            if (path.cluster.unsignedIntValue == MTRClusterIDTypeOnOffID && path.attribute.unsignedLongValue == MTRAttributeIDTypeClusterOnOffAttributeOnTimeID) {
                NSDictionary * dataValue = attributeReponseValue[MTRDataKey];
                NSNumber * onTimeValue = dataValue[MTRValueKey];
                if (onTimeValue && (onTimeValue.unsignedIntValue == testOnTimeValue)) {
                    [onTimeWriteSuccess fulfill];
                }

                NSDictionary * previousDataValue = attributeReponseValue[MTRPreviousDataKey];
                NSNumber * previousOnTimeValue = previousDataValue[MTRValueKey];
                if (previousOnTimeValue) {
                    [onTimePreviousValue fulfill];
                }
            }
        }
    };
    NSDictionary * writeOnTimeValue = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(testOnTimeValue) };
    [device writeAttributeWithEndpointID:@(1)
                               clusterID:@(MTRClusterIDTypeOnOffID)
                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)
                                   value:writeOnTimeValue
                   expectedValueInterval:@(10000)
                       timedWriteTimeout:nil];

    [self waitForExpectations:@[ onTimeWriteSuccess, onTimePreviousValue ] timeout:10];

    // Test if errors are properly received
    XCTestExpectation * attributeReportErrorExpectation = [self expectationWithDescription:@"Attribute read error"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * attributeReponseValue in data) {
            if (attributeReponseValue[MTRErrorKey]) {
                [attributeReportErrorExpectation fulfill];
            }
        }
    };
    // use the nonexistent attribute and expect read error
    [device readAttributeWithEndpointID:testEndpointID clusterID:testClusterID attributeID:testAttributeID params:nil];
    [self waitForExpectations:@[ attributeReportErrorExpectation ] timeout:10];

    // Resubscription test setup
    XCTestExpectation * subscriptionDroppedExpectation = [self expectationWithDescription:@"Subscription has dropped"];

    delegate.onNotReachable = ^() {
        [subscriptionDroppedExpectation fulfill];
    };
    XCTestExpectation * resubscriptionReachableExpectation =
        [self expectationWithDescription:@"Resubscription has become reachable"];
    delegate.onReachable = ^() {
        [resubscriptionReachableExpectation fulfill];
    };
    XCTestExpectation * resubscriptionGotReportsExpectation = [self expectationWithDescription:@"Resubscription got reports"];
    delegate.onReportEnd = ^() {
        [resubscriptionGotReportsExpectation fulfill];
    };

    // reset the onAttributeDataReceived to validate the following resubscribe test
    attributeReportsReceived = 0;
    eventReportsReceived = 0;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributeReportsReceived += data.count;
    };

    delegate.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * eventReport) {
        eventReportsReceived += eventReport.count;
    };

    // Now trigger another subscription which will cause ours to drop; we should re-subscribe after that.
    MTRBaseDevice * baseDevice = GetConnectedDevice();
    __auto_type params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    params.replaceExistingSubscriptions = YES;
    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    [baseDevice subscribeAttributeWithEndpointId:@10000
                                       clusterId:@6
                                     attributeId:@0
                                     minInterval:@(1)
                                     maxInterval:@(2)
                                          params:params
                                     clientQueue:queue
                                   reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                                   }
                         subscriptionEstablished:^() {}];

    [self waitForExpectations:@[ subscriptionDroppedExpectation ] timeout:60];

    // Check that device resets start time on subscription drop
    XCTAssertNil(device.estimatedStartTime);

    [self waitForExpectations:@[ resubscriptionReachableExpectation, resubscriptionGotReportsExpectation ] timeout:60];

    // Now make sure we ignore later tests.  Ideally we would just unsubscribe
    // or remove the delegate, but there's no good way to do that.
    delegate.onReachable = nil;
    delegate.onNotReachable = nil;
    delegate.onAttributeDataReceived = nil;
    delegate.onEventDataReceived = nil;

    // Make sure we got no updated reports (because we had a cluster state cache
    // with data versions) during the resubscribe.
    XCTAssertEqual(attributeReportsReceived, 0);
    XCTAssertEqual(eventReportsReceived, 0);
}

- (void)test018_SubscriptionErrorWhenNotResubscribing
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * firstSubscribeExpectation = [self expectationWithDescription:@"First subscription complete"];
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"First subscription errored out"];

    // Subscribe
    MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    params.replaceExistingSubscriptions = NO; // Not strictly needed, but checking that doing this does not
    // affect this subscription erroring out correctly.
    __block BOOL subscriptionEstablished = NO;
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            if (subscriptionEstablished) {
                // We should only get an error here.
                XCTAssertNil(values);
                XCTAssertNotNil(error);
                [errorExpectation fulfill];
            } else {
                XCTAssertNotNil(values);
                XCTAssertNil(error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            XCTAssertFalse(subscriptionEstablished);
            subscriptionEstablished = YES;
            [firstSubscribeExpectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:@[ firstSubscribeExpectation ] timeout:kTimeoutInSeconds];

    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    params.replaceExistingSubscriptions = YES;
    [device subscribeAttributeWithEndpointId:@10000
                                   clusterId:@6
                                 attributeId:@0
                                 minInterval:@(1)
                                 maxInterval:@(2)
                                      params:params
                                 clientQueue:queue
                               reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                               }
                     subscriptionEstablished:^() {}];
    [self waitForExpectations:@[ errorExpectation ] timeout:60];
}

- (void)test019_MTRDeviceMultipleCommands
{
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * opcredsCluster = [[MTRClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:queue];
    __auto_type * onOffCluster = [[MTRClusterOnOff alloc] initWithDevice:device endpointID:@(1) queue:queue];
    __auto_type * badOnOffCluster = [[MTRClusterOnOff alloc] initWithDevice:device endpointID:@(0) queue:queue];

    // Ensure our existing fabric label is not "Test".  This uses a "base"
    // cluster to ensure read-through to the other side.
    __auto_type * baseOpCredsCluster = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:GetConnectedDevice() endpointID:@(0) queue:queue];
    XCTestExpectation * readFabricLabelExpectation = [self expectationWithDescription:@"Read fabric label first time"];
    [baseOpCredsCluster readAttributeFabricsWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        XCTAssertEqual(value.count, 1);
        MTROperationalCredentialsClusterFabricDescriptorStruct * entry = value[0];
        XCTAssertNotEqualObjects(entry.label, @"Test");
        [readFabricLabelExpectation fulfill];
    }];

    XCTestExpectation * readFabricIndexExpectation = [self expectationWithDescription:@"Read current fabric index"];
    __block NSNumber * currentFabricIndex;
    [baseOpCredsCluster readAttributeCurrentFabricIndexWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        currentFabricIndex = value;
        [readFabricIndexExpectation fulfill];
    }];

    [self waitForExpectations:@[ readFabricLabelExpectation, readFabricIndexExpectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * onExpectation = [self expectationWithDescription:@"On command executed"];
    [onOffCluster onWithParams:nil
                expectedValues:nil
         expectedValueInterval:nil
                    completion:^(NSError * _Nullable error) {
                        XCTAssertNil(error);
                        [onExpectation fulfill];
                    }];

    XCTestExpectation * offFailedExpectation = [self expectationWithDescription:@"Off command failed"];
    [badOnOffCluster offWithParams:nil
                    expectedValues:nil
             expectedValueInterval:nil
                        completion:^(NSError * _Nullable error) {
                            XCTAssertNotNil(error);
                            [offFailedExpectation fulfill];
                        }];

    XCTestExpectation * updateLabelExpectation = [self expectationWithDescription:@"Fabric label updated"];
    __auto_type * params = [[MTROperationalCredentialsClusterUpdateFabricLabelParams alloc] init];
    params.label = @("Test");
    [opcredsCluster updateFabricLabelWithParams:params
                                 expectedValues:nil
                          expectedValueInterval:nil
                                     completion:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                         NSError * _Nullable error) {
                                         XCTAssertNil(error);
                                         XCTAssertNotNil(data);
                                         XCTAssertEqualObjects(data.statusCode, @(0));
                                         XCTAssertNotNil(data.fabricIndex);
                                         XCTAssertEqualObjects(data.fabricIndex, currentFabricIndex);
                                         [updateLabelExpectation fulfill];
                                     }];

    XCTestExpectation * offExpectation = [self expectationWithDescription:@"Off command executed"];
    // Send this one via MTRDevice, to test that codepath.
    [device invokeCommandWithEndpointID:@(1)
                              clusterID:@(MTRClusterIDTypeOnOffID)
                              commandID:@(MTRCommandIDTypeClusterOnOffCommandOffID)
                          commandFields:nil
                         expectedValues:nil
                  expectedValueInterval:nil
                                  queue:queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 XCTAssertNil(error);
                                 [offExpectation fulfill];
                             }];

    XCTestExpectation * onFailedExpectation = [self expectationWithDescription:@"On command failed"];
    [badOnOffCluster onWithParams:nil
                   expectedValues:nil
            expectedValueInterval:nil
                       completion:^(NSError * _Nullable error) {
                           XCTAssertNotNil(error);
                           [onFailedExpectation fulfill];
                       }];

    XCTestExpectation * updateLabelFailedExpectation = [self expectationWithDescription:@"Fabric label update failed"];
    params.label = @("12345678901234567890123445678901234567890"); // Too long
    [opcredsCluster updateFabricLabelWithParams:params
                                 expectedValues:nil
                          expectedValueInterval:nil
                                     completion:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                         NSError * _Nullable error) {
                                         XCTAssertNotNil(error);
                                         XCTAssertNil(data);
                                         [updateLabelFailedExpectation fulfill];
                                     }];

    [self waitForExpectations:@[
        onExpectation, offFailedExpectation, updateLabelExpectation, offExpectation, onFailedExpectation,
        updateLabelFailedExpectation
    ]
                      timeout:60
                 enforceOrder:YES];

    // Now make sure our fabric label got updated.
    readFabricLabelExpectation = [self expectationWithDescription:@"Read fabric label second time"];
    [baseOpCredsCluster readAttributeFabricsWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        XCTAssertEqual(value.count, 1);
        MTROperationalCredentialsClusterFabricDescriptorStruct * entry = value[0];
        XCTAssertEqualObjects(entry.label, @"Test");
        [readFabricLabelExpectation fulfill];
    }];

    [self waitForExpectations:@[ readFabricLabelExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test020_ReadMultipleAttributes
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read Multiple Attributes (Descriptor, Basic Information Cluster) for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSArray<MTRAttributeRequestPath *> * attributePaths = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@0],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@1],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@2],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@3],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@4],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@5],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@6],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@7]
    ];

    NSArray<MTREventRequestPath *> * eventPaths = @[ [MTREventRequestPath requestPathWithEndpointID:nil clusterID:@40 eventID:@0] ];

    [device readAttributePaths:attributePaths
                    eventPaths:eventPaths
                        params:nil
                         queue:queue
                    completion:^(id _Nullable values, NSError * _Nullable error) {
                        NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                        XCTAssertNil(error);

                        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                        NSArray * resultArray = values;
                        size_t attributeResultCount = 0;
                        size_t eventResultCount = 0;
                        for (NSDictionary * result in resultArray) {
                            if ([result objectForKey:@"eventPath"]) {
                                ++eventResultCount;

                                __auto_type * report = [[MTREventReport alloc] initWithResponseValue:result error:nil];
                                XCTAssertNotNil(report);
                                XCTAssertNotNil(report.path);
                                XCTAssertEqualObjects(report.path.endpoint, @(0));
                                XCTAssertEqualObjects(report.path.cluster, @(40));
                                XCTAssertEqualObjects(report.path.event, @(0));
                                XCTAssertNotNil(report.eventNumber);
                                XCTAssertEqualObjects(report.priority, @(MTREventPriorityCritical));
                                XCTAssertEqual(report.eventTimeType, MTREventTimeTypeTimestampDate);
                                XCTAssertNotNil(report.timestampDate);
                                XCTAssertNotNil(report.value);
                                XCTAssertTrue([report.value isKindOfClass:[MTRBasicInformationClusterStartUpEvent class]]);
                                XCTAssertNil(report.error);

                                MTREventPath * path = result[@"eventPath"];
                                XCTAssertEqualObjects(path.endpoint, @0);
                                XCTAssertEqualObjects(path.cluster, @40);
                                XCTAssertEqualObjects(path.event, @0);
                                XCTAssertNil(result[@"error"]);

                                XCTAssertNotNil(result[@"data"]);
                                XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);

                                XCTAssertNotNil(result[@"eventNumber"]);
                                XCTAssertTrue([result[@"eventNumber"] isKindOfClass:[NSNumber class]]);

                                XCTAssertNotNil(result[@"eventPriority"]);
                                XCTAssertTrue([result[@"eventPriority"] isKindOfClass:[NSNumber class]]);
                                XCTAssertEqualObjects(result[@"eventPriority"], @(MTREventPriorityCritical));

                                XCTAssertNotNil(result[@"eventTimeType"]);
                                XCTAssertTrue([result[@"eventTimeType"] isKindOfClass:[NSNumber class]]);

                                XCTAssertTrue(result[@"eventSystemUpTime"] != nil || result[@"eventTimestampDate"] != nil);
                                if (result[@"eventSystemUpTime"] != nil) {
                                    XCTAssertTrue([result[@"eventSystemUpTime"] isKindOfClass:[NSNumber class]]);
                                } else {
                                    XCTAssertTrue([result[@"eventTimestampDate"] isKindOfClass:[NSDate class]]);
                                }
                            } else if ([result objectForKey:@"attributePath"]) {
                                ++attributeResultCount;

                                __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:result error:nil];
                                XCTAssertNotNil(report);
                                XCTAssertNil(report.error);
                                XCTAssertNotNil(report.value);
                                switch ([report.path.attribute unsignedLongValue]) {
                                case 0:
                                    XCTAssertEqualObjects(report.path.cluster, @29);
                                    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);
                                    for (id entry in report.value) {
                                        XCTAssertTrue([entry isKindOfClass:[MTRDescriptorClusterDeviceTypeStruct class]]);
                                    }
                                    break;
                                case 1:
                                    XCTAssertEqualObjects(report.path.cluster, @29);
                                    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);
                                    for (id entry in report.value) {
                                        XCTAssertTrue([entry isKindOfClass:[NSNumber class]]);
                                    }
                                    break;
                                case 2:
                                    XCTAssertEqualObjects(report.path.cluster, @29);
                                    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);
                                    for (id entry in report.value) {
                                        XCTAssertTrue([entry isKindOfClass:[NSNumber class]]);
                                    }
                                    break;
                                case 3:
                                    XCTAssertEqualObjects(report.path.cluster, @29);
                                    XCTAssertTrue([report.value isKindOfClass:[NSArray class]]);
                                    for (id entry in report.value) {
                                        XCTAssertTrue([entry isKindOfClass:[NSNumber class]]);
                                    }
                                    break;
                                case 4:
                                    XCTAssertEqualObjects(report.path.cluster, @40);
                                    XCTAssertEqualObjects(report.path.endpoint, @0);
                                    XCTAssertTrue([report.value isKindOfClass:[NSNumber class]]);
                                    break;
                                case 5:
                                    XCTAssertEqualObjects(report.path.cluster, @40);
                                    XCTAssertEqualObjects(report.path.endpoint, @0);
                                    XCTAssertTrue([report.value isKindOfClass:[NSString class]]);
                                    break;
                                case 6:
                                    XCTAssertEqualObjects(report.path.cluster, @40);
                                    XCTAssertEqualObjects(report.path.endpoint, @0);
                                    XCTAssertTrue([report.value isKindOfClass:[NSString class]]);
                                    break;
                                case 7:
                                    XCTAssertEqualObjects(report.path.cluster, @40);
                                    XCTAssertEqualObjects(report.path.endpoint, @0);
                                    XCTAssertTrue([report.value isKindOfClass:[NSNumber class]]);
                                    break;
                                }

                                MTRAttributePath * path = result[@"attributePath"];
                                if ([path.attribute unsignedIntegerValue] < 4) {
                                    XCTAssertEqualObjects(path.cluster, @29);
                                    __auto_type endpoint = [path.endpoint unsignedShortValue];
                                    XCTAssertTrue(endpoint == 0 || endpoint == 1 || endpoint == 2);
                                } else {
                                    XCTAssertEqualObjects(path.cluster, @40);
                                    XCTAssertEqualObjects(path.endpoint, @0);
                                }
                                XCTAssertNotNil(result[@"data"]);
                                XCTAssertNil(result[@"error"]);
                                XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                            } else {
                                XCTFail("Unexpected result dictionary %@", result);
                            }
                        }
                        // Our test application has 3 endpoints.  We have a descriptor on each one,
                        // so that's 4 results per endpoint, and we only have Basic Information on
                        // endpoint 0, so that's 4 more results.
                        XCTAssertEqual(attributeResultCount, 3 * 4 + 4);
                        XCTAssertEqual(eventResultCount, [eventPaths count]);

                        [expectation fulfill];
                    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test021_ReadMultipleWildcardPathsIncludeUnsupportedAttribute
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Read the PartList of descriptor on endpoint 0 to find out how many endpoints there are.
    XCTestExpectation * descriptorReadExpectation = [self expectationWithDescription:@"read PartsList from endpoint 0"];
    __auto_type * descriptorCluster = [[MTRBaseClusterDescriptor alloc] initWithDevice:device endpointID:@(0) queue:queue];
    __block size_t endpointCount = 0;
    [descriptorCluster readAttributePartsListWithCompletion:^(NSArray<NSNumber *> * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        endpointCount = value.count + 1; // Include endpoint 0
        [descriptorReadExpectation fulfill];
    }];

    [self waitForExpectations:@[ descriptorReadExpectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read Basic Information Cluster's attributes and include 1 unsupported attribute"];

    NSNumber * failAttributeID = @10000;

    NSArray<MTRAttributeRequestPath *> * attributePaths = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@0],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@1],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@2],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:@3],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@29 attributeID:failAttributeID], // Fail Case
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@4],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@5],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@6],
        [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@40 attributeID:@7]
    ];

    [device readAttributePaths:attributePaths
                    eventPaths:nil
                        params:nil
                         queue:queue
                    completion:^(id _Nullable values, NSError * _Nullable error) {
                        NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                        XCTAssertNil(error);

                        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                        NSArray * resultArray = values;
                        // We have a descriptor on each endpoint, so that's 4 results per endpoint,
                        // and we only have Basic Information on endpoint 0, so that's 4 more
                        // results.  Note that there are no results for failAttributeID, because we
                        // used a wildcard path and hence it got ignored.
                        XCTAssertEqual(resultArray.count, endpointCount * 4 + 4);

                        for (NSDictionary * result in resultArray) {
                            MTRAttributePath * path = result[@"attributePath"];
                            if ([path.attribute unsignedIntegerValue] < 4) {
                                XCTAssertEqualObjects(path.cluster, @29);
                                __auto_type endpoint = [path.endpoint unsignedShortValue];
                                XCTAssertTrue(endpoint == 0 || endpoint == 1 || endpoint == 2);
                            } else {
                                XCTAssertEqualObjects(path.cluster, @40);
                                XCTAssertEqualObjects(path.endpoint, @0);
                            }

                            XCTAssertNotNil(result[@"data"]);
                            XCTAssertNil(result[@"error"]);
                            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                        }

                        [expectation fulfill];
                    }];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test022_ReadMultipleConcretePathsIncludeUnsupportedAttribute
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read Basic Information Cluster's attributes and include 1 unsupported attribute"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSNumber * failAttributeID = @10000;

    NSArray<MTRAttributeRequestPath *> * attributePaths = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@29 attributeID:@0],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@29 attributeID:@1],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@29 attributeID:@2],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@29 attributeID:@3],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@29 attributeID:failAttributeID], // Fail Case
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@40 attributeID:@4],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@40 attributeID:@5],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@40 attributeID:@6],
        [MTRAttributeRequestPath requestPathWithEndpointID:@0 clusterID:@40 attributeID:@7]
    ];

    [device readAttributePaths:attributePaths
                    eventPaths:nil
                        params:nil
                         queue:queue
                    completion:^(id _Nullable values, NSError * _Nullable error) {
                        NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                        XCTAssertNil(error);

                        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                        NSArray * resultArray = values;
                        XCTAssertEqual(resultArray.count, attributePaths.count);

                        for (NSDictionary * result in resultArray) {
                            MTRAttributePath * path = result[@"attributePath"];
                            XCTAssertEqualObjects(path.endpoint, @0);
                            if ([path.attribute unsignedIntegerValue] < 4 || [path.attribute isEqualToNumber:failAttributeID]) {
                                XCTAssertEqualObjects(path.cluster, @29);
                            } else {
                                XCTAssertEqualObjects(path.cluster, @40);
                            }

                            if ([path.attribute isEqualToNumber:failAttributeID]) {
                                XCTAssertNil(result[@"data"]);
                                XCTAssertNotNil(result[@"error"]);
                                XCTAssertTrue([MTRErrorTestUtils error:result[@"error"] isInteractionModelError:MTRInteractionErrorCodeUnsupportedAttribute]);
                            } else {
                                XCTAssertNotNil(result[@"data"]);
                                XCTAssertNil(result[@"error"]);
                                XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                            }
                        }

                        [expectation fulfill];
                    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test023_SubscribeMultipleAttributes
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];

    NSNumber * failClusterId = @5678;
    NSNumber * failEndpointId = @1000;

    NSArray<MTRAttributeRequestPath *> * attributePaths = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:@1 clusterID:@6 attributeID:@0],
        [MTRAttributeRequestPath requestPathWithEndpointID:@1 clusterID:failClusterId attributeID:@1],
    ];

    NSArray<MTREventRequestPath *> * eventPaths = @[ [MTREventRequestPath requestPathWithEndpointID:failEndpointId
                                                                                          clusterID:@40
                                                                                            eventID:@0] ];

    XCTestExpectation * onOffReportExpectation = [self expectationWithDescription:@"report OnOff attribute"];
    XCTestExpectation * attributeErrorReportExpectation = [self expectationWithDescription:@"report nonexistent attribute"];
    XCTestExpectation * eventErrorReportExpectation = [self expectationWithDescription:@"report nonexistent event"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);

        for (NSDictionary * result in values) {
            if (result[@"attributePath"] != nil) {
                MTRAttributePath * path = result[@"attributePath"];

                if ([path.attribute isEqualToNumber:@0]) {
                    XCTAssertEqualObjects(path.endpoint, @1);
                    XCTAssertEqualObjects(path.cluster, @6);
                    XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                    XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
                    XCTAssertEqualObjects(result[@"data"][@"value"], @NO);
                    [onOffReportExpectation fulfill];
                } else if ([path.attribute isEqualToNumber:@1]) {
                    XCTAssertEqualObjects(path.endpoint, @1);
                    XCTAssertEqualObjects(path.cluster, failClusterId);
                    XCTAssertNil(result[@"data"]);
                    XCTAssertNotNil(result[@"error"]);
                    XCTAssertTrue([MTRErrorTestUtils error:result[@"error"] isInteractionModelError:MTRInteractionErrorCodeUnsupportedCluster]);
                    [attributeErrorReportExpectation fulfill];
                } else {
                    XCTFail("Unexpected attribute value");
                }
            } else if (result[@"eventPath"] != nil) {
                MTREventPath * path = result[@"eventPath"];
                XCTAssertEqualObjects(path.endpoint, failEndpointId);
                XCTAssertEqualObjects(path.cluster, @40);
                XCTAssertEqualObjects(path.event, @0);
                XCTAssertNil(result[@"data"]);
                XCTAssertNotNil(result[@"error"]);
                XCTAssertTrue([MTRErrorTestUtils error:result[@"error"] isInteractionModelError:MTRInteractionErrorCodeUnsupportedEndpoint]);
                [eventErrorReportExpectation fulfill];
            } else {
                XCTFail("Unexpected result dictionary");
            }
        }
    };

    [device subscribeToAttributePaths:attributePaths
        eventPaths:eventPaths
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attributes: values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute");
            [expectation fulfill];
        }
        resubscriptionScheduled:nil];

    // Wait till establishment
    [self waitForExpectations:@[ onOffReportExpectation, attributeErrorReportExpectation, eventErrorReportExpectation, expectation ]
                      timeout:kTimeoutInSeconds];

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);

        for (NSDictionary * result in values) {
            // Note: we will get updates for our event subscription too, each time
            // with errors.
            if (result[@"eventPath"] != nil) {
                continue;
            }

            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertNotNil(path);

            // We will only be getting incremental attribute reports for the OnOff attribute.
            XCTAssertEqualObjects(path.endpoint, @1);
            XCTAssertEqualObjects(path.cluster, @6);
            XCTAssertEqualObjects(path.attribute, @0);

            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            if ([result[@"data"][@"value"] boolValue] == YES) {
                [reportExpectation fulfill];
                globalReportHandler = nil;
            }
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : [NSArray array] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     XCTAssertEqual(resultArray.count, 1);
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqualObjects(path.endpoint, @1);
                                         XCTAssertEqualObjects(path.cluster, @6);
                                         XCTAssertEqualObjects(path.command, @1);
                                         XCTAssertNil(result[@"error"]);
                                         // This command just has a status response.
                                         XCTAssertNil(result[@"value"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];

    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Set up expectation for 2nd report
    reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);

        for (NSDictionary * result in values) {
            // Note: we will get updates for our event subscription too, each time
            // with errors.
            if (result[@"eventPath"] != nil) {
                continue;
            }

            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertNotNil(path);

            XCTAssertEqualObjects(path.endpoint, @1);
            XCTAssertEqualObjects(path.cluster, @6);
            XCTAssertEqualObjects(path.attribute, @0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            if ([result[@"data"][@"value"] boolValue] == NO) {
                [reportExpectation fulfill];
                globalReportHandler = nil;
            }
        }
    };

    // Send command to trigger attribute change
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqualObjects(path.endpoint, @1);
                                         XCTAssertEqualObjects(path.cluster, @6);
                                         XCTAssertEqualObjects(path.command, @0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    expectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [expectation fulfill];
                                   }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test024_SubscribeMultipleAttributesAllErrors
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"subscribe failure"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;

    NSNumber * failClusterId = @5678;
    NSNumber * failEndpointId = @1000;

    // All the paths are invalid, so we will just get an INVALID_ACTION error.
    NSArray<MTRAttributeRequestPath *> * attributePaths = @[
        [MTRAttributeRequestPath requestPathWithEndpointID:failEndpointId clusterID:@6 attributeID:@0],
        [MTRAttributeRequestPath requestPathWithEndpointID:@1 clusterID:failClusterId attributeID:@1],
    ];

    [device subscribeToAttributePaths:attributePaths
        eventPaths:nil
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            XCTAssertNotNil(error);
            XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeInvalidAction]);
            XCTAssertNil(values);
            [errorExpectation fulfill];
        }
        subscriptionEstablished:^{
            XCTFail("This subscription should never be established");
        }
        resubscriptionScheduled:nil];

    [self waitForExpectations:@[ errorExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test025_SubscribeMultipleEvents
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe multiple events"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];

    NSArray<MTREventRequestPath *> * eventPaths = @[
        // Startup event.
        [MTREventRequestPath requestPathWithEndpointID:@0 clusterID:@40 eventID:@0],
        // Shutdown event.
        [MTREventRequestPath requestPathWithEndpointID:@0 clusterID:@40 eventID:@1],
    ];

    XCTestExpectation * startupEventExpectation = [self expectationWithDescription:@"report startup event"];
    __auto_type reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);

        for (NSDictionary * result in values) {
            XCTAssertNotNil(result[@"eventPath"]);

            MTREventPath * path = result[@"eventPath"];
            // We only expect to see a Startup event here.
            XCTAssertEqualObjects(path.endpoint, @0);
            XCTAssertEqualObjects(path.cluster, @40);
            XCTAssertEqualObjects(path.event, @0);

            XCTAssertNil(result[@"error"]);

            XCTAssertNotNil(result[@"data"]);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);

            XCTAssertNotNil(result[@"eventNumber"]);
            XCTAssertTrue([result[@"eventNumber"] isKindOfClass:[NSNumber class]]);

            XCTAssertNotNil(result[@"eventPriority"]);
            XCTAssertTrue([result[@"eventPriority"] isKindOfClass:[NSNumber class]]);
            XCTAssertEqualObjects(result[@"eventPriority"], @(MTREventPriorityCritical));

            XCTAssertNotNil(result[@"eventTimeType"]);
            XCTAssertTrue([result[@"eventTimeType"] isKindOfClass:[NSNumber class]]);

            XCTAssertTrue(result[@"eventSystemUpTime"] != nil || result[@"eventTimestampDate"] != nil);
            if (result[@"eventSystemUpTime"] != nil) {
                XCTAssertTrue([result[@"eventSystemUpTime"] isKindOfClass:[NSNumber class]]);
            } else {
                XCTAssertTrue([result[@"eventTimestampDate"] isKindOfClass:[NSDate class]]);
            }

            [startupEventExpectation fulfill];
        }
    };

    [device subscribeToAttributePaths:nil
                           eventPaths:eventPaths
                               params:params
                                queue:queue
                        reportHandler:reportHandler
              subscriptionEstablished:^{
                  NSLog(@"subscribe complete");
                  [expectation fulfill];
              }
              resubscriptionScheduled:nil];

    // Wait till establishment
    [self waitForExpectations:@[ startupEventExpectation, expectation ] timeout:kTimeoutInSeconds];
}

- (void)test026_LocationAttribute
{
    __auto_type * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * expectation = [self expectationWithDescription:@"read Basic Information Location attribute"];

    __auto_type * cluster = [[MTRBaseClusterBasicInformation alloc] initWithDevice:device endpointID:@(0) queue:queue];
    [cluster readAttributeLocationWithCompletion:^(NSString * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);

        // Matches what we passed in during commissioning.
        XCTAssertEqualObjects(value, @"au");
        [expectation fulfill];
    }];

    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test027_AttestationChallenge
{
    // Check that we have an attestation challenge result in
    // MTROperationalCredentialsClusterAttestationResponseParams.
    dispatch_queue_t queue = dispatch_get_main_queue();

    // We don't care about our nonce being random here, so just all-0 is fine.
    __auto_type * nonce = [[NSMutableData alloc] initWithLength:32];
    __auto_type * params = [[MTROperationalCredentialsClusterAttestationRequestParams alloc] init];
    params.attestationNonce = nonce;

    __auto_type * baseDevice = GetConnectedDevice();
    __auto_type * baseCluster = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:baseDevice endpointID:@(0) queue:queue];
    XCTestExpectation * attestationRequestedViaBaseCluster = [self expectationWithDescription:@"Invoked AttestationRequest via base cluster"];
    [baseCluster attestationRequestWithParams:params completion:^(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(data);
        XCTAssertNotNil(data.attestationChallenge);
        [attestationRequestedViaBaseCluster fulfill];
    }];

    [self waitForExpectations:@[ attestationRequestedViaBaseCluster ] timeout:kTimeoutInSeconds];

    __auto_type * requestFields = @{
        MTRTypeKey : MTRStructureValueType,
        MTRValueKey : @[
            @{
                MTRContextTagKey : @(0), // AttestationNonce
                MTRDataKey : @ {
                    MTRTypeKey : MTROctetStringValueType,
                    MTRValueKey : nonce,
                },
            },
        ],
    };

    XCTestExpectation * attestationRequestedViaBaseDevice = [self expectationWithDescription:@"Invoked AttestationRequest via base device"];
    [baseDevice invokeCommandWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeOperationalCredentialsID) commandID:@(MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID) commandFields:requestFields timedInvokeTimeout:nil queue:queue completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(values);
        XCTAssertEqual(values.count, 1);
        __auto_type * response = [[MTROperationalCredentialsClusterAttestationResponseParams alloc] initWithResponseValue:values[0] error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(response);
        XCTAssertNotNil(response.attestationChallenge);
        [attestationRequestedViaBaseDevice fulfill];
    }];

    [self waitForExpectations:@[ attestationRequestedViaBaseDevice ] timeout:kTimeoutInSeconds];

    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    __auto_type * cluster = [[MTRClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:queue];
    XCTestExpectation * attestationRequestedViaCluster = [self expectationWithDescription:@"Invoked AttestationRequest via cluster"];
    [cluster attestationRequestWithParams:params expectedValues:nil expectedValueInterval:nil completion:^(MTROperationalCredentialsClusterAttestationResponseParams * _Nullable data, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(data);
        XCTAssertNotNil(data.attestationChallenge);
        [attestationRequestedViaCluster fulfill];
    }];

    [self waitForExpectations:@[ attestationRequestedViaCluster ] timeout:kTimeoutInSeconds];

    XCTestExpectation * attestationRequestedViaDevice = [self expectationWithDescription:@"Invoked AttestationRequest via device"];
    [device invokeCommandWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeOperationalCredentialsID) commandID:@(MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID) commandFields:requestFields expectedValues:nil expectedValueInterval:nil timedInvokeTimeout:nil queue:queue completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(values);
        XCTAssertEqual(values.count, 1);
        __auto_type * response = [[MTROperationalCredentialsClusterAttestationResponseParams alloc] initWithResponseValue:values[0] error:&error];
        XCTAssertNil(error);
        XCTAssertNotNil(response);
        XCTAssertNotNil(response.attestationChallenge);
        [attestationRequestedViaDevice fulfill];
    }];

    [self waitForExpectations:@[ attestationRequestedViaDevice ] timeout:kTimeoutInSeconds];
}

- (void)test028_TimeZoneAndDST
{
    // Time synchronization is marked provisional so far, so we can only test it
    // when MTR_ENABLE_PROVISIONAL is set.
#if MTR_ENABLE_PROVISIONAL
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * device = GetConnectedDevice();
    __auto_type * cluster = [[MTRBaseClusterTimeSynchronization alloc] initWithDevice:device endpointID:@(0) queue:queue];

    XCTestExpectation * readTimeZoneExpectation = [self expectationWithDescription:@"Read TimeZone attribute"];
    __block NSArray<MTRTimeSynchronizationClusterTimeZoneStruct *> * timeZone;
    [cluster readAttributeTimeZoneWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        timeZone = value;
        [readTimeZoneExpectation fulfill];
    }];

    [self waitForExpectations:@[ readTimeZoneExpectation ] timeout:kTimeoutInSeconds];

    __block NSArray<MTRTimeSynchronizationClusterDSTOffsetStruct *> * dstOffset;
    XCTestExpectation * readDSTOffsetExpectation = [self expectationWithDescription:@"Read DSTOffset attribute"];
    [cluster readAttributeDSTOffsetWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        dstOffset = value;
        [readDSTOffsetExpectation fulfill];
    }];

    [self waitForExpectations:@[ readDSTOffsetExpectation ] timeout:kTimeoutInSeconds];

    // Check that the first DST offset entry matches what we expect.  If we
    // happened to cross a DST boundary during execution of this function, some
    // of these checks will fail, but that seems pretty low-probability.

    XCTAssertTrue(dstOffset.count > 0);
    MTRTimeSynchronizationClusterDSTOffsetStruct * currentDSTOffset = dstOffset[0];

    __auto_type * utcTz = [NSTimeZone timeZoneForSecondsFromGMT:0];
    __auto_type * dateComponents = [[NSDateComponents alloc] init];
    dateComponents.timeZone = utcTz;
    dateComponents.year = 2000;
    dateComponents.month = 1;
    dateComponents.day = 1;
    NSCalendar * gregorianCalendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDate * matterEpoch = [gregorianCalendar dateFromComponents:dateComponents];

    NSDate * nextReportedDSTTransition;
    if (currentDSTOffset.validUntil == nil) {
        nextReportedDSTTransition = nil;
    } else {
        double validUntilMicroSeconds = currentDSTOffset.validUntil.doubleValue;
        nextReportedDSTTransition = [NSDate dateWithTimeInterval:validUntilMicroSeconds / 1e6 sinceDate:matterEpoch];
    }

    __auto_type * tz = [NSTimeZone localTimeZone];
    NSDate * nextDSTTransition = tz.nextDaylightSavingTimeTransition;
    XCTAssertEqualObjects(nextReportedDSTTransition, nextDSTTransition);

    XCTAssertEqual(currentDSTOffset.offset.intValue, tz.daylightSavingTimeOffset);

    // Now check the timezone info we got.  We always set exactly one timezone.
    XCTAssertEqual(timeZone.count, 1);
    MTRTimeSynchronizationClusterTimeZoneStruct * currentTimeZone = timeZone[0];
    XCTAssertEqual(tz.secondsFromGMT, currentTimeZone.offset.intValue + currentDSTOffset.offset.intValue);
#endif // MTR_ENABLE_PROVISIONAL
}

- (void)test900_SubscribeAllAttributes
{
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:nil
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"Subscribe all - report attribute values: %@, error: %@, report handler: %d", values, error,
                (reportHandler != nil));

            if (reportHandler) {
                __auto_type callback = reportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    // Set up expectation for report
    __auto_type reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);

        if ([path.attribute unsignedIntegerValue] == 0 && [result[@"data"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send commands to set attribute state to a known state
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : @[] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Send commands to trigger attribute change
    commandExpectation = [self expectationWithDescription:@"command responded"];
    fields = @{ @"type" : @"Structure", @"value" : @[] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Set up expectation for 2nd report
    reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        if ([path.attribute unsignedIntegerValue] == 0 && [result[@"data"][@"value"] boolValue] == NO) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send command to trigger attribute change
    commandExpectation = [self expectationWithDescription:@"command responded"];
    fields = @{ @"type" : @"Structure", @"value" : @[] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertNil(error);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];
}

- (void)test029_PathsBehavior
{
    __auto_type * commandPath1 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(3)];
    __auto_type * commandPath2 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(4)];
    __auto_type * commandPath3 = [MTRCommandPath commandPathWithEndpointID:@(2) clusterID:@(2) commandID:@(3)];
    __auto_type * commandPath4 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(1) commandID:@(3)];
    __auto_type * commandPath5 = [MTRCommandPath commandPathWithEndpointID:@(1) clusterID:@(2) commandID:@(3)];

    __auto_type * eventPath1 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(3)];
    __auto_type * eventPath2 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(4)];
    __auto_type * eventPath3 = [MTREventPath eventPathWithEndpointID:@(2) clusterID:@(2) eventID:@(3)];
    __auto_type * eventPath4 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(1) eventID:@(3)];
    __auto_type * eventPath5 = [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(2) eventID:@(3)];

    __auto_type * attributePath1 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(3)];
    __auto_type * attributePath2 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(4)];
    __auto_type * attributePath3 = [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(2) attributeID:@(3)];
    __auto_type * attributePath4 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(1) attributeID:@(3)];
    __auto_type * attributePath5 = [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(2) attributeID:@(3)];

    __auto_type * clusterPath1 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)];
    __auto_type * clusterPath2 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(3)];
    __auto_type * clusterPath3 = [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(2)];
    __auto_type * clusterPath4 = [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(2)];

    // Command paths
    XCTAssertTrue([commandPath1 isEqual:commandPath5]);
    XCTAssertEqualObjects(commandPath1, commandPath5);

    XCTAssertFalse([commandPath1 isEqual:commandPath2]);
    XCTAssertNotEqualObjects(commandPath1, commandPath2);

    XCTAssertFalse([commandPath1 isEqual:commandPath3]);
    XCTAssertNotEqualObjects(commandPath1, commandPath3);

    XCTAssertFalse([commandPath1 isEqual:commandPath4]);
    XCTAssertNotEqualObjects(commandPath1, commandPath4);

    // Event paths
    XCTAssertTrue([eventPath1 isEqual:eventPath5]);
    XCTAssertEqualObjects(eventPath1, eventPath5);

    XCTAssertFalse([eventPath1 isEqual:eventPath2]);
    XCTAssertNotEqualObjects(eventPath1, eventPath2);

    XCTAssertFalse([eventPath1 isEqual:eventPath3]);
    XCTAssertNotEqualObjects(eventPath1, eventPath3);

    XCTAssertFalse([eventPath1 isEqual:eventPath4]);
    XCTAssertNotEqualObjects(eventPath1, eventPath4);

    // Attribute paths
    XCTAssertTrue([attributePath1 isEqual:attributePath5]);
    XCTAssertEqualObjects(attributePath1, attributePath5);

    XCTAssertFalse([attributePath1 isEqual:attributePath2]);
    XCTAssertNotEqualObjects(attributePath1, attributePath2);

    XCTAssertFalse([attributePath1 isEqual:attributePath3]);
    XCTAssertNotEqualObjects(attributePath1, attributePath3);

    XCTAssertFalse([attributePath1 isEqual:attributePath4]);
    XCTAssertNotEqualObjects(attributePath1, attributePath4);

    // Clusters
    XCTAssertTrue([clusterPath1 isEqual:clusterPath4]);
    XCTAssertEqualObjects(clusterPath1, clusterPath4);

    XCTAssertFalse([clusterPath1 isEqual:clusterPath2]);
    XCTAssertNotEqualObjects(clusterPath1, clusterPath2);

    XCTAssertFalse([clusterPath1 isEqual:clusterPath3]);
    XCTAssertNotEqualObjects(clusterPath1, clusterPath3);

    // Mix
    XCTAssertFalse([commandPath1 isEqual:eventPath1]);
    XCTAssertFalse([eventPath1 isEqual:commandPath1]);

    XCTAssertFalse([commandPath1 isEqual:attributePath1]);
    XCTAssertFalse([attributePath1 isEqual:commandPath1]);

    XCTAssertFalse([attributePath1 isEqual:eventPath1]);
    XCTAssertFalse([eventPath1 isEqual:attributePath1]);
    XCTAssertFalse([clusterPath1 isEqual:attributePath1]);
    XCTAssertFalse([clusterPath1 isEqual:eventPath1]);
}

- (void)test030_DeviceAndClusterProperties
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    XCTAssertEqual(device.deviceController, sController);
    XCTAssertEqualObjects(device.nodeID, @(kDeviceId));

    __auto_type * cluster = [[MTRClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:queue];
    XCTAssertEqual(cluster.device, device);
    XCTAssertEqualObjects(cluster.endpointID, @(0));
}

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
- (void)test031_MTRDeviceAttributeCacheLocalTestStorage
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    // First start with clean slate by removing the MTRDevice and clearing the persisted cache
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    [sController removeDevice:device];
    [sController.controllerDataStore clearAllStoredAttributes];
    NSArray * storedAttributesAfterClear = [sController.controllerDataStore getStoredAttributesForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedAttributesAfterClear.count, 0);

    // Now recreate device and get subscription primed
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received"];
    XCTestExpectation * gotDeviceCachePrimed = [self expectationWithDescription:@"Device cache primed for the first time"];
    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    __weak __auto_type weakDelegate = delegate;
    delegate.onReportEnd = ^{
        [gotReportsExpectation fulfill];
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };
    delegate.onDeviceCachePrimed = ^{
        [gotDeviceCachePrimed fulfill];
    };
    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ gotReportsExpectation, gotDeviceCachePrimed ] timeout:60];

    NSUInteger attributesReportedWithFirstSubscription = [device unitTestAttributesReportedSinceLastCheck];

    NSArray * dataStoreValuesAfterFirstSubscription = [sController.controllerDataStore getStoredAttributesForNodeID:@(kDeviceId)];
    XCTAssertTrue(dataStoreValuesAfterFirstSubscription.count > 0);

    // Now remove device, resubscribe, and see that it succeeds
    [sController removeDevice:device];
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];

    XCTestExpectation * resubGotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received for resubscription"];
    delegate.onReportEnd = ^{
        [resubGotReportsExpectation fulfill];
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };
    __block BOOL onDeviceCachePrimedCalled = NO;
    delegate.onDeviceCachePrimed = ^{
        onDeviceCachePrimedCalled = YES;
    };
    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ resubGotReportsExpectation ] timeout:60];

    // Make sure that the new callback is only ever called once, the first time subscription was primed
    XCTAssertFalse(onDeviceCachePrimedCalled);

    NSUInteger attributesReportedWithSecondSubscription = [device unitTestAttributesReportedSinceLastCheck];

    XCTAssertTrue(attributesReportedWithSecondSubscription < attributesReportedWithFirstSubscription);

    // 1) MTRDevice actually gets some attributes reported more than once
    // 2) Some attributes do change on resubscribe
    //   * With all-clusts-app as of 2024-02-10, out of 1287 persisted attributes, still 450 attributes were reported with filter
    // And so conservatively, assert that data version filters save at least 300 entries.
    NSArray * dataStoreValuesAfterSecondSubscription = [sController.controllerDataStore getStoredAttributesForNodeID:@(kDeviceId)];
    NSUInteger storedAttributeCountDifferenceFromMTRDeviceReport = dataStoreValuesAfterSecondSubscription.count - attributesReportedWithSecondSubscription;
    XCTAssertTrue(storedAttributeCountDifferenceFromMTRDeviceReport > 300);
}
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

@end

@interface MTRDeviceEncoderTests : XCTestCase
@end

@implementation MTRDeviceEncoderTests

- (void)testSignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:-713], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testSignedInteger64Bits
{
    NSDictionary * input = [NSDictionary
        dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:-0x7000111122223333ll], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUnsignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:1025], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUnsignedInteger64Bits
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                         [NSNumber numberWithUnsignedLongLong:0xCCCCDDDDEEEEFFFFull], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testBoolean
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type", [NSNumber numberWithBool:YES], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUTF8String
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"UTF8String", @"type", @"Hello World", @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testOctetString
{
    const uint8_t data[] = { 0x00, 0xF2, 0x63 };
    NSDictionary * input = [NSDictionary
        dictionaryWithObjectsAndKeys:@"OctetString", @"type", [NSData dataWithBytes:data length:sizeof(data)], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testFloat
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"type", [NSNumber numberWithFloat:0.1245f], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output[@"type"] isEqualToString:@"Float"]);
    XCTAssertTrue(([output[@"value"] floatValue] - [input[@"value"] floatValue]) < 0.0001);
}

- (void)testDouble
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Double", @"type", [NSNumber numberWithDouble:0.1245], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output[@"type"] isEqualToString:@"Double"]);
    XCTAssertTrue(([output[@"value"] doubleValue] - [input[@"value"] doubleValue]) < 0.0001);
}

- (void)testNull
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"Null", @"type", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testStructure
{
    NSArray * inputFields = @[
        @{
            @"contextTag" : @1,
            @"data" : @ { @"type" : @"Boolean", @"value" : @NO },
        },
        @{
            @"contextTag" : @2,
            @"data" : @ { @"type" : @"SignedInteger", @"value" : @5 },
        }
    ];
    NSDictionary * inputValue = @{ @"type" : @"Structure", @"value" : inputFields };

    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);

    XCTAssertTrue([output isEqual:inputValue]);
}

- (void)testArray
{
    NSArray * inputFields = @[
        @{ @"data" : @ { @"type" : @"Boolean", @"value" : @NO } }, @{ @"data" : @ { @"type" : @"SignedInteger", @"value" : @5 } }
    ];
    NSDictionary * inputValue = @{ @"type" : @"Array", @"value" : inputFields };

    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:inputValue]);
}

@end
