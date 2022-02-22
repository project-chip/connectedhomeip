//
//  CHIPDeviceTests.m
//  CHIPDeviceTests
/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#import <CHIP/CHIPDevice.h>

#import "CHIPErrorTestUtils.h"

#import <app/util/af-enums.h>

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

// Set the following to 1 in order to run individual test case manually.
#define MANUAL_INDIVIDUAL_TEST 0

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kCASESetupTimeoutInSeconds = 30;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static const uint16_t kDiscriminator = 3840;
static const uint32_t kSetupPINCode = 20202021;
static const uint16_t kRemotePort = 5540;
static const uint16_t kLocalPort = 5541;
static NSString * kAddress = @"::1";

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static CHIPDevice * mConnectedDevice;

static void WaitForCommissionee(XCTestExpectation * expectation, dispatch_queue_t queue)
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    [controller getConnectedDevice:kDeviceId
                             queue:dispatch_get_main_queue()
                 completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                     XCTAssertEqual(error.code, 0);
                     [expectation fulfill];
                     mConnectedDevice = device;
                 }];
}

static CHIPDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(mConnectedDevice);
    return mConnectedDevice;
}

@interface CHIPDeviceTestPairingDelegate : NSObject <CHIPDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation CHIPDeviceTestPairingDelegate
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

- (void)onCommissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

- (void)onAddressUpdated:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}
@end

@interface CHIPDeviceTests : XCTestCase
@end

@implementation CHIPDeviceTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)tearDown
{
#if MANUAL_INDIVIDUAL_TEST
    [self shutdownStack];
#endif
    [super tearDown];
}

- (void)initStack
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    CHIPDeviceTestPairingDelegate * pairing = [[CHIPDeviceTestPairingDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    [controller setListenPort:kLocalPort];
    [controller setPairingDelegate:pairing queue:callbackQueue];

    BOOL started = [controller startup:nil vendorId:0 nocSigner:nil];
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

    __block XCTestExpectation * connectionExpectation = [self expectationWithDescription:@"CASE established"];
    [controller getConnectedDevice:kDeviceId
                             queue:dispatch_get_main_queue()
                 completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                     XCTAssertEqual(error.code, 0);
                     [connectionExpectation fulfill];
                     connectionExpectation = nil;
                 }];
    [self waitForExpectationsWithTimeout:kCASESetupTimeoutInSeconds handler:nil];
}

- (void)shutdownStack
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    BOOL stopped = [controller shutdown];
    XCTAssertTrue(stopped);
}

- (void)waitForCommissionee
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Wait for the commissioned device to be retrieved"];

    dispatch_queue_t queue = dispatch_get_main_queue();
    WaitForCommissionee(expectation, queue);
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test000_SetUp
{
    [self initStack];
    [self waitForCommissionee];
}
#endif

- (void)test001_ReadAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributeWithEndpointId:0xffff
                              clusterId:29
                            attributeId:0
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 29);
                                         XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 0);
                                         XCTAssertTrue([result[@"endpointId"] isKindOfClass:[NSNumber class]]);
                                         XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                         XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Array"]);
                                     }
                                     XCTAssertTrue([resultArray count] > 0);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test002_WriteAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"write LevelControl Brightness attribute"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointId:1
                               clusterId:8
                             attributeId:17
                                   value:writeValue
                             clientQueue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                          XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 8);
                                          XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 17);
                                          XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                      }
                                      XCTAssertEqual([resultArray count], 1);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test003_InvokeCommand
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = [NSDictionary
        dictionaryWithObjectsAndKeys:@"Structure", @"type",
        [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:0], @"tag",
                                                [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                                              [NSNumber numberWithUnsignedInteger:0], @"value", nil],
                                                @"value", nil],
                 [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:1], @"tag",
                               [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                             [NSNumber numberWithUnsignedInteger:10], @"value", nil],
                               @"value", nil],
                 nil],
        @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:8
                              commandId:4
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 8);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 4);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

static void (^globalReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

#if !MANUAL_INDIVIDUAL_TEST
- (void)test004_SubscribeOnly
{
    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    [device subscribeAttributeWithEndpointId:1
        clusterId:6
        attributeId:0
        minInterval:1
        maxInterval:10
        clientQueue:queue
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
}
#endif

// Report behavior is erratic on the accessory side at the moment.
// Hence this test is enabled only for individual manual test.
#if MANUAL_INDIVIDUAL_TEST
- (void)test005_Subscribe
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

#if MANUAL_INDIVIDUAL_TEST
    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    [device subscribeAttributeWithEndpointId:1
        clusterId:6
        attributeId:0
        minInterval:1
        maxInterval:10
        clientQueue:queue
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
#endif

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
        NSDictionary * result = value;
        XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
        XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
        XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"value"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"value"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"value"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:6
                              commandId:1
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
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
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
        NSDictionary * result = value;
        XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
        XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
        XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"value"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"value"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"value"][@"value"] boolValue] == NO) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send command to trigger attribute change
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:6
                              commandId:0
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 0);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];
}
#endif

- (void)test006_ReadAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device
        readAttributeWithEndpointId:0
                          clusterId:10000
                        attributeId:0
                        clientQueue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                             XCTAssertNil(values);
                             XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER);

                             [expectation fulfill];
                         }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test007_WriteAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"write failed"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointId:1
                               clusterId:8
                             attributeId:10000
                                   value:writeValue
                             clientQueue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertNil(values);
                                  XCTAssertEqual(
                                      [CHIPErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);

                                  [expectation fulfill];
                              }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

#if 0 // Re-enable test if the crash bug in CHIP stack is fixed to handle bad command Id
- (void)test008_InvokeCommandFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary *fields = [NSDictionary dictionaryWithObjectsAndKeys:
                            @"Structure", @"type",
                            [NSArray arrayWithObjects:
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInteger:0], @"tag",
                              [NSDictionary dictionaryWithObjectsAndKeys:
                               @"UnsignedInteger", @"type",
                               [NSNumber numberWithUnsignedInteger:0], @"value", nil], @"value", nil],
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInteger:1], @"tag",
                              [NSDictionary dictionaryWithObjectsAndKeys:
                               @"UnsignedInteger", @"type",
                               [NSNumber numberWithUnsignedInteger:10], @"value", nil], @"value", nil],
                             nil], @"value", nil];
    [device invokeCommandWithEndpointId:1 clusterId:8 commandId:40000 commandFields:fields clientQueue:queue completion:^(id _Nullable values, NSError * _Nullable error) {
        NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

        XCTAssertNil(values);
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);

        [expectation fulfill];
    }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}
#endif

- (void)test009_SubscribeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Set up expectation for report
    XCTestExpectation * errorReportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    reportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(value);
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
        [errorReportExpectation fulfill];
    };

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device subscribeAttributeWithEndpointId:10000
        clusterId:6
        attributeId:0
        minInterval:2
        maxInterval:10
        clientQueue:queue
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
            [expectation fulfill];
        }];

    // Wait till establishment and error report
    [self waitForExpectations:[NSArray arrayWithObjects:expectation, errorReportExpectation, nil] timeout:kTimeoutInSeconds];
}

- (void)test010_ReadAllAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributeWithEndpointId:1
                              clusterId:29
                            attributeId:0xffffffff
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 29);
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                     }
                                     XCTAssertTrue([resultArray count] > 0);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

// Report behavior is erratic on the accessory side at the moment.
// Hence this test is enabled only for individual manual test.
#if MANUAL_INDIVIDUAL_TEST
- (void)test900_SubscribeAllAttributes
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    [device subscribeAttributeWithEndpointId:1
        clusterId:6
        attributeId:0xffffffff
        minInterval:2
        maxInterval:10
        clientQueue:queue
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
    reportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
        NSDictionary * result = value;
        XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
        XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"value"] isKindOfClass:[NSDictionary class]]);

        if ([result[@"attributeId"] unsignedIntegerValue] == 0 && [result[@"value"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:6
                              commandId:1
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
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
    reportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
        NSDictionary * result = value;
        XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
        XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"value"] isKindOfClass:[NSDictionary class]]);
        if ([result[@"attributeId"] unsignedIntegerValue] == 0 && [result[@"value"][@"value"] boolValue] == NO) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send command to trigger attribute change
    commandExpectation = [self expectationWithDescription:@"command responded"];
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:6
                              commandId:0
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 0);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];
}
#endif

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    [self shutdownStack];
}
#endif

@end

@interface CHIPDevice (Test)
// Test function for whitebox testing
+ (id)CHIPEncodeAndDecodeNSObject:(id)object;
@end

@interface CHIPDeviceEncoderTests : XCTestCase
@end

@implementation CHIPDeviceEncoderTests

- (void)testSignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:-713], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testUnsignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:1025], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testBoolean
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type", [NSNumber numberWithBool:YES], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testUTF8String
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"UTF8String", @"type", @"Hello World", @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testOctetString
{
    const uint8_t data[] = { 0x00, 0xF2, 0x63 };
    NSDictionary * input = [NSDictionary
        dictionaryWithObjectsAndKeys:@"OctetString", @"type", [NSData dataWithBytes:data length:sizeof(data)], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testFloat
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"type", [NSNumber numberWithFloat:0.1245], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    // Note that conversion doesn't guarantee back to Float.
    XCTAssertTrue([output[@"type"] isEqualToString:@"Float"] || [output[@"type"] isEqualToString:@"Double"]);
    XCTAssertTrue(([output[@"value"] doubleValue] - [input[@"value"] doubleValue]) < 0.0001);
}

- (void)testDouble
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Double", @"type", [NSNumber numberWithFloat:0.1245], @"value", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    // Note that conversion doesn't guarantee back to Double.
    XCTAssertTrue([output[@"type"] isEqualToString:@"Float"] || [output[@"type"] isEqualToString:@"Double"]);
    XCTAssertTrue(([output[@"value"] doubleValue] - [input[@"value"] doubleValue]) < 0.0001);
}

- (void)testNull
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"Null", @"type", nil];
    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqualTo:input]);
}

- (void)testStructure
{
    NSArray * inputFields = [NSArray
        arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:1], @"tag",
                                       [NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type", [NSNumber numberWithBool:NO],
                                                     @"value", nil],
                                       @"value", nil],
        [NSDictionary
            dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:2], @"tag",
            [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:5], @"value", nil],
            @"value", nil],
        nil];
    NSDictionary * inputValue = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", inputFields, @"value", nil];

    // Output will have context tags and hence build object with context tags for comparison
    NSMutableArray * contextTaggedInputFields = [NSMutableArray arrayWithCapacity:[inputFields count]];
    for (NSDictionary * field in inputFields) {
        [contextTaggedInputFields
            addObject:[NSDictionary
                          dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:([field[@"tag"] unsignedIntegerValue]
                                                                                               | 0xffffffff00000000ull)],
                          @"tag", field[@"value"], @"value", nil]];
    }

    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);

    NSDictionary * compare =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", contextTaggedInputFields, @"value", nil];
    XCTAssertTrue([output isEqualTo:compare]);
}

- (void)testArray
{
    NSArray * inputFields = [NSArray
        arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type",
                                                                                  [NSNumber numberWithBool:NO], @"value", nil],
                                       @"value", nil],
        [NSDictionary dictionaryWithObjectsAndKeys:[NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                                 [NSNumber numberWithInteger:5], @"value", nil],
                      @"value", nil],
        nil];
    NSDictionary * inputValue = [NSDictionary dictionaryWithObjectsAndKeys:@"Array", @"type", inputFields, @"value", nil];

    // Output will have anonymous tags and hence build object with context tags for comparison
    NSMutableArray * contextTaggedInputFields = [NSMutableArray arrayWithCapacity:[inputFields count]];
    for (NSDictionary * field in inputFields) {
        [contextTaggedInputFields
            addObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:0xffffffffffffffffull], @"tag",
                                    field[@"value"], @"value", nil]];
    }

    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);

    NSDictionary * compare = [NSDictionary dictionaryWithObjectsAndKeys:@"Array", @"type", contextTaggedInputFields, @"value", nil];
    XCTAssertTrue([output isEqualTo:compare]);
}

- (void)testList
{
    NSArray * inputFields = [NSArray
        arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:1], @"tag",
                                       [NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type", [NSNumber numberWithBool:NO],
                                                     @"value", nil],
                                       @"value", nil],
        [NSDictionary
            dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:2], @"tag",
            [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:5], @"value", nil],
            @"value", nil],
        nil];
    NSDictionary * inputValue = [NSDictionary dictionaryWithObjectsAndKeys:@"List", @"type", inputFields, @"value", nil];

    // Output will have context tags and hence build object with context tags for comparison
    NSMutableArray * contextTaggedInputFields = [NSMutableArray arrayWithCapacity:[inputFields count]];
    for (NSDictionary * field in inputFields) {
        [contextTaggedInputFields
            addObject:[NSDictionary
                          dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:([field[@"tag"] unsignedIntegerValue]
                                                                                               | 0xffffffff00000000ull)],
                          @"tag", field[@"value"], @"value", nil]];
    }

    id output = [CHIPDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);

    NSDictionary * compare = [NSDictionary dictionaryWithObjectsAndKeys:@"List", @"type", contextTaggedInputFields, @"value", nil];
    XCTAssertTrue([output isEqualTo:compare]);
}

@end
