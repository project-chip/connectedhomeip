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
#import "MTRDeviceClusterData.h"
#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRDeviceDataValidation.h"
#import "MTRDeviceStorageBehaviorConfiguration.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRDevice_Internal.h"
#import "MTRErrorTestUtils.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestResetCommissioneeHelper.h"
#import "MTRTestStorage.h"

#import <math.h> // For INFINITY
#import <os/lock.h>

// system dependencies
#import <XCTest/XCTest.h>

// Fixture: chip-all-clusters-app --KVS "$(mktemp -t chip-test-kvs)" --interface-id -1

static const uint16_t kPairingTimeoutInSeconds = 30;
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

    NSError * getDeviceError = nil;
    __auto_type * device = [controller deviceBeingCommissionedWithNodeID:@(kDeviceId) error:&getDeviceError];
    XCTAssertNil(getDeviceError);
    XCTAssertNotNil(device);

    // Now check that getting with some other random id fails.
    device = [controller deviceBeingCommissionedWithNodeID:@(kDeviceId + 1) error:&getDeviceError];
    XCTAssertNil(device);
    XCTAssertNotNil(getDeviceError);

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

static BOOL slocalTestStorageEnabledBeforeUnitTest;

+ (void)setUp
{
    XCTestExpectation * pairingExpectation = [[XCTestExpectation alloc] initWithDescription:@"Pairing Complete"];

    slocalTestStorageEnabledBeforeUnitTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = YES;

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

    // Restore testing setting to previous state, and remove all persisted attributes
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = slocalTestStorageEnabledBeforeUnitTest;
    [sController.controllerDataStore clearAllStoredClusterData];
    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

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

    // Ensure the test starts with clean slate in terms of stored data.
    if (sController != nil) {
        [sController.controllerDataStore clearAllStoredClusterData];
        NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
        XCTAssertEqual(storedClusterDataAfterClear.count, 0);
    }
}

- (void)tearDown
{
    // Make sure our MTRDevice instances, which are stateful, do not keep that
    // state between different tests.
    if (sController != nil) {
        __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
        [sController removeDevice:device];
    }

    // Try to make sure we don't have any outstanding subscriptions from
    // previous tests, by sending a subscribe request that will get rid of
    // existing subscriptions and then fail out due to requesting a subscribe to
    // a nonexistent cluster.
    if (mConnectedDevice != nil) {
        dispatch_queue_t queue = dispatch_get_main_queue();

        MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)];
        params.resubscribeAutomatically = NO;
        params.replaceExistingSubscriptions = YES;

        XCTestExpectation * errorExpectation = [self expectationWithDescription:@"Canceled all subscriptions"];

        // There should be no Basic Information cluster on random endpoints.
        [mConnectedDevice subscribeToAttributesWithEndpointID:@10000
            clusterID:@(MTRClusterIDTypeBasicInformationID)
            attributeID:@(0)
            params:params
            queue:queue
            reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                XCTAssertNil(values);
                XCTAssertNotNil(error);
                [errorExpectation fulfill];
            }
            subscriptionEstablished:^() {
                XCTFail("Did not expect subscription to Basic Information on random endpoint to succeed");
            }];

        [self waitForExpectations:@[ errorExpectation ] timeout:kTimeoutInSeconds];
    }

    [super tearDown];
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
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(60)];
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
    MTRSubscribeParams * newParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(60)];
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
            globalReportHandler = nil;
            [reportExpectation fulfill];
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

    XCTestExpectation * errorExpectation1 = [self expectationWithDescription:@"First subscription errored out"];
    XCTestExpectation * errorExpectation2 = [self expectationWithDescription:@"Second subscription errored out"];

    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertNil(values);
        XCTAssertNotNil(error);
        globalReportHandler = nil;
        [errorExpectation1 fulfill];
    };

    // Try to create a second subscription, which will cancel the first
    // subscription.  We can use a non-existent path here to cut down on the
    // work that gets done.
    params.replaceExistingSubscriptions = YES;
    [device subscribeToAttributesWithEndpointID:@10000
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            XCTAssertNil(values);
            XCTAssertNotNil(error);
            [errorExpectation2 fulfill];
        }
        subscriptionEstablished:^() {
            XCTFail("Did not expect this subscription to succeed");
        }];
    [self waitForExpectations:@[ errorExpectation1, errorExpectation2 ] timeout:60];

    deregisterExpectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [deregisterExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ deregisterExpectation ] timeout:kTimeoutInSeconds];
}
#endif // DEBUG

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
    params.replaceExistingSubscriptions = NO; // Not strictly needed, but checking that doing this does not affect this subscription erroring out correctly.
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
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Given reachable state becomes true before underlying OnSubscriptionEstablished callback, this expectation is necessary but
    // not sufficient as a mark to the end of reports
    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onReachable = ^() {
        [subscriptionExpectation fulfill];
    };

    NSMutableSet<NSNumber *> * endpoints = [[NSMutableSet alloc] init];
    NSMutableSet<MTRClusterPath *> * clusters = [[NSMutableSet alloc] init];
    NSMutableSet<NSNumber *> * rootBasicInformationAttributes = [[NSMutableSet alloc] init];
    __block unsigned attributeReportsReceived = 0;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * dataItem in data) {
            MTRAttributePath * path = dataItem[MTRAttributePathKey];
            XCTAssertNotNil(path);
            if (dataItem[MTRDataKey]) {
                [endpoints addObject:path.endpoint];
                [clusters addObject:[MTRClusterPath clusterPathWithEndpointID:path.endpoint clusterID:path.cluster]];
                if (path.endpoint.unsignedLongValue == 0 && path.cluster.unsignedLongValue == MTRClusterIDTypeBasicInformationID) {
                    [rootBasicInformationAttributes addObject:path.attribute];
                }
            }
        }

        attributeReportsReceived += data.count;
    };

    // This is dependent on current implementation that priming reports send attributes and events in that order, and also that
    // events in this test would fit in one report. So receiving events would mean all attributes and events have been received, and
    // can satisfy the test below.
    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received"];
    __block unsigned eventReportsReceived = 0;
    __block BOOL reportEnded = NO;
    __block BOOL gotOneNonPrimingEvent = NO;
    __block NSNumber * lastObservedEventNumber = @(0);
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
                lastObservedEventNumber = eventDict[MTREventNumberKey];
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
            MTREventTimestampDateKey : [NSDate date],
            MTREventIsHistoricalKey : @(NO),
            MTREventPriorityKey : @(MTREventPriorityInfo),
            MTREventNumberKey : @(lastObservedEventNumber.unsignedLongLongValue + 1),
            MTRDataKey : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[],
            },
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

    // Test readAttributePaths.  First, try DeviceTypeList across all endpoints.
    __auto_type * deviceTypeListPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID)];
    __auto_type * deviceTypes = [device readAttributePaths:@[ deviceTypeListPath ]];
    XCTAssertEqual(deviceTypes.count, endpoints.count);

    // Now try ClusterRevision across all clusters.
    __auto_type * clusterRevisionsPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:nil attributeID:@(MTRAttributeIDTypeGlobalAttributeClusterRevisionID)];
    __auto_type * clusterRevisions = [device readAttributePaths:@[ clusterRevisionsPath ]];
    XCTAssertEqual(clusterRevisions.count, clusters.count);

    // Now try BasicInformation in a few different ways:
    __auto_type * basicInformationAllAttributesPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@(MTRClusterIDTypeBasicInformationID) attributeID:nil];
    __auto_type * basicInformationAllAttributes = [device readAttributePaths:@[ basicInformationAllAttributesPath ]];

    __auto_type * basicInformationAllRootAttributesPath = [MTRAttributeRequestPath requestPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeBasicInformationID) attributeID:nil];
    __auto_type * basicInformationAllRootAttributes = [device readAttributePaths:@[ basicInformationAllRootAttributesPath ]];
    // Should have gotten the same things, because Basic Information only exists
    // on the root endpoint.
    XCTAssertEqualObjects([NSSet setWithArray:basicInformationAllAttributes], [NSSet setWithArray:basicInformationAllRootAttributes]);
    XCTAssertEqual(basicInformationAllAttributes.count, rootBasicInformationAttributes.count);

    // Now try multiple paths.  Should just get the union of all the things for
    // each path.
    __auto_type * variousThings = [device readAttributePaths:@[ deviceTypeListPath, basicInformationAllRootAttributesPath ]];
    XCTAssertEqualObjects([NSSet setWithArray:variousThings],
        [[NSSet setWithArray:deviceTypes] setByAddingObjectsFromSet:[NSSet setWithArray:basicInformationAllRootAttributes]]);

    // And similar if the paths expand to overlapping sets of existent paths
    // (e.g. because Basic Information has a ClusterRevision).
    variousThings = [device readAttributePaths:@[ clusterRevisionsPath, basicInformationAllRootAttributesPath ]];
    XCTAssertEqualObjects([NSSet setWithArray:variousThings],
        [[NSSet setWithArray:clusterRevisions] setByAddingObjectsFromSet:[NSSet setWithArray:basicInformationAllRootAttributes]]);

    // Quick test for descriptorClusters
    __auto_type * descriptorPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:nil];
    __auto_type * descriptorRead = [device descriptorClusters];
    __auto_type * descriptorWildcardRead = [device readAttributePaths:@[ descriptorPath ]];
    XCTAssertEqual(descriptorRead.count, descriptorWildcardRead.count);
    for (MTRAttributePath * path in descriptorRead) {
        __auto_type * expectedObj = @{
            MTRAttributePathKey : path,
            MTRDataKey : descriptorRead[path],
        };
        XCTAssertTrue([descriptorWildcardRead containsObject:expectedObj]);
    }
    for (NSDictionary * item in descriptorWildcardRead) {
        XCTAssertEqualObjects(descriptorRead[item[MTRAttributePathKey]], item[MTRDataKey]);
    }

    // Some quick tests for waitForAttributeValues.  First, values that we know
    // are already there:
    XCTestExpectation * deviceTypesWaitExpectation = [self expectationWithDescription:@"deviceTypes is already the value we expect"];
    __auto_type * deviceTypesToWaitFor = [NSMutableDictionary dictionary];
    for (NSDictionary<NSString *, id> * deviceTypesValue in deviceTypes) {
        deviceTypesToWaitFor[deviceTypesValue[MTRAttributePathKey]] = deviceTypesValue[MTRDataKey];
    }
    __auto_type * waiter1 = [device waitForAttributeValues:deviceTypesToWaitFor timeout:200 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [deviceTypesWaitExpectation fulfill];
    }];
    XCTAssertNotNil(waiter1);
    [self waitForExpectations:@[ deviceTypesWaitExpectation ] timeout:kTimeoutInSeconds];

    // Now values that we know will never be there (the type is wrong).
    __auto_type * bogusDeviceType = @{
        deviceTypes[0][MTRAttributePathKey] : @ {
            MTRTypeKey : MTROctetStringValueType,
            MTRValueKey : [@"abc" dataUsingEncoding:NSUTF8StringEncoding],
        },
    };
    XCTestExpectation * bogusDeviceTypesWaitExpectation = [self expectationWithDescription:@"bogusDeviceTypes wait should time out"];
    __auto_type * waiter2 = [device waitForAttributeValues:bogusDeviceType timeout:0.5 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNotNil(error);
        XCTAssertEqual(error.domain, MTRErrorDomain);
        XCTAssertEqual(error.code, MTRErrorCodeTimeout);
        [bogusDeviceTypesWaitExpectation fulfill];
    }];
    XCTAssertNotNil(waiter2);
    [self waitForExpectations:@[ bogusDeviceTypesWaitExpectation ] timeout:kTimeoutInSeconds];

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

    // Also test that waitForAttributeValues does not pick up the transition to
    // the expected value.
    XCTestExpectation * nonexistentAttributeValueWaitExpectation = [self expectationWithDescription:@"waiting for a value for an attribute that does not exist should time out"];
    __auto_type * waiter3 = [device waitForAttributeValues:@{
        [MTRAttributePath attributePathWithEndpointID:testEndpointID clusterID:testClusterID attributeID:testAttributeID] : writeValue,
    }
                                                   timeout:0.5
                                                     queue:queue
                                                completion:^(NSError * _Nullable error) {
                                                    XCTAssertNotNil(error);
                                                    XCTAssertEqual(error.domain, MTRErrorDomain);
                                                    XCTAssertEqual(error.code, MTRErrorCodeTimeout);
                                                    [nonexistentAttributeValueWaitExpectation fulfill];
                                                }];
    XCTAssertNotNil(waiter3);

    [device writeAttributeWithEndpointID:testEndpointID
                               clusterID:testClusterID
                             attributeID:testAttributeID
                                   value:writeValue
                   expectedValueInterval:@(20000)
                       timedWriteTimeout:nil];

    // expected value interval is 20s but expect it get reverted immediately as the write fails because it's writing to a
    // nonexistent attribute
    [self waitForExpectations:@[ expectedValueReportedExpectation, expectedValueRemovedExpectation, nonexistentAttributeValueWaitExpectation ] timeout:5 enforceOrder:YES];

    // Get the current OnTime value, to make sure we are writing a different one.
    __auto_type * currentOnTimeValueDictionary = [device readAttributeWithEndpointID:@(1)
                                                                           clusterID:@(MTRClusterIDTypeOnOffID)
                                                                         attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)
                                                                              params:nil];
    XCTAssertNotNil(currentOnTimeValueDictionary);
    XCTAssertEqualObjects(currentOnTimeValueDictionary[MTRTypeKey], MTRUnsignedIntegerValueType);
    uint16_t currentOnTimeValue = [currentOnTimeValueDictionary[MTRValueKey] unsignedShortValue];

    // Test if previous value is reported on a write.  Make sure to write a
    // value different from the current one.
    uint16_t testOnTimeValue = currentOnTimeValue > 10 ? currentOnTimeValue - 1 : currentOnTimeValue + 1;
    XCTestExpectation * onTimeWriteSuccess = [self expectationWithDescription:@"OnTime write success"];
    XCTestExpectation * onTimePreviousValue = [self expectationWithDescription:@"OnTime previous value"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * attributeResponseValue in data) {
            MTRAttributePath * path = attributeResponseValue[MTRAttributePathKey];
            if (path.cluster.unsignedIntValue == MTRClusterIDTypeOnOffID && path.attribute.unsignedLongValue == MTRAttributeIDTypeClusterOnOffAttributeOnTimeID) {
                NSDictionary * dataValue = attributeResponseValue[MTRDataKey];
                NSNumber * onTimeValue = dataValue[MTRValueKey];
                if (onTimeValue && (onTimeValue.unsignedIntValue == testOnTimeValue)) {
                    [onTimeWriteSuccess fulfill];
                }

                NSDictionary * previousDataValue = attributeResponseValue[MTRPreviousDataKey];
                NSNumber * previousOnTimeValue = previousDataValue[MTRValueKey];
                if (previousOnTimeValue) {
                    [onTimePreviousValue fulfill];
                }
            }
        }
    };
    __auto_type * writeOnTimeValue = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(testOnTimeValue) };

    // Also set up a few attribute value waits to see what happens.
    __auto_type * onTimeValueToWaitFor = @{
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(MTRClusterIDTypeOnOffID)
                                          attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)] : writeOnTimeValue,
    };

    XCTestExpectation * waitingForOnTimeValue1Expectation = [self expectationWithDescription:@"OnTime value is now the expected value"];
    __auto_type * waiter4 = [device waitForAttributeValues:onTimeValueToWaitFor timeout:200 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [waitingForOnTimeValue1Expectation fulfill];
    }];
    XCTAssertNotNil(waiter4);

    XCTestExpectation * waitingForOnTimeValue2Expectation = [self expectationWithDescription:@"OnTime value is now the expected value and first device type is the expected value"];
    __auto_type * onTimeAndDeviceTypeValuesToWaitFor = [NSMutableDictionary dictionaryWithDictionary:onTimeValueToWaitFor];
    onTimeAndDeviceTypeValuesToWaitFor[deviceTypes[0][MTRAttributePathKey]] = deviceTypes[0][MTRDataKey];

    __auto_type * waiter5 = [device waitForAttributeValues:onTimeAndDeviceTypeValuesToWaitFor timeout:200 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNil(error);
        [waitingForOnTimeValue2Expectation fulfill];
    }];
    XCTAssertNotNil(waiter5);

    XCTestExpectation * waitingForOnTimeValue3Expectation = [self expectationWithDescription:@"OnTime value is now the expected value and first device type is bogus, or we timed out"];
    __auto_type * onTimeAndBogusDeviceTypeValuesToWaitFor = [NSMutableDictionary dictionaryWithDictionary:onTimeValueToWaitFor];
    [onTimeAndBogusDeviceTypeValuesToWaitFor addEntriesFromDictionary:bogusDeviceType];
    __auto_type * waiter6 = [device waitForAttributeValues:onTimeAndBogusDeviceTypeValuesToWaitFor timeout:0.5 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNotNil(error);
        XCTAssertEqual(error.domain, MTRErrorDomain);
        XCTAssertEqual(error.code, MTRErrorCodeTimeout);
        [waitingForOnTimeValue3Expectation fulfill];
    }];
    XCTAssertNotNil(waiter6);

    XCTestExpectation * waitingForOnTimeValue4Expectation = [self expectationWithDescription:@"Waiter should have been canceled"];
    __auto_type * waiter7 = [device waitForAttributeValues:onTimeValueToWaitFor timeout:200 queue:queue completion:^(NSError * _Nullable error) {
        XCTAssertNotNil(error);
        XCTAssertEqual(error.domain, MTRErrorDomain);
        XCTAssertEqual(error.code, MTRErrorCodeCancelled);
        [waitingForOnTimeValue4Expectation fulfill];
    }];
    XCTAssertNotNil(waiter7);
    [waiter7 cancel];

    XCTestExpectation * waitingForOnTimeValue5Expectation = [self expectationWithDescription:@"Waiter should have been canceled due to being destroyed"];
    @autoreleasepool {
        // To force the waiter created here to die quickly.
        [device waitForAttributeValues:onTimeValueToWaitFor timeout:200 queue:queue completion:^(NSError * _Nullable error) {
            XCTAssertNotNil(error);
            XCTAssertEqual(error.domain, MTRErrorDomain);
            XCTAssertEqual(error.code, MTRErrorCodeCancelled);
            [waitingForOnTimeValue5Expectation fulfill];
        }];
    }

    [device writeAttributeWithEndpointID:@(1)
                               clusterID:@(MTRClusterIDTypeOnOffID)
                             attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)
                                   value:writeOnTimeValue
                   expectedValueInterval:@(10000)
                       timedWriteTimeout:nil];

    [self waitForExpectations:@[
        onTimeWriteSuccess,
        onTimePreviousValue,
        waitingForOnTimeValue1Expectation,
        waitingForOnTimeValue2Expectation,
        waitingForOnTimeValue3Expectation,
        waitingForOnTimeValue4Expectation,
        waitingForOnTimeValue5Expectation,
    ]
                      timeout:10];

    __auto_type getOnOffValue = ^{
        return [device readAttributeWithEndpointID:@(1)
                                         clusterID:@(MTRClusterIDTypeOnOffID)
                                       attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnOffID)
                                            params:nil];
    };
    __auto_type * onOffValue = getOnOffValue();

    [device unitTestClearClusterData];

    // Test that we can still get the value (will get paged in from storage).
    XCTAssertEqualObjects(getOnOffValue(), onOffValue);

    // Test if errors are properly received
    // TODO: We might stop reporting these altogether from MTRDevice, and then
    // this test will need updating.
    __auto_type * readThroughForUnknownAttributesParams = [[MTRReadParams alloc] init];
    readThroughForUnknownAttributesParams.assumeUnknownAttributesReportable = NO;
    XCTestExpectation * attributeReportErrorExpectation = [self expectationWithDescription:@"Attribute read error"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * attributeResponseValue in data) {
            if (attributeResponseValue[MTRErrorKey]) {
                [attributeReportErrorExpectation fulfill];
            }
        }
    };

    // use the nonexistent attribute and expect read error
    [device readAttributeWithEndpointID:testEndpointID clusterID:testClusterID attributeID:testAttributeID params:readThroughForUnknownAttributesParams];
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

    // Make sure there are no device configuration changed notifications during this test.
    // There is nothing changing about the server that would lead to those.
    __block BOOL wasOnDeviceConfigurationChangedCallbackCalled = NO;
    delegate.onDeviceConfigurationChanged = ^() {
        wasOnDeviceConfigurationChangedCallbackCalled = YES;
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

    XCTAssertFalse(wasOnDeviceConfigurationChangedCallbackCalled);

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

    // Now test doing the UpdateFabricLabel command but directly via the
    // MTRDevice API.
    XCTestExpectation * updateLabelExpectation2 = [self expectationWithDescription:@"Fabric label updated a second time"];
    // IMPORTANT: commandFields here uses hardcoded strings, not MTR* constants
    // for the strings, to check for places that are doing string equality wrong.
    __auto_type * commandFields = @{
        @"type" : @"Structure",
        @"value" : @[
            @{
                @"contextTag" : @0,
                @"data" : @ {
                    @"type" : @"UTF8String",
                    @"value" : @"Test2",
                },
            },
        ],
    };

    [device invokeCommandWithEndpointID:@(0)
                              clusterID:@(MTRClusterIDTypeOperationalCredentialsID)
                              commandID:@(MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID)
                          commandFields:commandFields
                         expectedValues:nil
                  expectedValueInterval:nil
                                  queue:queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 XCTAssertNil(error);
                                 [updateLabelExpectation2 fulfill];
                             }];

    [self waitForExpectations:@[ updateLabelExpectation2 ] timeout:kTimeoutInSeconds];

    // And again, make sure our fabric label got updated.
    readFabricLabelExpectation = [self expectationWithDescription:@"Read fabric label third time"];
    [baseOpCredsCluster readAttributeFabricsWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        XCTAssertEqual(value.count, 1);
        MTROperationalCredentialsClusterFabricDescriptorStruct * entry = value[0];
        XCTAssertEqualObjects(entry.label, @"Test2");
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

    // Get the list of endpoints from the device.  Endpoint 0 we have to add explicitly, since it's
    // not in its own PartsList.
    XCTestExpectation * descriptorReadExpectation = [self expectationWithDescription:@"read PartsList from endpoint 0"];
    __auto_type * descriptorCluster = [[MTRBaseClusterDescriptor alloc] initWithDevice:device endpointID:@(0) queue:queue];
    __block NSMutableArray<NSNumber *> * endpointList = [NSMutableArray arrayWithObject:@0];
    [descriptorCluster readAttributePartsListWithCompletion:^(NSArray<NSNumber *> * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        [endpointList addObjectsFromArray:value];
        [descriptorReadExpectation fulfill];
    }];

    [self waitForExpectations:@[ descriptorReadExpectation ] timeout:kTimeoutInSeconds];

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
                                    XCTAssertTrue([endpointList containsObject:path.endpoint]);
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
                        // We have a descriptor on each endpoint, so that's 4 results per endpoint,
                        // and we only have Basic Information on endpoint 0, so that's 4 more
                        // results.
                        XCTAssertEqual(attributeResultCount, endpointList.count * 4 + 4);
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
    __block NSMutableArray<NSNumber *> * endpointList = [NSMutableArray arrayWithObject:@0];
    [descriptorCluster readAttributePartsListWithCompletion:^(NSArray<NSNumber *> * _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(value);
        [endpointList addObjectsFromArray:value];
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
                        XCTAssertEqual(resultArray.count, endpointList.count * 4 + 4);

                        for (NSDictionary * result in resultArray) {
                            MTRAttributePath * path = result[@"attributePath"];
                            if ([path.attribute unsignedIntegerValue] < 4) {
                                XCTAssertEqualObjects(path.cluster, @29);
                                XCTAssertTrue([endpointList containsObject:path.endpoint]);
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
    params.resubscribeAutomatically = NO;

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
    params.resubscribeAutomatically = NO;

    NSArray<MTREventRequestPath *> * eventPaths = @[
        // Startup event.
        [MTREventRequestPath requestPathWithEndpointID:@0 clusterID:@40 eventID:@0],
        // Shutdown event.
        [MTREventRequestPath requestPathWithEndpointID:@0 clusterID:@40 eventID:@1],
    ];

    XCTestExpectation * startupEventExpectation = [self expectationWithDescription:@"report startup event"];
    __block __auto_type reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
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
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            if (reportHandler != nil) {
                reportHandler(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe complete");
            [expectation fulfill];
        }
        resubscriptionScheduled:nil];

    // Wait till establishment
    [self waitForExpectations:@[ startupEventExpectation, expectation ] timeout:kTimeoutInSeconds];

    // Null out reportHandler, so we don't notify it when the
    // subscription tears down.
    reportHandler = nil;
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

- (void)test029_MTRDeviceWriteCoalescing
{
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Given reachable state becomes true before underlying OnSubscriptionEstablished callback, this expectation is necessary but
    // not sufficient as a mark to the end of reports
    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onReportEnd = ^() {
        [gotReportsExpectation fulfill];
    };
    // Skip reports for expected values so we actually have some idea of what
    // the server is reporting.
    delegate.skipExpectedValuesForWrite = YES;

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ gotReportsExpectation ] timeout:60];

    delegate.onReportEnd = nil;

    uint16_t testOnTimeValue = 10;
    XCTestExpectation * onTimeWriteSuccess = [self expectationWithDescription:@"OnTime write success"];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        for (NSDictionary<NSString *, id> * attributeResponseValue in data) {
            MTRAttributePath * path = attributeResponseValue[MTRAttributePathKey];
            if (path.cluster.unsignedIntValue == MTRClusterIDTypeOnOffID && path.attribute.unsignedLongValue == MTRAttributeIDTypeClusterOnOffAttributeOnTimeID) {
                NSDictionary * dataValue = attributeResponseValue[MTRDataKey];
                NSNumber * onTimeValue = dataValue[MTRValueKey];
                if ([onTimeValue isEqual:@(testOnTimeValue + 4)]) {
                    [onTimeWriteSuccess fulfill];
                } else {
                    // The first write we did might get reported, but none of
                    // the other ones should be.
                    XCTAssertEqualObjects(onTimeValue, @(testOnTimeValue + 1));
                }
            }
        }
    };

    __auto_type writeOnTimeValue = ^(uint16_t value) {
        NSDictionary * writeValue = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(value) };
        [device writeAttributeWithEndpointID:@(1)
                                   clusterID:@(MTRClusterIDTypeOnOffID)
                                 attributeID:@(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID)
                                       value:writeValue
                       expectedValueInterval:@(0)
                           timedWriteTimeout:nil];
    };

    writeOnTimeValue(testOnTimeValue + 1);
    writeOnTimeValue(testOnTimeValue + 2);
    writeOnTimeValue(testOnTimeValue + 3);
    writeOnTimeValue(testOnTimeValue + 4);

    [self waitForExpectations:@[ onTimeWriteSuccess ] timeout:10];
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

- (void)test031_MTRDeviceAttributeCacheLocalTestStorage
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Get the subscription primed
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];

    NSTimeInterval baseTestDelayTime = 1;
    MTRDeviceStorageBehaviorConfiguration * config = [MTRDeviceStorageBehaviorConfiguration
        configurationWithReportToPersistenceDelayTime:baseTestDelayTime
                      reportToPersistenceDelayTimeMax:baseTestDelayTime * 2
                            recentReportTimesMaxCount:5
                  timeBetweenReportsTooShortThreshold:baseTestDelayTime * 0.4
               timeBetweenReportsTooShortMinThreshold:baseTestDelayTime * 0.2
                reportToPersistenceDelayMaxMultiplier:baseTestDelayTime * 5
          deviceReportingExcessivelyIntervalThreshold:baseTestDelayTime * 10];
    [device setStorageBehaviorConfiguration:config];

    XCTestExpectation * gotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received"];
    XCTestExpectation * gotDeviceCachePrimed = [self expectationWithDescription:@"Device cache primed for the first time"];
    XCTestExpectation * gotClusterDataPersisted1 = [self expectationWithDescription:@"Cluster data persisted 1"];
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
    delegate.onClusterDataPersisted = ^{
        [gotClusterDataPersisted1 fulfill];
    };
    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ gotReportsExpectation, gotDeviceCachePrimed, gotClusterDataPersisted1 ] timeout:60];

    NSUInteger attributesReportedWithFirstSubscription = [device unitTestAttributesReportedSinceLastCheck];

    NSDictionary * dataStoreClusterDataAfterFirstSubscription = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertTrue(dataStoreClusterDataAfterFirstSubscription.count > 0);

    // Now remove device, resubscribe, and see that it succeeds
    [sController removeDevice:device];
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];

    XCTestExpectation * resubGotReportsExpectation = [self expectationWithDescription:@"Attribute and Event reports have been received for resubscription"];
    XCTestExpectation * gotClusterDataPersisted2 = [self expectationWithDescription:@"Cluster data persisted 2"];
    delegate.onReportEnd = ^{
        [resubGotReportsExpectation fulfill];
        __strong __auto_type strongDelegate = weakDelegate;
        strongDelegate.onReportEnd = nil;
    };
    __block BOOL onDeviceCachePrimedCalled = NO;
    delegate.onDeviceCachePrimed = ^{
        onDeviceCachePrimedCalled = YES;
    };
    delegate.onClusterDataPersisted = ^{
        [gotClusterDataPersisted2 fulfill];
    };
    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ resubGotReportsExpectation, gotClusterDataPersisted2 ] timeout:60];

    // Make sure that the new callback is only ever called once, the first time subscription was primed
    XCTAssertFalse(onDeviceCachePrimedCalled);

    NSUInteger attributesReportedWithSecondSubscription = [device unitTestAttributesReportedSinceLastCheck];

    XCTAssertTrue(attributesReportedWithSecondSubscription < attributesReportedWithFirstSubscription);

    // 1) MTRDevice actually gets some attributes reported more than once
    // 2) Some attributes do change on resubscribe
    //   * With all-clusts-app as of 2024-02-10, out of 1287 persisted attributes, still 450 attributes were reported with filter
    // And so conservatively, assert that data version filters save at least 300 entries.
    NSDictionary * storedClusterDataAfterSecondSubscription = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    NSUInteger dataStoreAttributeCountAfterSecondSubscription = 0;
    for (NSNumber * clusterID in storedClusterDataAfterSecondSubscription) {
        MTRDeviceClusterData * clusterData = storedClusterDataAfterSecondSubscription[clusterID];
        dataStoreAttributeCountAfterSecondSubscription += clusterData.attributes.count;
    }
    NSUInteger storedAttributeCountDifferenceFromMTRDeviceReport = dataStoreAttributeCountAfterSecondSubscription - attributesReportedWithSecondSubscription;
    XCTAssertTrue(storedAttributeCountDifferenceFromMTRDeviceReport > 300);
}

- (void)doEncodeDecodeRoundTrip:(id<NSSecureCoding>)encodable
{
    // We know all our encodables are in fact NSObject.
    NSObject * obj = (NSObject *) encodable;

    NSError * encodeError;
    NSData * encodedData = [NSKeyedArchiver archivedDataWithRootObject:encodable requiringSecureCoding:YES error:&encodeError];
    XCTAssertNil(encodeError, @"Failed to encode %@", NSStringFromClass(obj.class));

    NSError * decodeError;
    id decodedValue = [NSKeyedUnarchiver unarchivedObjectOfClasses:[NSSet setWithObject:obj.class] fromData:encodedData error:&decodeError];
    XCTAssertNil(decodeError, @"Failed to decode %@", NSStringFromClass([obj class]));
    XCTAssertTrue([decodedValue isKindOfClass:obj.class], @"Expected %@ but got %@", NSStringFromClass(obj.class), NSStringFromClass([decodedValue class]));

    XCTAssertEqualObjects(obj, decodedValue, @"Decoding for %@ did not round-trip correctly", NSStringFromClass([obj class]));
}

- (void)test032_MTRPathClassesEncoding
{
    // Test attribute path encode / decode
    MTRAttributePath * attributePath = [MTRAttributePath attributePathWithEndpointID:@(101) clusterID:@(102) attributeID:@(103)];
    [self doEncodeDecodeRoundTrip:attributePath];

    // Test event path encode / decode
    MTREventPath * eventPath = [MTREventPath eventPathWithEndpointID:@(201) clusterID:@(202) eventID:@(203)];
    [self doEncodeDecodeRoundTrip:eventPath];

    // Test command path encode / decode
    MTRCommandPath * commandPath = [MTRCommandPath commandPathWithEndpointID:@(301) clusterID:@(302) commandID:@(303)];
    [self doEncodeDecodeRoundTrip:commandPath];

    // Test attribute request path encode/decode
    MTRAttributeRequestPath * attributeRequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:nil attributeID:nil];
    [self doEncodeDecodeRoundTrip:attributeRequestPath];

    attributeRequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:@(101) clusterID:@(102) attributeID:@(103)];
    [self doEncodeDecodeRoundTrip:attributeRequestPath];

    attributeRequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:nil clusterID:@(105) attributeID:@(106)];
    [self doEncodeDecodeRoundTrip:attributeRequestPath];

    attributeRequestPath = [MTRAttributeRequestPath requestPathWithEndpointID:@(107) clusterID:nil attributeID:@(109)];
    [self doEncodeDecodeRoundTrip:attributeRequestPath];
}

// Helper API to test if changes in an attribute with a path specified by endpointId, clusterId and attributeId trigger
// device configuration changed callbacks for a given MTRDevice. This API creates a fake attribute report for the given attribute path
// and injects it into MTRDevice to exercise and test the delegate's device configuration changed callback.
- (void)checkAttributeReportTriggersConfigurationChanged:(MTRAttributeIDType)attributeId
                                               clusterId:(MTRClusterIDType)clusterId
                                              endpointId:(NSNumber *)endpointId
                                                  device:(MTRDevice *)device
                                                delegate:(MTRDeviceTestDelegate *)delegate
                                             dataVersion:(NSNumber *)dataVersion
                                         attributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
                                             description:(NSString *)description
                              expectConfigurationChanged:(BOOL)expectConfigurationChanged
{
    XCTAssertNotNil(endpointId, @"%@", description);
    XCTAssertNotNil(device, @"%@", description);
    XCTAssertNotNil(delegate, @"%@", description);
    XCTAssertNotNil(dataVersion, @"%@", description);
    XCTAssertNotNil(attributeReport, @"%@", description);

    XCTestExpectation * gotAttributeReportExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"Attribute report has been received (%@)", description]];
    XCTestExpectation * gotAttributeReportEndExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"Attribute report has ended (%@)", description]];
    XCTestExpectation * deviceConfigurationChangedExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"Device configuration changed was received (%@)", description]];
    deviceConfigurationChangedExpectation.inverted = !expectConfigurationChanged;

    __block unsigned attributeReportsReceived = 0;
    __block id testDataValue = nil;

    for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
        MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
        if (attributePath.attribute.unsignedLongLongValue == attributeId) {
            NSDictionary * data = attributeDict[MTRDataKey];
            XCTAssertNotNil(data, @"%@", description);
            testDataValue = data[MTRValueKey];
        }
    }

    // Check if the received attribute report matches the injected attribute report.
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        NSLog(@"checkAttributeReportTriggersConfigurationChanged: onAttributeDataReceived called");
        attributeReportsReceived += attributeReport.count;
        XCTAssert(attributeReportsReceived > 0, @"%@", description);
        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath, @"%@", description);

            XCTAssertEqualObjects(attributePath.cluster, @(clusterId), @"%@", description);
            XCTAssertEqualObjects(attributePath.attribute, @(attributeId), @"%@", description);

            NSDictionary * data = attributeDict[MTRDataKey];
            XCTAssertNotNil(data, @"%@", description);
            XCTAssertEqualObjects(data[MTRDataVersionKey], dataVersion, @"%@", description);

            // This code assumes that none of the attributes in the report can have null values.
            // Since we are injecting the attribute report for testing this with non-null values,
            // we are fine for now. But if we plan to inject attribute reports with attributes having
            // null values, we need to fix the code accordingly.
            id dataValue = data[MTRValueKey];
            XCTAssertNotNil(dataValue, @"%@", description);
            XCTAssertNotNil(testDataValue, @"%@", description);
            XCTAssertEqualObjects(dataValue, testDataValue, @"%@", description);
            [gotAttributeReportExpectation fulfill];
        }
    };

    delegate.onReportEnd = ^() {
        NSLog(@"checkAttributeReportTriggersConfigurationChanged: onReportEnd called");
        [gotAttributeReportEndExpectation fulfill];
    };

    __block BOOL wasOnDeviceConfigurationChangedCallbackCalled = NO;

    delegate.onDeviceConfigurationChanged = ^() {
        NSLog(@"checkAttributeReportTriggersConfigurationChanged: onDeviceConfigurationChanged called");
        [deviceConfigurationChangedExpectation fulfill];
        wasOnDeviceConfigurationChangedCallbackCalled = YES;
    };

    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ gotAttributeReportExpectation, gotAttributeReportEndExpectation, deviceConfigurationChangedExpectation ] timeout:kTimeoutInSeconds];
    if (!expectConfigurationChanged) {
        XCTAssertFalse(wasOnDeviceConfigurationChangedCallbackCalled, @"%@", description);
    }
}

- (void)test033_TestMTRDeviceDeviceConfigurationChanged
{
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Check if subscription is set up and initial reports are received.
    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];
    XCTestExpectation * gotInitialReportsExpectation = [self expectationWithDescription:@"Initial Attribute and Event reports have been received"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onReachable = ^() {
        [subscriptionExpectation fulfill];
    };

    __block unsigned attributeReportsReceived = 0;

    // Get the data version for the following clusters that we will use for the tests - Descriptor cluster on endpoint 0,
    // Identify cluster, On/Off cluster, Oven Cavity Operational State cluster and Groups cluster.
    __block NSNumber * dataVersionForDescriptor;
    __block NSMutableArray<NSDictionary<NSString *, id> *> * valueForPartsList;
    const uint32_t kDescriptorEndpoint = 0;

    __block NSNumber * dataVersionForOvenCavityOperationalState;
    __block NSNumber * endpointForOvenCavityOperationalStateAcceptedCommandList;

    __block NSNumber * dataVersionForIdentify;
    __block NSNumber * endpointForIdentifyAttributeList;

    __block NSNumber * dataVersionForOnOff;
    __block NSNumber * endpointForOnOffClusterRevision;

    __block NSNumber * dataVersionForGroups;
    __block NSNumber * endpointForGroupsFeatureMap;

    __block NSNumber * dataVersionForPowerSourceConfiguration;
    __block NSNumber * endpointForPowerSourceConfigurationSources;

    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        NSLog(@"test033_TestMTRDeviceDeviceConfigurationChanged: onAttributeDataReceived called");
        attributeReportsReceived += attributeReport.count;
        XCTAssert(attributeReportsReceived > 0);

        for (NSDictionary<NSString *, id> * attributeDict in attributeReport) {
            MTRAttributePath * attributePath = attributeDict[MTRAttributePathKey];
            XCTAssertNotNil(attributePath);

            if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeDescriptorID && attributePath.endpoint.unsignedLongValue == kDescriptorEndpoint) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForDescriptor = data[MTRDataVersionKey];

                switch (attributePath.attribute.unsignedLongValue) {
                case MTRAttributeIDTypeClusterDescriptorAttributePartsListID: {
                    valueForPartsList = [data[MTRValueKey] mutableCopy];
                    break;
                }
                }
            } else if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeIdentifyID) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                switch (attributePath.attribute.unsignedLongValue) {
                case MTRAttributeIDTypeGlobalAttributeAttributeListID: {
                    dataVersionForIdentify = data[MTRDataVersionKey];
                    endpointForIdentifyAttributeList = attributePath.endpoint;
                    break;
                }
                }
            } else if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeOnOffID) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                switch (attributePath.attribute.unsignedLongValue) {
                case MTRAttributeIDTypeGlobalAttributeClusterRevisionID: {
                    dataVersionForOnOff = data[MTRDataVersionKey];
                    endpointForOnOffClusterRevision = attributePath.endpoint;
                    break;
                }
                }
            } else if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeOvenCavityOperationalStateID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForOvenCavityOperationalState = data[MTRDataVersionKey];
                endpointForOvenCavityOperationalStateAcceptedCommandList = attributePath.endpoint;
            } else if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypeGroupsID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeGlobalAttributeFeatureMapID) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForGroups = data[MTRDataVersionKey];
                endpointForGroupsFeatureMap = attributePath.endpoint;
            } else if (attributePath.cluster.unsignedLongValue == MTRClusterIDTypePowerSourceConfigurationID && attributePath.attribute.unsignedLongValue == MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID) {
                NSDictionary * data = attributeDict[MTRDataKey];
                XCTAssertNotNil(data);
                dataVersionForPowerSourceConfiguration = data[MTRDataVersionKey];
                endpointForPowerSourceConfigurationSources = attributePath.endpoint;
            }
        }
    };

    delegate.onReportEnd = ^() {
        NSLog(@"test033_TestMTRDeviceDeviceConfigurationChanged: onReportEnd called");
        XCTAssertNotNil(dataVersionForDescriptor);
        XCTAssertNotNil(dataVersionForOvenCavityOperationalState);
        XCTAssertNotNil(dataVersionForIdentify);
        XCTAssertNotNil(dataVersionForOnOff);
        XCTAssertNotNil(dataVersionForGroups);
        XCTAssertNotNil(dataVersionForPowerSourceConfiguration);
        [gotInitialReportsExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    // Wait for subscription set up and initial reports received.
    [self waitForExpectations:@[
        subscriptionExpectation,
        gotInitialReportsExpectation,
    ]
                      timeout:60];

    // The multiple tests below do the following:
    // 1. Set the test data version to the data version for the attribute path saved from the initial attribute report incremented by 1.
    // 2. Create the fake data for the attribute report for the attribute.
    // 3. Call the helper checkAttributeReportTriggersConfigurationChanged to test if the attribute report affects device configuration changes.

    // Test attribute path - endpointId = 0, clusterId = descriptor, attributeId = parts list.
    dataVersionForDescriptor = @(dataVersionForDescriptor.unsignedLongLongValue + 1);
    // Figure out an endpoint ID (not 0) we can add to PartsList.
    for (unsigned i = 1; true; ++i) {
        __auto_type unsignedIntegerValue = @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(i),
            }
        };

        if (![valueForPartsList containsObject:unsignedIntegerValue]) {
            [valueForPartsList addObject:unsignedIntegerValue];
            break;
        }
    }
    NSArray<NSDictionary<NSString *, id> *> * attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(kDescriptorEndpoint) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForDescriptor,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : valueForPartsList,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeClusterDescriptorAttributePartsListID clusterId:MTRClusterIDTypeDescriptorID endpointId:@(kDescriptorEndpoint) device:device delegate:delegate dataVersion:dataVersionForDescriptor attributeReport:attributeReport description:@"Descriptor PartsList" expectConfigurationChanged:YES];

    // Test attribute path - endpointId = 0, clusterId = descriptor, attributeId = device types list.
    dataVersionForDescriptor = @(dataVersionForDescriptor.unsignedLongLongValue + 1);
    NSArray<NSDictionary<NSString *, id> *> * deviceTypesListValue = @[
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @0,
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @1
                        },
                    },
                    @{
                        MTRContextTagKey : @1,
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @2
                        },
                    },
                ],
            },
        },
    ];

    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(kDescriptorEndpoint) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForDescriptor,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : deviceTypesListValue,
        }
    } ];

    // unsignedIntegerArrayValue is used for a variety of tests below.
    NSArray<NSDictionary<NSString *, id> *> * unsignedIntegerArrayValue = @[
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @1,
            }
        },
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @2,
            }
        },
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @3,
            }
        },
    ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID clusterId:MTRClusterIDTypeDescriptorID endpointId:@(kDescriptorEndpoint) device:device delegate:delegate dataVersion:dataVersionForDescriptor attributeReport:attributeReport description:@"Descriptor DeviceTypeList" expectConfigurationChanged:YES];

    // Test attribute path - endpointId = 0, clusterId = descriptor, attributeId = server list.
    dataVersionForDescriptor = @(dataVersionForDescriptor.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(kDescriptorEndpoint) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributeServerListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForDescriptor,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : unsignedIntegerArrayValue,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeClusterDescriptorAttributeServerListID clusterId:MTRClusterIDTypeDescriptorID endpointId:@(kDescriptorEndpoint) device:device delegate:delegate dataVersion:dataVersionForDescriptor attributeReport:attributeReport description:@"Descriptor ServerList" expectConfigurationChanged:YES];

    // Test attribute path - clusterId = ovencavityoperationalstateID, attributeId = accepted command list.
    dataVersionForOvenCavityOperationalState = @(dataVersionForOvenCavityOperationalState.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForOvenCavityOperationalStateAcceptedCommandList clusterID:@(MTRClusterIDTypeOvenCavityOperationalStateID) attributeID:@(MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForOvenCavityOperationalState,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : unsignedIntegerArrayValue,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID clusterId:MTRClusterIDTypeOvenCavityOperationalStateID endpointId:endpointForOvenCavityOperationalStateAcceptedCommandList device:device delegate:delegate dataVersion:dataVersionForOvenCavityOperationalState attributeReport:attributeReport description:@"OvenCavityOperationalState AcceptedCommandList" expectConfigurationChanged:YES];

    // Test attribute path - clusterId = identify, attributeId = attribute list.
    dataVersionForIdentify = @(dataVersionForIdentify.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForIdentifyAttributeList clusterID:@(MTRClusterIDTypeIdentifyID) attributeID:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForIdentify,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : unsignedIntegerArrayValue,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeGlobalAttributeAttributeListID clusterId:MTRClusterIDTypeIdentifyID endpointId:endpointForIdentifyAttributeList device:device delegate:delegate dataVersion:dataVersionForIdentify attributeReport:attributeReport description:@"Identify AttributeList" expectConfigurationChanged:YES];

    // Test attribute path - clusterId = OnOff, attributeId = cluster revision.
    dataVersionForOnOff = @(dataVersionForOnOff.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForOnOffClusterRevision clusterID:@(MTRClusterIDTypeIdentifyID) attributeID:@(MTRAttributeIDTypeGlobalAttributeClusterRevisionID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForOnOff,
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @9999,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeGlobalAttributeClusterRevisionID clusterId:MTRClusterIDTypeIdentifyID endpointId:endpointForOnOffClusterRevision device:device delegate:delegate dataVersion:dataVersionForOnOff attributeReport:attributeReport description:@"OnOff ClusterRevision" expectConfigurationChanged:YES];

    // Test attribute path - clusterId = groupsID, attributeId = feature map.
    dataVersionForGroups = @(dataVersionForGroups.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForGroupsFeatureMap clusterID:@(MTRClusterIDTypeGroupsID) attributeID:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForGroups,
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @2,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeGlobalAttributeFeatureMapID clusterId:MTRClusterIDTypeGroupsID endpointId:endpointForGroupsFeatureMap device:device delegate:delegate dataVersion:dataVersionForGroups attributeReport:attributeReport description:@"Groups FeatureMap" expectConfigurationChanged:YES];

    // Test attribute path that doesn't cause a device configuration clusterId = power source configuration, attributeId = sources.
    dataVersionForPowerSourceConfiguration = @(dataVersionForPowerSourceConfiguration.unsignedLongLongValue + 1);
    attributeReport = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForPowerSourceConfigurationSources clusterID:@(MTRClusterIDTypePowerSourceConfigurationID) attributeID:@(MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID)],
        MTRDataKey : @ {
            MTRDataVersionKey : dataVersionForPowerSourceConfiguration,
            MTRTypeKey : MTRArrayValueType,
            MTRValueKey : unsignedIntegerArrayValue,
        }
    } ];

    [self checkAttributeReportTriggersConfigurationChanged:MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID clusterId:MTRClusterIDTypePowerSourceConfigurationID endpointId:endpointForPowerSourceConfigurationSources device:device delegate:delegate dataVersion:dataVersionForPowerSourceConfiguration attributeReport:attributeReport description:@"PowerSourceConfiguration Sources" expectConfigurationChanged:NO];

    NSArray<NSDictionary<NSString *, id> *> * newUnsignedIntegerArrayValue = @[
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @1,
            }
        },
        @{
            MTRDataKey : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @2,
            }
        }
    ];

    // Test an attribute report with multiple attributes at least one of which triggers device configuration changed.
    dataVersionForIdentify = @(dataVersionForIdentify.unsignedLongLongValue + 1);
    dataVersionForGroups = @(dataVersionForGroups.unsignedLongLongValue + 1);
    dataVersionForPowerSourceConfiguration = @(dataVersionForPowerSourceConfiguration.unsignedLongLongValue + 1);
    attributeReport = @[
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForIdentifyAttributeList clusterID:@(MTRClusterIDTypeIdentifyID) attributeID:@(MTRAttributeIDTypeGlobalAttributeAttributeListID)],
            MTRDataKey : @ {
                MTRDataVersionKey : dataVersionForIdentify,
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : unsignedIntegerArrayValue,
            }
        },
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForGroupsFeatureMap clusterID:@(MTRClusterIDTypeGroupsID) attributeID:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)],
            MTRDataKey : @ {
                MTRDataVersionKey : dataVersionForGroups,
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @3,
            }
        },
        @{
            MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointForPowerSourceConfigurationSources clusterID:@(MTRClusterIDTypePowerSourceConfigurationID) attributeID:@(MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID)],
            MTRDataKey : @ {
                MTRDataVersionKey : dataVersionForPowerSourceConfiguration,
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : newUnsignedIntegerArrayValue,
            }
        }
    ];

    XCTestExpectation * gotAttributeReportWithMultipleAttributesExpectation = [self expectationWithDescription:@"Attribute report with multiple attributes has been received"];
    XCTestExpectation * gotAttributeReportWithMultipleAttributesEndExpectation = [self expectationWithDescription:@"Attribute report with multiple attributes has ended"];
    XCTestExpectation * deviceConfigurationChangedExpectationForAttributeReportWithMultipleAttributes = [self expectationWithDescription:@"Device configuration changed was receieved due to an attribute report with multiple attributes "];
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        NSLog(@"test033_TestMTRDeviceDeviceConfigurationChanged: onAttributeDataReceived called with multiple attributes");
        attributeReportsReceived += attributeReport.count;
        XCTAssert(attributeReportsReceived > 0);
        [gotAttributeReportWithMultipleAttributesExpectation fulfill];
    };

    delegate.onReportEnd = ^() {
        NSLog(@"test033_TestMTRDeviceDeviceConfigurationChanged: onReportEnd called with multiple attributes");
        [gotAttributeReportWithMultipleAttributesEndExpectation fulfill];
    };

    delegate.onDeviceConfigurationChanged = ^() {
        NSLog(@"test033_TestMTRDeviceDeviceConfigurationChanged: onDeviceConfigurationChanged called for testing with multiple attributes");
        [deviceConfigurationChangedExpectationForAttributeReportWithMultipleAttributes fulfill];
    };

    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];
    [self waitForExpectations:@[ gotAttributeReportWithMultipleAttributesExpectation, gotAttributeReportWithMultipleAttributesEndExpectation, deviceConfigurationChangedExpectationForAttributeReportWithMultipleAttributes ] timeout:kTimeoutInSeconds];
}

- (void)test034_TestMTRDeviceHistoricalEvents
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    // Set up a subscription via mConnectedDevice that will send us continuous
    // reports.
    XCTestExpectation * firstSubscriptionExpectation = [self expectationWithDescription:@"First subscription established"];

    MTRSubscribeParams * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(0)];
    params.resubscribeAutomatically = NO;

    [mConnectedDevice subscribeToAttributesWithEndpointID:@(0)
        clusterID:@(MTRClusterIDTypeBasicInformationID)
        attributeID:@(0)
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
        }
        subscriptionEstablished:^() {
            [firstSubscriptionExpectation fulfill];
        }];

    [self waitForExpectations:@[ firstSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    // Now set up our MTRDevice and do a subscribe.  Make sure all the events we
    // get are marked "historical".
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    XCTestExpectation * secondSubscriptionExpectation = [self expectationWithDescription:@"Second subscription established"];
    XCTestExpectation * gotFirstReportsExpectation = [self expectationWithDescription:@"First Attribute and Event reports have been received"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onReachable = ^() {
        [secondSubscriptionExpectation fulfill];
    };

    __block unsigned eventReportsReceived = 0;
    delegate.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * eventReport) {
        eventReportsReceived += eventReport.count;
        for (NSDictionary<NSString *, id> * eventDict in eventReport) {
            NSNumber * reportIsHistorical = eventDict[MTREventIsHistoricalKey];
            XCTAssertTrue(reportIsHistorical.boolValue);
        }
    };

    delegate.onReportEnd = ^() {
        [gotFirstReportsExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ secondSubscriptionExpectation, gotFirstReportsExpectation ] timeout:60];

    // Must have gotten some events (at least StartUp!)
    XCTAssertTrue(eventReportsReceived > 0);

    // Remove the device, then try again, now with us having stored cluster
    // data.  All the events should still be reported as historical.
    [sController removeDevice:device];

    eventReportsReceived = 0;

    device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    XCTestExpectation * thirdSubscriptionExpectation = [self expectationWithDescription:@"Third subscription established"];
    XCTestExpectation * gotSecondReportsExpectation = [self expectationWithDescription:@"Second Attribute and Event reports have been received"];

    delegate.onReachable = ^() {
        [thirdSubscriptionExpectation fulfill];
    };

    delegate.onReportEnd = ^() {
        [gotSecondReportsExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ thirdSubscriptionExpectation, gotSecondReportsExpectation ] timeout:60];

    // Must have gotten some events (at least StartUp!)
    XCTAssertTrue(eventReportsReceived > 0);

    // Now try doing some event injection.
    __block uint64_t eventNumber = 0x1000000; // Should't have that many events on the device yet!
    __block uint64_t firstNewEventNumber = eventNumber;
    __auto_type createEventReport = ^{
        return @{
            MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(1) eventID:@(1)],
            MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
            MTREventTimestampDateKey : [NSDate date],
            MTREventPriorityKey : @(MTREventPriorityInfo),
            MTREventNumberKey : @(eventNumber++),
            MTRDataKey : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[],
            },
        };
    };

    eventReportsReceived = 0;
    const int eventReportsToInject = 5;
    __block XCTestExpectation * eventReportsFinishedExpectation = [self expectationWithDescription:@"Injecting some new event reports"];
    __block int historicalEvents = 0;
    __block int eventReportsToExpect = eventReportsToInject;

    delegate.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * eventReport) {
        eventReportsReceived += eventReport.count;
        for (NSDictionary<NSString *, id> * eventDict in eventReport) {
            NSNumber * reportIsHistorical = eventDict[MTREventIsHistoricalKey];
            NSNumber * eventNumber = eventDict[MTREventNumberKey];
            XCTAssertEqual(reportIsHistorical.boolValue, eventNumber.unsignedLongLongValue < firstNewEventNumber,
                @"eventNumber: %@, firstNewEventNumber: %llu", eventNumber, firstNewEventNumber);
            historicalEvents += reportIsHistorical.boolValue;
        }

        if (eventReportsReceived >= eventReportsToExpect) {
            [eventReportsFinishedExpectation fulfill];
        }
    };

    for (int i = 0; i < eventReportsToInject; ++i) {
        [device unitTestInjectEventReport:@[ createEventReport() ]];
    }

    [self waitForExpectations:@[ eventReportsFinishedExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertEqual(historicalEvents, 0);

    // Now inject some mix of historical and non-historical events.  Since this
    // includes events with old event numbers, those should get filtered out
    // from reporting.
    const uint64_t expectedHistoricalEvents = 2;
    firstNewEventNumber = eventNumber;
    eventNumber -= expectedHistoricalEvents;
    eventReportsReceived = 0;
    historicalEvents = 0;
    // Events with already-observed event numbers do not get reported.
    eventReportsToExpect = eventReportsToInject - expectedHistoricalEvents;
    eventReportsFinishedExpectation = [self expectationWithDescription:@"Injecting a mix of old and new reports"];

    for (int i = 0; i < eventReportsToInject; ++i) {
        [device unitTestInjectEventReport:@[ createEventReport() ]];
    }

    [self waitForExpectations:@[ eventReportsFinishedExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertEqual(historicalEvents, 0);
}

- (void)test035_TestMTRDeviceSubscriptionNotEstablishedOverXPC
{
    NSString * const MTRDeviceControllerId = @"MTRController";
    __auto_type remoteController = [MTRDeviceController
        sharedControllerWithID:MTRDeviceControllerId
               xpcConnectBlock:^NSXPCConnection * _Nonnull {
                   return nil;
               }];

    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:remoteController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];
    subscriptionExpectation.inverted = YES;

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];

    XCTAssertEqual([device _getInternalState], MTRInternalDeviceStateUnsubscribed);

    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * attributeReport) {
        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];
    [self waitForExpectations:@[ subscriptionExpectation ] timeout:5];

    XCTAssertEqual([device _getInternalState], MTRInternalDeviceStateUnsubscribed);
}

- (NSArray<NSDictionary<NSString *, id> *> *)_testAttributeReportWithValue:(unsigned int)testValue
{
    return [self _testAttributeReportWithValue:testValue dataVersion:testValue];
}

- (NSArray<NSDictionary<NSString *, id> *> *)_testAttributeReportWithValue:(unsigned int)testValue dataVersion:(unsigned int)dataVersion
{
    return @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeLevelControlID) attributeID:@(MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID)],
        MTRDataKey : @ {
            MTRDataVersionKey : @(dataVersion),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(testValue),
        }
    } ];
}

- (void)test036_TestStorageBehaviorConfiguration
{
    // Use separate queue for timing sensitive test
    dispatch_queue_t queue = dispatch_queue_create("storage-behavior-queue", DISPATCH_QUEUE_SERIAL);

    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];

    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    __block os_unfair_lock lock = OS_UNFAIR_LOCK_INIT;
    __block NSDate * reportEndTime = nil;
    __block NSDate * dataPersistedTime = nil;

    XCTestExpectation * dataPersistedInitial = [self expectationWithDescription:@"data persisted initial"];
    delegate.onReportEnd = ^() {
        os_unfair_lock_lock(&lock);
        if (!reportEndTime) {
            reportEndTime = [NSDate now];
        }
        os_unfair_lock_unlock(&lock);
    };

    delegate.onClusterDataPersisted = ^{
        os_unfair_lock_lock(&lock);
        if (!dataPersistedTime) {
            dataPersistedTime = [NSDate now];
        }
        os_unfair_lock_unlock(&lock);
        [dataPersistedInitial fulfill];
    };

    // Do not subscribe - only inject sequence of reports to control the timing
    delegate.skipSetupSubscription = YES;

    NSTimeInterval baseTestDelayTime = 3;

    // Set up a config of relatively short timers so this test doesn't take too long
    MTRDeviceStorageBehaviorConfiguration * config = [MTRDeviceStorageBehaviorConfiguration
        configurationWithReportToPersistenceDelayTime:baseTestDelayTime
                      reportToPersistenceDelayTimeMax:baseTestDelayTime * 2
                            recentReportTimesMaxCount:5
                  timeBetweenReportsTooShortThreshold:baseTestDelayTime * 0.4
               timeBetweenReportsTooShortMinThreshold:baseTestDelayTime * 0.2
                reportToPersistenceDelayMaxMultiplier:baseTestDelayTime * 5
          deviceReportingExcessivelyIntervalThreshold:baseTestDelayTime * 7];
    [device setStorageBehaviorConfiguration:config];

    [device setDelegate:delegate queue:queue];

    // Use a counter that will be incremented for each report as the value.
    __block unsigned int currentTestValue = 1;

    // Initial setup: Inject report and see that the attribute persisted.  No delay is
    // expected for the first (priming) report.
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    [self waitForExpectations:@[ dataPersistedInitial ] timeout:60];

    XCTestExpectation * dataPersisted1 = [self expectationWithDescription:@"data persisted 1"];
    delegate.onClusterDataPersisted = ^{
        os_unfair_lock_lock(&lock);
        if (!dataPersistedTime) {
            dataPersistedTime = [NSDate now];
        }
        os_unfair_lock_unlock(&lock);
        [dataPersisted1 fulfill];
    };

    // Test 1: Inject report and see that the attribute persisted, with a delay
    reportEndTime = nil;
    dataPersistedTime = nil;
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    [self waitForExpectations:@[ dataPersisted1 ] timeout:60];

    os_unfair_lock_lock(&lock);
    NSTimeInterval reportToPersistenceDelay = [dataPersistedTime timeIntervalSinceDate:reportEndTime];
    os_unfair_lock_unlock(&lock);
    // Check delay exists
    XCTAssertGreaterThan(reportToPersistenceDelay, baseTestDelayTime / 2);
    // Check delay is expectd - use base delay plus small fudge in case of CPU slowness with dispatch_after
    XCTAssertLessThan(reportToPersistenceDelay, baseTestDelayTime * 1.3);

    XCTestExpectation * dataPersisted2 = [self expectationWithDescription:@"data persisted 2"];

    delegate.onClusterDataPersisted = ^{
        os_unfair_lock_lock(&lock);
        if (!dataPersistedTime) {
            dataPersistedTime = [NSDate now];
        }
        os_unfair_lock_unlock(&lock);
        [dataPersisted2 fulfill];
    };

    // Test 2: Inject multiple reports with delay and see that the attribute persisted eventually
    reportEndTime = nil;
    dataPersistedTime = nil;
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    double frequentReportMultiplier = 0.5;
    usleep((useconds_t) (baseTestDelayTime * frequentReportMultiplier * USEC_PER_SEC));
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    usleep((useconds_t) (baseTestDelayTime * frequentReportMultiplier * USEC_PER_SEC));
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    usleep((useconds_t) (baseTestDelayTime * frequentReportMultiplier * USEC_PER_SEC));
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    usleep((useconds_t) (baseTestDelayTime * frequentReportMultiplier * USEC_PER_SEC));
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    // At this point, the threshold for reportToPersistenceDelayTimeMax should have hit, and persistence
    // should have happened with timer running down to persist again with the 5th report above. Need to
    // wait for expectation and immediately clear the onClusterDataPersisted callback

    [self waitForExpectations:@[ dataPersisted2 ] timeout:60];

    os_unfair_lock_lock(&lock);
    reportToPersistenceDelay = [dataPersistedTime timeIntervalSinceDate:reportEndTime];
    os_unfair_lock_unlock(&lock);
    // Check delay exists and approximately reportToPersistenceDelayTimeMax, which is base delay times 2
    XCTAssertGreaterThan(reportToPersistenceDelay, baseTestDelayTime * 2 * 0.9);
    XCTAssertLessThan(reportToPersistenceDelay, baseTestDelayTime * 2 * 1.3); // larger upper limit in case machine is slow

    delegate.onClusterDataPersisted = nil;

    // sleep the base delay interval to allow the onClusterDataPersisted callback to happen.
    usleep((useconds_t) (baseTestDelayTime * 1.1 * USEC_PER_SEC));

    // Test 3: test reporting frequently, and see that the delay time increased
    reportEndTime = nil;
    dataPersistedTime = nil;
    XCTestExpectation * dataPersisted3 = [self expectationWithDescription:@"data persisted 3"];
    delegate.onClusterDataPersisted = ^{
        os_unfair_lock_lock(&lock);
        if (!dataPersistedTime) {
            dataPersistedTime = [NSDate now];
        }
        os_unfair_lock_unlock(&lock);
        [dataPersisted3 fulfill];
    };

    // Set report times with short delay and check that the multiplier is engaged
    [device unitTestSetMostRecentReportTimes:[NSMutableArray arrayWithArray:@[
        [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.3 * 4)],
        [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.3 * 3)],
        [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.3 * 2)],
        [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.3)],
    ]]];

    // Inject final report that makes MTRDevice recalculate delay with multiplier
    [device unitTestInjectAttributeReport:[self _testAttributeReportWithValue:currentTestValue++] fromSubscription:YES];

    [self waitForExpectations:@[ dataPersisted3 ] timeout:60];

    // 0.3 is between 0.4 and 0.2, which should get us at least 50% of the multiplier.
    // The multiplier is 5, which is +400% of the base delay, and so 50% of the multiplier
    // is +200% of the base delay, meaning 3x the base delay.

    os_unfair_lock_lock(&lock);
    reportToPersistenceDelay = [dataPersistedTime timeIntervalSinceDate:reportEndTime];
    os_unfair_lock_unlock(&lock);
    // Check delay exists and at least base delay times 3
    XCTAssertGreaterThan(reportToPersistenceDelay, baseTestDelayTime * 3 * 0.9);
    // upper limit at most max delay times full multiplier + extra in case machine is slow
    XCTAssertLessThan(reportToPersistenceDelay, baseTestDelayTime * 2 * 5 * 1.3);

    // Test 4: test reporting excessively, and see that persistence does not happen until
    // reporting frequency goes back below the threshold
    __auto_type excessiveReportTest = ^(unsigned int testId, NSArray<NSDictionary<NSString *, id> *> * (^reportGenerator)(void), bool expectPersistence) {
        reportEndTime = nil;
        dataPersistedTime = nil;
        XCTestExpectation * dataPersisted = [self expectationWithDescription:[NSString stringWithFormat:@"data persisted %u", testId]];
        dataPersisted.inverted = !expectPersistence;
        delegate.onClusterDataPersisted = ^{
            os_unfair_lock_lock(&lock);
            if (!dataPersistedTime) {
                dataPersistedTime = [NSDate now];
            }
            os_unfair_lock_unlock(&lock);
            [dataPersisted fulfill];
        };

        // Set report times with short delay and check that the multiplier is engaged
        [device unitTestSetMostRecentReportTimes:[NSMutableArray arrayWithArray:@[
            [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.1 * 4)],
            [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.1 * 3)],
            [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.1 * 2)],
            [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 0.1)],
        ]]];

        // Inject report that makes MTRDevice detect the device is reporting excessively
        [device unitTestInjectAttributeReport:reportGenerator() fromSubscription:YES];

        // Now keep reporting excessively for base delay time max times max multiplier, plus a bit more
        NSDate * excessiveStartTime = [NSDate now];
        for (;;) {
            usleep((useconds_t) (baseTestDelayTime * 0.1 * USEC_PER_SEC));
            [device unitTestInjectAttributeReport:reportGenerator() fromSubscription:YES];
            NSTimeInterval elapsed = -[excessiveStartTime timeIntervalSinceNow];
            if (elapsed > (baseTestDelayTime * 2 * 5 * 1.2)) {
                break;
            }
        }

        // Check that persistence has not happened because it's now turned off
        XCTAssertNil(dataPersistedTime);

        // Now force report times to large number, to simulate time passage
        [device unitTestSetMostRecentReportTimes:[NSMutableArray arrayWithArray:@[
            [NSDate dateWithTimeIntervalSinceNow:-(baseTestDelayTime * 10)],
        ]]];

        // And inject a report to trigger MTRDevice to recalculate that this device is no longer
        // reporting excessively
        [device unitTestInjectAttributeReport:reportGenerator() fromSubscription:YES];

        [self waitForExpectations:@[ dataPersisted ] timeout:60];
    };

    excessiveReportTest(
        4, ^{
            return [self _testAttributeReportWithValue:currentTestValue++];
        }, true);

    // Test 5: test reporting excessively with the same value and different data
    // versions, and see that persistence does not happen until reporting
    // frequency goes back below the threshold.
    __block __auto_type dataVersion = currentTestValue;
    // We incremented currentTestValue after injecting the last report.  Make sure all the new
    // reports use that last-reported value.
    __auto_type lastReportedValue = currentTestValue - 1;
    excessiveReportTest(
        5, ^{
            return [self _testAttributeReportWithValue:lastReportedValue dataVersion:dataVersion++];
        }, true);

    // Test 6: test reporting excessively with the same value and same data
    // version, and see that persistence does not happen at all.
    // We incremented dataVersion after injecting the last report.  Make sure all the new
    // reports use that last-reported value.
    __block __auto_type lastReportedDataVersion = dataVersion - 1;
    excessiveReportTest(
        6, ^{
            return [self _testAttributeReportWithValue:lastReportedValue dataVersion:lastReportedDataVersion];
        }, false);

    delegate.onReportEnd = nil;
    delegate.onClusterDataPersisted = nil;
}

- (void)test037_MTRDeviceMultipleDelegatesGetReports
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    // First start with clean slate by removing the MTRDevice and clearing the persisted cache
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    [sController removeDevice:device];
    [sController.controllerDataStore clearAllStoredClusterData];
    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    // Now recreate device and get subscription primed
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    XCTestExpectation * gotReportEnd1 = [self expectationWithDescription:@"Report end  for delegate 1"];
    __auto_type * delegate1 = [[MTRDeviceTestDelegate alloc] init];
    __weak __auto_type weakDelegate1 = delegate1;
    delegate1.onReportEnd = ^{
        [gotReportEnd1 fulfill];
        __strong __auto_type strongDelegate = weakDelegate1;
        strongDelegate.onReportEnd = nil;
    };

    [device addDelegate:delegate1 queue:queue];

    // Use autoreleasepool to dealloc a second delegate upon exiting the scope
    @autoreleasepool {
        // Test that a second delegate can also receive attribute reports
        XCTestExpectation * gotAReport2 = [self expectationWithDescription:@"Report end  for delegate 2"];
        __auto_type * delegate2 = [[MTRDeviceTestDelegate alloc] init];
        __weak __auto_type weakDelegate2 = delegate2;
        delegate2.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
            [gotAReport2 fulfill];
            __strong __auto_type strongDelegate = weakDelegate2;
            strongDelegate.onAttributeDataReceived = nil;
        };

        [device addDelegate:delegate2 queue:queue];

        // Wait just long enough for 1 report
        [self waitForExpectations:@[ gotAReport2 ] timeout:60];

        // Verify that at this point MTRDevice is still seeing 2 delegates
        XCTAssertEqual([device unitTestNonnullDelegateCount], 2);
    }

    [self waitForExpectations:@[ gotReportEnd1 ] timeout:60];

    // Verify that once the entire report comes in from all-clusters, that delegate2 had been dealloced, and MTRDevice no longer sees it
    XCTAssertEqual([device unitTestNonnullDelegateCount], 1);
}

- (NSDictionary<NSString *, id> *)_testAttributeResponseValueWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID value:(unsigned int)testValue
{
    return @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID],
        MTRDataKey : @ {
            MTRDataVersionKey : @(testValue),
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(testValue),
        }
    };
}

- (NSDictionary<NSString *, id> *)_testEventResponseValueWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID eventID:(NSNumber *)eventID
{
    // Pick a large event number, so that in practice these event reports will
    // all have larger event numbers than anything that might have been observed
    // before.
    static uint64_t eventNumber = 0x100000000llu;
    return @{
        MTREventPathKey : [MTREventPath eventPathWithEndpointID:endpointID clusterID:clusterID eventID:eventID],
        MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
        MTREventTimestampDateKey : [NSDate date],
        MTREventIsHistoricalKey : @(NO),
        MTREventPriorityKey : @(MTREventPriorityInfo),
        MTREventNumberKey : @(eventNumber++),
        // Empty payload.
        MTRDataKey : @ {
            MTRTypeKey : MTRStructureValueType,
            MTRValueKey : @[],
        },
    };
}

- (void)test038_MTRDeviceMultipleDelegatesInterestedPaths
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    // First start with clean slate by removing the MTRDevice and clearing the persisted cache
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    [sController removeDevice:device];
    [sController.controllerDataStore clearAllStoredClusterData];
    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    // Now recreate device and get subscription primed
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    XCTestExpectation * gotReportEnd1 = [self expectationWithDescription:@"Report end for delegate 1"];

    __auto_type * delegate1 = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate1.skipSetupSubscription = YES;
    __weak __auto_type weakDelegate1 = delegate1;
    __block NSUInteger attributesReceived1 = 0;
    delegate1.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributesReceived1 += data.count;
    };
    __block NSUInteger eventsReceived1 = 0;
    delegate1.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        eventsReceived1 += data.count;
    };
    delegate1.onReportEnd = ^{
        [gotReportEnd1 fulfill];
        __strong __auto_type strongDelegate = weakDelegate1;
        strongDelegate.onReportEnd = nil;
    };

    // All 9 attributes from endpoint 1, plus 3 from endpoint 2, plus endpoint 3 = total 21
    NSArray * interestedAttributePaths1 = @[
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(11) attributeID:@(111)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(11) attributeID:@(112)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(11) attributeID:@(113)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(12) attributeID:@(121)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(12) attributeID:@(122)],
        [MTRAttributePath attributePathWithEndpointID:@(1) clusterID:@(12) attributeID:@(123)],
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(13)],
        [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(21) attributeID:@(211)],
        [MTRAttributePath attributePathWithEndpointID:@(2) clusterID:@(21) attributeID:@(212)],
        [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(21)],
        @(3),
    ];
    // All 9 event from endpoint 1, plus 3 from endpoint 2, plus endpoint 3 = total 21
    NSArray * interestedEventPaths1 = @[
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(11) eventID:@(111)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(11) eventID:@(112)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(11) eventID:@(113)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(12) eventID:@(121)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(12) eventID:@(122)],
        [MTREventPath eventPathWithEndpointID:@(1) clusterID:@(12) eventID:@(123)],
        [MTRClusterPath clusterPathWithEndpointID:@(1) clusterID:@(13)],
        [MTREventPath eventPathWithEndpointID:@(2) clusterID:@(21) eventID:@(211)],
        [MTREventPath eventPathWithEndpointID:@(2) clusterID:@(21) eventID:@(212)],
        [MTRClusterPath clusterPathWithEndpointID:@(2) clusterID:@(21)],
        @(3),
    ];
    [device addDelegate:delegate1 queue:queue interestedPathsForAttributes:interestedAttributePaths1 interestedPathsForEvents:interestedEventPaths1];

    // Delegate 2
    XCTestExpectation * gotReportEnd2 = [self expectationWithDescription:@"Report end for delegate 2"];
    __auto_type * delegate2 = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate2.skipSetupSubscription = YES;
    __weak __auto_type weakDelegate2 = delegate2;
    __block NSUInteger attributesReceived2 = 0;
    delegate2.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributesReceived2 += data.count;
    };
    __block NSUInteger eventsReceived2 = 0;
    delegate2.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        eventsReceived2 += data.count;
    };
    delegate2.onReportEnd = ^{
        [gotReportEnd2 fulfill];
        __strong __auto_type strongDelegate = weakDelegate2;
        strongDelegate.onReportEnd = nil;
    };

    // All 9 attributes from endpoint 3
    NSArray * interestedAttributePaths2 = @[
        [MTRClusterPath clusterPathWithEndpointID:@(3) clusterID:@(31)],
        [MTRClusterPath clusterPathWithEndpointID:@(3) clusterID:@(32)],
        [MTRClusterPath clusterPathWithEndpointID:@(3) clusterID:@(33)],
    ];
    // Test empty events (all filtered)
    [device addDelegate:delegate2 queue:queue interestedPathsForAttributes:interestedAttributePaths2 interestedPathsForEvents:@[]];

    // Delegate 3
    XCTestExpectation * gotReportEnd3 = [self expectationWithDescription:@"Report end for delegate 3"];
    __auto_type * delegate3 = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate3.skipSetupSubscription = YES;
    __weak __auto_type weakDelegate3 = delegate3;
    __block NSUInteger attributesReceived3 = 0;
    delegate3.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributesReceived3 += data.count;
    };
    __block NSUInteger eventsReceived3 = 0;
    delegate3.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        eventsReceived3 += data.count;
    };
    delegate3.onReportEnd = ^{
        [gotReportEnd3 fulfill];
        __strong __auto_type strongDelegate = weakDelegate3;
        strongDelegate.onReportEnd = nil;
    };

    // All 9 events from endpoint 4
    NSArray * interestedEventPaths3 = @[
        [MTRClusterPath clusterPathWithEndpointID:@(4) clusterID:@(41)],
        [MTRClusterPath clusterPathWithEndpointID:@(4) clusterID:@(42)],
        [MTRClusterPath clusterPathWithEndpointID:@(4) clusterID:@(43)],
    ];
    // Test empty attributes (all filtered)
    [device addDelegate:delegate3 queue:queue interestedPathsForAttributes:@[] interestedPathsForEvents:interestedEventPaths3];

    // Delegate 4
    XCTestExpectation * gotReportEnd4 = [self expectationWithDescription:@"Report end for delegate 4"];
    __auto_type * delegate4 = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate3.skipSetupSubscription = YES;
    __weak __auto_type weakDelegate4 = delegate4;
    __block NSUInteger attributesReceived4 = 0;
    delegate4.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributesReceived4 += data.count;
    };
    __block NSUInteger eventsReceived4 = 0;
    delegate4.onEventDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        eventsReceived4 += data.count;
    };
    delegate4.onReportEnd = ^{
        [gotReportEnd4 fulfill];
        __strong __auto_type strongDelegate = weakDelegate4;
        strongDelegate.onReportEnd = nil;
    };

    // Test a fourth delegate that receives everything will get all the reports
    [device addDelegate:delegate4 queue:queue];

    // Inject events first
    NSMutableArray * eventReport = [NSMutableArray array];
    // Construct 36 events with endpoints 1~4, clusters 11 ~ 33, and events 111~333
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 3; j++) {
            for (int k = 1; k <= 3; k++) {
                int endpointID = i;
                int clusterID = i * 10 + j;
                int eventID = i * 100 + j * 10 + k;
                [eventReport addObject:[self _testEventResponseValueWithEndpointID:@(endpointID) clusterID:@(clusterID) eventID:@(eventID)]];
            }
        }
    }
    [device unitTestInjectEventReport:eventReport];

    // Now inject attributes and check that each delegate gets the right set of attributes
    NSMutableArray * attributeReport = [NSMutableArray array];
    // Construct 36 attributes with endpoints 1~4, clusters 11 ~ 33, and attributes 111~333
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 3; j++) {
            for (int k = 1; k <= 3; k++) {
                int endpointID = i;
                int clusterID = i * 10 + j;
                int attributeID = i * 100 + j * 10 + k;
                int value = attributeID + 10000;
                [attributeReport addObject:[self _testAttributeResponseValueWithEndpointID:@(endpointID) clusterID:@(clusterID) attributeID:@(attributeID) value:value]];
            }
        }
    }
    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ gotReportEnd1, gotReportEnd2, gotReportEnd3, gotReportEnd4 ] timeout:60];

    XCTAssertEqual(attributesReceived1, 21);
    XCTAssertEqual(eventsReceived1, 21);
    XCTAssertEqual(attributesReceived2, 9);
    XCTAssertEqual(eventsReceived2, 0);
    XCTAssertEqual(attributesReceived3, 0);
    XCTAssertEqual(eventsReceived3, 9);
    XCTAssertEqual(attributesReceived4, 36);
    XCTAssertEqual(eventsReceived4, 36);

    // Now reset the counts, remove delegate1 and verify that only delegates 2~4 got reports
    attributesReceived1 = 0;
    eventsReceived1 = 0;
    attributesReceived2 = 0;
    eventsReceived2 = 0;
    attributesReceived3 = 0;
    eventsReceived3 = 0;
    attributesReceived4 = 0;
    eventsReceived4 = 0;
    [device removeDelegate:delegate1];

    XCTestExpectation * gotReportEnd2again = [self expectationWithDescription:@"Report end for delegate 2 again"];
    delegate2.onReportEnd = ^{
        [gotReportEnd2again fulfill];
        __strong __auto_type strongDelegate = weakDelegate2;
        strongDelegate.onReportEnd = nil;
    };
    XCTestExpectation * gotReportEnd3again = [self expectationWithDescription:@"Report end for delegate 3 again"];
    delegate3.onReportEnd = ^{
        [gotReportEnd3again fulfill];
        __strong __auto_type strongDelegate = weakDelegate3;
        strongDelegate.onReportEnd = nil;
    };
    XCTestExpectation * gotReportEnd4again = [self expectationWithDescription:@"Report end for delegate 4 again"];
    delegate4.onReportEnd = ^{
        [gotReportEnd4again fulfill];
        __strong __auto_type strongDelegate = weakDelegate4;
        strongDelegate.onReportEnd = nil;
    };

    // Construct 36 new events with new timestamps
    [eventReport removeAllObjects];
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 3; j++) {
            for (int k = 1; k <= 3; k++) {
                int endpointID = i;
                int clusterID = i * 10 + j;
                int eventID = i * 100 + j * 10 + k;
                [eventReport addObject:[self _testEventResponseValueWithEndpointID:@(endpointID) clusterID:@(clusterID) eventID:@(eventID)]];
            }
        }
    }
    [device unitTestInjectEventReport:eventReport];

    // Construct 36 new attributes with new values / data versions
    [attributeReport removeAllObjects];
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 3; j++) {
            for (int k = 1; k <= 3; k++) {
                int endpointID = i;
                int clusterID = i * 10 + j;
                int attributeID = i * 100 + j * 10 + k;
                int value = attributeID + 20000;
                [attributeReport addObject:[self _testAttributeResponseValueWithEndpointID:@(endpointID) clusterID:@(clusterID) attributeID:@(attributeID) value:value]];
            }
        }
    }
    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];
    [self waitForExpectations:@[ gotReportEnd2again, gotReportEnd3again, gotReportEnd4again ] timeout:60];

    XCTAssertEqual(attributesReceived1, 0);
    XCTAssertEqual(eventsReceived1, 0);
    XCTAssertEqual(attributesReceived2, 9);
    XCTAssertEqual(eventsReceived2, 0);
    XCTAssertEqual(attributesReceived3, 0);
    XCTAssertEqual(eventsReceived3, 9);
    XCTAssertEqual(attributesReceived4, 36);
    XCTAssertEqual(eventsReceived4, 36);
}

- (void)test039_GetAllAttributesReport
{
    dispatch_queue_t queue = dispatch_get_main_queue();

    // First start with clean slate by removing the MTRDevice and clearing the persisted cache
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    [sController removeDevice:device];
    [sController.controllerDataStore clearAllStoredClusterData];
    NSDictionary * storedClusterDataAfterClear = [sController.controllerDataStore getStoredClusterDataForNodeID:@(kDeviceId)];
    XCTAssertEqual(storedClusterDataAfterClear.count, 0);

    // Now recreate device and get subscription primed
    device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:sController];
    XCTestExpectation * gotReportEnd = [self expectationWithDescription:@"Report end for delegate"];

    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    __weak __auto_type weakdelegate = delegate;
    __block NSUInteger attributesReceived = 0;
    delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * data) {
        attributesReceived += data.count;
    };
    delegate.onReportEnd = ^{
        [gotReportEnd fulfill];
        __strong __auto_type strongDelegate = weakdelegate;
        strongDelegate.onReportEnd = nil;
    };

    [device addDelegate:delegate queue:queue];

    // Now inject attributes and check that each delegate gets the right set of attributes
    NSMutableArray * attributeReport = [NSMutableArray array];
    // Construct 36 attributes with endpoints 1~4, clusters 11 ~ 33, and attributes 111~333
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 3; j++) {
            for (int k = 1; k <= 3; k++) {
                int endpointID = i;
                int clusterID = i * 10 + j;
                int attributeID = i * 100 + j * 10 + k;
                int value = attributeID + 10000;
                [attributeReport addObject:[self _testAttributeResponseValueWithEndpointID:@(endpointID) clusterID:@(clusterID) attributeID:@(attributeID) value:value]];
            }
        }
    }
    [device unitTestInjectAttributeReport:attributeReport fromSubscription:YES];

    [self waitForExpectations:@[ gotReportEnd ] timeout:60];

    NSArray * allAttributesReport = [device getAllAttributesReport];

    XCTAssertEqual(allAttributesReport.count, attributeReport.count);

    for (NSDictionary<NSString *, id> * newResponseValueDict in allAttributesReport) {
        MTRAttributePath * newPath = newResponseValueDict[MTRAttributePathKey];
        NSDictionary<NSString *, id> * newDataValueDict = newResponseValueDict[MTRDataKey];
        NSNumber * newValue = newDataValueDict[MTRValueKey];
        XCTAssertNotNil(newValue);

        for (NSDictionary<NSString *, id> * originalResponseValueDict in attributeReport) {
            MTRAttributePath * originalPath = originalResponseValueDict[MTRAttributePathKey];
            // Find same attribute path and compare value
            if ([newPath isEqual:originalPath]) {
                NSDictionary<NSString *, id> * originalDataValueDict = originalResponseValueDict[MTRDataKey];
                NSNumber * originalValue = originalDataValueDict[MTRValueKey];
                XCTAssertNotNil(originalValue);
                XCTAssertEqualObjects(newValue, originalValue);
                continue;
            }
        }
    }
}

- (void)test040_AttributeValueExpectationSatisfaction
{
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];

    __auto_type * testData = @[
        @{
            @"expected" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(7),
            },
            @"observed" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(7)
            },
            // Equal unsigned integer should satisfy expectation.
            @"expectedComparison" : @(YES),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(7),
            },
            @"observed" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(9),
            },
            // Unequal unsigned integer should not satisfy expectation
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(7),
            },
            @"observed" : @ {
                MTRTypeKey : MTRSignedIntegerValueType,
                MTRValueKey : @(7),
            },
            // A signed integer does not satisfy expectation for an unsigned integer.
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRNullValueType,
            },
            @"observed" : @ {
                MTRTypeKey : MTRNullValueType,
            },
            // Null satisfies expectation for null.
            @"expectedComparison" : @(YES),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(7),
                        }
                    },
                ],
            },
            // A longer list does not satisfy expectation for a shorter array.
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(7),
                        }
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                ],
            },
            // A shorter list does not satisfy expectation for a longer array.
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                ],
            },
            // An observed array identical to an expected one satisfies the expectation.
            @"expectedComparison" : @(YES),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        }
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(5),
                        }
                    },
                ],
            },
            // An array with entries in a different order does not satisfy the expectation.
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abc"),
                        },
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abc"),
                        },
                    },
                ],
            },
            // A struct that has the same fields in the same order satisfiess the
            // expectation.
            @"expectedComparison" : @(YES),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abc"),
                        },
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abcd"),
                        },
                    },
                ],
            },
            // A struct that has different fields in the same order does not
            // satisfy the expectation.
            @"expectedComparison" : @(NO),
        },
        @{
            @"expected" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abc"),
                        },
                    },
                ],
            },
            @"observed" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUTF8StringValueType,
                            MTRValueKey : @("abc"),
                        },
                    },
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(6),
                        },
                    },
                ],
            },
            // A struct that has the same fields in a different order satisfies
            // the expectation.
            @"expectedComparison" : @(YES),
        },
    ];

    for (NSDictionary * test in testData) {
        XCTAssertEqual([device _attributeDataValue:test[@"observed"] satisfiesValueExpectation:test[@"expected"]], [test[@"expectedComparison"] boolValue],
            "observed: %@, expected: %@", test[@"observed"], test[@"expected"]);
    }
}

- (void)test041_AttributeDataValueValidation
{
    __auto_type * testData = @[
        @{
            @"input" : @ {
                MTRTypeKey : MTRSignedIntegerValueType,
                MTRValueKey : @(-5),
            },
            // -5 is a valid signed integer.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRSignedIntegerValueType,
                MTRValueKey : @ {},
            },
            // A dictionary is not a valid signed integer.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @(7),
            },
            // 7 is a valid unsigned integer.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRUnsignedIntegerValueType,
                MTRValueKey : @("abc"),
            },
            // "abc" is not an unsigned integer.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRBooleanValueType,
                MTRValueKey : @(YES),
            },
            // YES is a boolean.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRBooleanValueType,
                MTRValueKey : [NSData data],
            },
            // NSData is not a boolean integer.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRFloatValueType,
                MTRValueKey : @(8),
            },
            // 8 is a valid float.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRFloatValueType,
                MTRValueKey : @(8.5),
            },
            // 8.5 is a valid float.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRFloatValueType,
                MTRValueKey : @[],
            },
            // An array is not a float.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRDoubleValueType,
                MTRValueKey : @(180),
            },
            // 180 is a valid double.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRDoubleValueType,
                MTRValueKey : @(9.5),
            },
            // 9.5 is a valid double.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRDoubleValueType,
                MTRValueKey : [NSDate date],
            },
            // A date is not a double.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRNullValueType,
            },
            // This is a valid null value.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRUTF8StringValueType,
                MTRValueKey : @("def"),
            },
            // "def" is a valid string.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRUTF8StringValueType,
                MTRValueKey : [NSData data],
            },
            // NSData is not a string.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTROctetStringValueType,
                MTRValueKey : [NSData data],
            },
            // NSData is an octet string.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTROctetStringValueType,
                MTRValueKey : @(7),
            },
            // 7 is not an octet string.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTROctetStringValueType,
                MTRValueKey : @("abc"),
            },
            // "abc" is not an octet string.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[],
            },
            // This is a valid empty structure.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[],
            },
            // This is a valid empty structure.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(7),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRNullValueType
                        },
                    },
                ],
            },
            // This is a valid structure, one null field.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(1),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRNullValueType
                        },
                    },
                    @{
                        MTRContextTagKey : @(2),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(9)
                        },
                    },
                ],
            },
            // This is a valid structure with two fields.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @(19),
            },
            // 19 is not a structure.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRNullValueType
                        },
                    },
                ],
            },
            // Field does not have a context tag.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(7),
                    },
                ],
            },
            // Field does not have a value.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(7),
                        MTRDataKey : @(5),
                    },
                ],
            },
            // Field value is a number, not a data-value
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(7),
                        MTRDataKey : @[],
                    },
                ],
            },
            // Field value is an array, not a data-value
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @(7),
                        MTRDataKey : @ {},
                    },
                ],
            },
            // Field value is an invalid data-value
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRStructureValueType,
                MTRValueKey : @[
                    @{
                        MTRContextTagKey : @("abc"),
                        MTRDataKey : @ {
                            MTRTypeKey : MTRNullValueType
                        },
                    },
                ],
            },
            // Tag is not a number.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[],
            },
            // This is a valid empty array.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRNullValueType
                        },
                    },
                ],
            },
            // This is an array with a single null value in it.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(8),
                        },
                    },
                    @{
                        MTRDataKey : @ {
                            MTRTypeKey : MTRUnsignedIntegerValueType,
                            MTRValueKey : @(10),
                        },
                    },
                ],
            },
            // This is an array with two integers in it.
            @"valid" : @(YES),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[
                    @{
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(8),
                    },
                ],
            },
            // This does not have a proper array-value in the array: missing MTRDataKey.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[ @(7) ],
            },
            // This does not have a proper array-value in the array: not a dictionary.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {
                MTRTypeKey : MTRArrayValueType,
                MTRValueKey : @[ @{} ],
            },
            // This does not have a proper array-value in the array: empty
            // dictionary, so no MTRDataKey.
            @"valid" : @(NO),
        },
    ];

    for (NSDictionary * test in testData) {
        XCTAssertEqual(MTRDataValueDictionaryIsWellFormed(test[@"input"]), [test[@"valid"] boolValue],
            "input: %@", test[@"input"]);
    }
}

- (void)test042_AttributeReportWellFormedness
{
    __auto_type * testData = @[
        @{
            @"input" : @[],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRBooleanValueType,
                        MTRValueKey : @(YES),
                    },
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRBooleanValueType,
                        MTRValueKey : @(YES),
                    },
                },
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(1)],
                    MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:0 userInfo:nil],
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(0)],
                },
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(1)],
                    MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:0 userInfo:nil],
                },
            ],
            // Missing both error and data
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(6) attributeID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRBooleanValueType,
                        MTRValueKey : @("abc"),
                    },
                },
            ],
            // Data dictionary is broken.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {},
            // Input is not an array.
            @"valid" : @(NO),
        },
    ];

    for (NSDictionary * test in testData) {
        XCTAssertEqual(MTRAttributeReportIsWellFormed(test[@"input"]), [test[@"valid"] boolValue],
            "input: %@", test[@"input"]);
    }
}

- (void)test043_EventReportWellFormedness
{
    __auto_type * testData = @[
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:0 userInfo:nil],
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @(5),
                    MTREventPriorityKey : @(MTREventPriorityInfo),
                    MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
                    MTREventTimestampDateKey : [NSDate now],
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @(5),
                    MTREventPriorityKey : @(MTREventPriorityInfo),
                    MTREventTimeTypeKey : @(MTREventTimeTypeSystemUpTime),
                    MTREventSystemUpTimeKey : @(5),
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @(5),
                    MTREventPriorityKey : @(MTREventPriorityInfo),
                    MTREventTimeTypeKey : @(MTREventTimeTypeTimestampDate),
                    MTREventTimestampDateKey : @(5),
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            // Wrong date type
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @("abc"),
                    MTREventPriorityKey : @(MTREventPriorityInfo),
                    MTREventTimeTypeKey : @(MTREventTimeTypeSystemUpTime),
                    MTREventSystemUpTimeKey : @(5),
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            // Wrong type of EventNumber
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @(5),
                    MTREventPriorityKey : @("abc"),
                    MTREventTimeTypeKey : @(MTREventTimeTypeSystemUpTime),
                    MTREventSystemUpTimeKey : @(5),
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            // Wrong type of EventPriority
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTREventPathKey : [MTREventPath eventPathWithEndpointID:@(0) clusterID:@(6) eventID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // No fields
                    },
                    MTREventNumberKey : @(5),
                    MTREventPriorityKey : @(MTREventPriorityInfo),
                    MTREventTimeTypeKey : @("abc"),
                    MTREventSystemUpTimeKey : @(5),
                    MTREventIsHistoricalKey : @(NO),
                },
            ],
            // Wrong type of EventTimeType
            @"valid" : @(NO),
        },
        @{
            @"input" : @[ @(5) ],
            // Wrong type of data entirely.
            @"valid" : @(NO),
        },
        @{
            @"input" : @ {},
            // Not even an array.
            @"valid" : @(NO),
        },
    ];

    for (NSDictionary * test in testData) {
        XCTAssertEqual(MTREventReportIsWellFormed(test[@"input"]), [test[@"valid"] boolValue],
            "input: %@", test[@"input"]);
    }
}

- (void)test044_InvokeResponseWellFormedness
{
    __auto_type * testData = @[
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                },
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                },
            ],
            // Multiple responses
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:0 userInfo:nil],
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // Empty structure, valid
                    },
                },
            ],
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    @"commandPath" : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    @"data" : @ {
                        @"type" : @"Structure",
                        @"value" : @[], // Empty structure, valid
                    },
                },
            ],
            // Same as the previous test, but not using the SDK string constants
            @"valid" : @(YES),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRStructureValueType,
                        MTRValueKey : @[], // Empty structure, valid
                    },
                    MTRErrorKey : [NSError errorWithDomain:MTRErrorDomain code:0 userInfo:nil],
                },
            ],
            // Having both data and error not valid.
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    MTRDataKey : @ {
                        MTRTypeKey : MTRUnsignedIntegerValueType,
                        MTRValueKey : @(5),
                    },
                },
            ],
            // Data is not a struct.
            @"valid" : @(NO),
        },
        @{
            @"input" : @[
                @{
                    MTRCommandPathKey : [MTRCommandPath commandPathWithEndpointID:@(0) clusterID:@(6) commandID:@(0)],
                    MTRDataKey : @(6),
                },
            ],
            // Data is not a data-value at all..
            @"valid" : @(NO),
        },
    ];

    for (NSDictionary * test in testData) {
        XCTAssertEqual(MTRInvokeResponseIsWellFormed(test[@"input"]), [test[@"valid"] boolValue],
            "input: %@", test[@"input"]);
    }
}

- (void)test045_MTRDeviceInvokeGroups
{
    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    // First test: Do an invoke with a single group with three commands, ensure
    // that we get reasonable responses for them all.

    __auto_type * onPath = [MTRCommandPath commandPathWithEndpointID:@(1)
                                                           clusterID:@(MTRClusterIDTypeOnOffID)
                                                           commandID:@(MTRCommandIDTypeClusterOnOffCommandOnID)];
    __auto_type * togglePath = [MTRCommandPath commandPathWithEndpointID:@(1)
                                                               clusterID:@(MTRClusterIDTypeOnOffID)
                                                               commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)];
    __auto_type * offPath = [MTRCommandPath commandPathWithEndpointID:@(1)
                                                            clusterID:@(MTRClusterIDTypeOnOffID)
                                                            commandID:@(MTRCommandIDTypeClusterOnOffCommandOffID)];

    __auto_type * onCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:onPath commandFields:nil requiredResponse:nil];
    __auto_type * toggleCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:togglePath commandFields:nil requiredResponse:nil];
    __auto_type * offCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:offPath commandFields:nil requiredResponse:nil];

    XCTestExpectation * simpleInvokeDone = [self expectationWithDescription:@"Invoke of a single 3-command group done"];
    [device invokeCommands:@[ @[ onCommand, toggleCommand, offCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    // Successful invoke is represented as a value with the relevant
                    // command path and neither data nor error.
                    __auto_type expectedValues = @[
                        @ { MTRCommandPathKey : onPath },
                        @ { MTRCommandPathKey : togglePath },
                        @ { MTRCommandPathKey : offPath },
                    ];
                    XCTAssertEqualObjects(values, expectedValues);
                    [simpleInvokeDone fulfill];
                }];

    // 3 commands, so use triple the timeout.
    [self waitForExpectations:@[ simpleInvokeDone ] timeout:(3 * kTimeoutInSeconds)];

    // Second test: Do an invoke with three groups.  First command in the first
    // group fails, but we should still run all commands in that group.  We
    // should not run any commands in any other groups.
    __auto_type * failingTogglePath = [MTRCommandPath commandPathWithEndpointID:@(1000) // No such endpoint
                                                                      clusterID:@(MTRClusterIDTypeOnOffID)
                                                                      commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)];
    __auto_type * failingToggleCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:failingTogglePath commandFields:nil requiredResponse:nil];

    XCTestExpectation * failingWithStatusInvokeDone = [self expectationWithDescription:@"Invoke of commands where one fails with a status done"];
    [device invokeCommands:@[ @[ failingToggleCommand, offCommand ], @[ onCommand, toggleCommand ], @[ failingToggleCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    // We should not have anything for groups after the first one
                    XCTAssertEqual(values.count, 2);
                    NSDictionary<NSString *, id> * firstValue = values[0];
                    XCTAssertEqualObjects(firstValue[MTRCommandPathKey], failingTogglePath);
                    XCTAssertNil(firstValue[MTRDataKey]);
                    XCTAssertNotNil(firstValue[MTRErrorKey]);
                    XCTAssertTrue([MTRErrorTestUtils error:firstValue[MTRErrorKey] isInteractionModelError:MTRInteractionErrorCodeUnsupportedEndpoint]);

                    XCTAssertEqualObjects(values[1], @ { MTRCommandPathKey : offPath });

                    [failingWithStatusInvokeDone fulfill];
                }];

    // 2 commands actually run, so use double the timeout.
    [self waitForExpectations:@[ failingWithStatusInvokeDone ] timeout:(2 * kTimeoutInSeconds)];

    // Third test: Do an invoke with three groups.  One of the commands in the
    // first group expects a data response but gets a status, which should be
    // treated as a failure.
    __auto_type * onCommandExpectingData = [[MTRCommandWithRequiredResponse alloc] initWithPath:onPath commandFields:nil requiredResponse:@{
        @(0) : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(0),
        }
    }];

    XCTestExpectation * failingWithMissingDataInvokeDone = [self expectationWithDescription:@"Invoke of commands where one fails with missing data done"];
    [device invokeCommands:@[ @[ toggleCommand, onCommandExpectingData, offCommand ], @[ onCommand, toggleCommand ], @[ failingToggleCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    // We should not have anything for groups after the first one
                    __auto_type * expectedValues = @[
                        @ { MTRCommandPathKey : togglePath },
                        @ { MTRCommandPathKey : onPath },
                        @ { MTRCommandPathKey : offPath },
                    ];
                    XCTAssertEqualObjects(values, expectedValues);

                    [failingWithMissingDataInvokeDone fulfill];
                }];

    // 3 commands actually run, so use triple the timeout.
    [self waitForExpectations:@[ failingWithMissingDataInvokeDone ] timeout:(3 * kTimeoutInSeconds)];

    // Fourth test: do an invoke with two groups.  One of the commands in the
    // first group expects to not get a falure status and gets data, which
    // should be treated as success.
    __auto_type * updateFabricLabelPath = [MTRCommandPath commandPathWithEndpointID:@(0)
                                                                          clusterID:@(MTRClusterIDTypeOperationalCredentialsID)
                                                                          commandID:@(MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID)];
    __auto_type * nocResponsePath = [MTRCommandPath commandPathWithEndpointID:@(0)
                                                                    clusterID:@(MTRClusterIDTypeOperationalCredentialsID)
                                                                    commandID:@(MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID)];
    __auto_type * updateFabricLabelFields = @{
        MTRTypeKey : MTRStructureValueType,
        MTRValueKey : @[
            @{
                MTRContextTagKey : @(0),
                MTRDataKey : @ {
                    MTRTypeKey : MTRUTF8StringValueType,
                    MTRValueKey : @"newlabel",
                },
            },
        ]
    };
    __auto_type * updateFabricLabelNotExpectingFailureCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:updateFabricLabelPath commandFields:updateFabricLabelFields requiredResponse:nil];

    XCTestExpectation * updateFabricLabelNotExpectingFailureExpectation = [self expectationWithDescription:@"Invoke of commands where no failure is expected and data response is received done"];
    [device invokeCommands:@[ @[ updateFabricLabelNotExpectingFailureCommand, onCommand ], @[ offCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    XCTAssertEqual(values.count, 3);

                    NSDictionary<NSString *, id> * updateFabricLabelResponse = values[0];
                    XCTAssertEqualObjects(updateFabricLabelResponse[MTRCommandPathKey], nocResponsePath);
                    NSDictionary<NSString *, id> * responseData = updateFabricLabelResponse[MTRDataKey];
                    XCTAssertEqualObjects(responseData[MTRTypeKey], MTRStructureValueType);
                    NSArray<NSDictionary<NSString *, id> *> * responseFields = responseData[MTRValueKey];
                    XCTAssertTrue(responseFields.count > 0);

                    for (NSDictionary<NSString *, id> * field in responseFields) {
                        if ([@(0) isEqual:field[MTRContextTagKey]]) {
                            // Check that this in fact succeeded.
                            NSDictionary<NSString *, id> * fieldValue = field[MTRDataKey];
                            XCTAssertEqualObjects(fieldValue[MTRTypeKey], MTRUnsignedIntegerValueType);
                            XCTAssertEqualObjects(fieldValue[MTRValueKey], @(MTROperationalCredentialsNodeOperationalCertStatusOK));
                        }
                    }

                    XCTAssertEqualObjects(values[1], @ { MTRCommandPathKey : onPath });
                    XCTAssertEqualObjects(values[2], @ { MTRCommandPathKey : offPath });

                    [updateFabricLabelNotExpectingFailureExpectation fulfill];
                }];

    // 3 commands actually run, so use triple the timeout.
    [self waitForExpectations:@[ updateFabricLabelNotExpectingFailureExpectation ] timeout:(3 * kTimeoutInSeconds)];

    // Fifth test: do an invoke with two groups.  One of the commands in the
    // first group expects to get a data response and gets it, which should be
    // treated as success.
    __auto_type * updateFabricLabelExpectingOKCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:updateFabricLabelPath commandFields:updateFabricLabelFields requiredResponse:@{
        @(0) : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(MTROperationalCredentialsNodeOperationalCertStatusOK),
        },
    }];

    XCTestExpectation * updateFabricLabelExpectingOKExpectation = [self expectationWithDescription:@"Invoke of commands where data response is expected and received done"];
    [device invokeCommands:@[ @[ updateFabricLabelExpectingOKCommand, onCommand ], @[ offCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    XCTAssertEqual(values.count, 3);

                    NSDictionary<NSString *, id> * updateFabricLabelResponse = values[0];
                    XCTAssertEqualObjects(updateFabricLabelResponse[MTRCommandPathKey], nocResponsePath);
                    NSDictionary<NSString *, id> * responseData = updateFabricLabelResponse[MTRDataKey];
                    XCTAssertEqualObjects(responseData[MTRTypeKey], MTRStructureValueType);
                    NSArray<NSDictionary<NSString *, id> *> * responseFields = responseData[MTRValueKey];
                    XCTAssertTrue(responseFields.count > 0);

                    for (NSDictionary<NSString *, id> * field in responseFields) {
                        if ([@(0) isEqual:field[MTRContextTagKey]]) {
                            // Check that this in fact succeeded.
                            NSDictionary<NSString *, id> * fieldValue = field[MTRDataKey];
                            XCTAssertEqualObjects(fieldValue[MTRTypeKey], MTRUnsignedIntegerValueType);
                            XCTAssertEqualObjects(fieldValue[MTRValueKey], @(MTROperationalCredentialsNodeOperationalCertStatusOK));
                        }
                    }

                    XCTAssertEqualObjects(values[1], @ { MTRCommandPathKey : onPath });
                    XCTAssertEqualObjects(values[2], @ { MTRCommandPathKey : offPath });

                    [updateFabricLabelExpectingOKExpectation fulfill];
                }];

    // 3 commands actually run, so use triple the timeout.
    [self waitForExpectations:@[ updateFabricLabelExpectingOKExpectation ] timeout:(3 * kTimeoutInSeconds)];

    // Sixth test: do an invoke with two groups.  One of the commands in the
    // first group expects to get a data response with a field that it does not get, which should be
    // treated as failure.
    __auto_type * updateFabricLabelExpectingNonexistentFieldCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:updateFabricLabelPath commandFields:updateFabricLabelFields requiredResponse:@{
        @(20) : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(MTROperationalCredentialsNodeOperationalCertStatusOK),
        },
    }];

    XCTestExpectation * updateFabricLabelExpectingNonexistentFieldExpectation = [self expectationWithDescription:@"Invoke of commands where data response is expected but the received one is missing a field done"];
    [device invokeCommands:@[ @[ updateFabricLabelExpectingNonexistentFieldCommand, onCommand ], @[ offCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    XCTAssertEqual(values.count, 2);

                    NSDictionary<NSString *, id> * updateFabricLabelResponse = values[0];
                    XCTAssertEqualObjects(updateFabricLabelResponse[MTRCommandPathKey], nocResponsePath);
                    NSDictionary<NSString *, id> * responseData = updateFabricLabelResponse[MTRDataKey];
                    XCTAssertEqualObjects(responseData[MTRTypeKey], MTRStructureValueType);
                    NSArray<NSDictionary<NSString *, id> *> * responseFields = responseData[MTRValueKey];
                    XCTAssertTrue(responseFields.count > 0);

                    for (NSDictionary<NSString *, id> * field in responseFields) {
                        if ([@(0) isEqual:field[MTRContextTagKey]]) {
                            // Check that this in fact succeeded.
                            NSDictionary<NSString *, id> * fieldValue = field[MTRDataKey];
                            XCTAssertEqualObjects(fieldValue[MTRTypeKey], MTRUnsignedIntegerValueType);
                            XCTAssertEqualObjects(fieldValue[MTRValueKey], @(MTROperationalCredentialsNodeOperationalCertStatusOK));
                        }
                    }

                    XCTAssertEqualObjects(values[1], @ { MTRCommandPathKey : onPath });

                    [updateFabricLabelExpectingNonexistentFieldExpectation fulfill];
                }];

    // 2 commands actually run, so use double the timeout.
    [self waitForExpectations:@[ updateFabricLabelExpectingNonexistentFieldExpectation ] timeout:(2 * kTimeoutInSeconds)];

    // Seventh test: do an invoke with two groups.  One of the commands in the    // first group expects to get a data response with a field value that does
    // not match what it gets, which should be treated as a failure.
    __auto_type * updateFabricLabelExpectingWrongValueCommand = [[MTRCommandWithRequiredResponse alloc] initWithPath:updateFabricLabelPath commandFields:updateFabricLabelFields requiredResponse:@{
        @(0) : @ {
            MTRTypeKey : MTRUnsignedIntegerValueType,
            MTRValueKey : @(MTROperationalCredentialsNodeOperationalCertStatusFabricConflict),
        },
    }];

    XCTestExpectation * updateFabricLabelExpectingWrongValueExpectation = [self expectationWithDescription:@"Invoke of commands where data response is expected but with the wrong value done"];
    [device invokeCommands:@[ @[ updateFabricLabelExpectingWrongValueCommand, onCommand ], @[ offCommand ] ]
                     queue:queue
                completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                    XCTAssertNil(error);
                    XCTAssertNotNil(values);
                    XCTAssertTrue(MTRInvokeResponsesAreWellFormed(values));

                    XCTAssertEqual(values.count, 2);

                    NSDictionary<NSString *, id> * updateFabricLabelResponse = values[0];
                    XCTAssertEqualObjects(updateFabricLabelResponse[MTRCommandPathKey], nocResponsePath);
                    NSDictionary<NSString *, id> * responseData = updateFabricLabelResponse[MTRDataKey];
                    XCTAssertEqualObjects(responseData[MTRTypeKey], MTRStructureValueType);
                    NSArray<NSDictionary<NSString *, id> *> * responseFields = responseData[MTRValueKey];
                    XCTAssertTrue(responseFields.count > 0);

                    for (NSDictionary<NSString *, id> * field in responseFields) {
                        if ([@(0) isEqual:field[MTRContextTagKey]]) {
                            // Check that this in fact succeeded.
                            NSDictionary<NSString *, id> * fieldValue = field[MTRDataKey];
                            XCTAssertEqualObjects(fieldValue[MTRTypeKey], MTRUnsignedIntegerValueType);
                            XCTAssertEqualObjects(fieldValue[MTRValueKey], @(MTROperationalCredentialsNodeOperationalCertStatusOK));
                        }
                    }

                    XCTAssertEqualObjects(values[1], @ { MTRCommandPathKey : onPath });

                    [updateFabricLabelExpectingWrongValueExpectation fulfill];
                }];

    // 2 commands actually run, so use double the timeout.
    [self waitForExpectations:@[ updateFabricLabelExpectingWrongValueExpectation ] timeout:(2 * kTimeoutInSeconds)];
}

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
