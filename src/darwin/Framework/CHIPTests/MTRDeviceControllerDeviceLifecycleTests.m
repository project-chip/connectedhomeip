/**
 *    Copyright (c) 2026 Project CHIP Authors
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
#import <XCTest/XCTest.h>

#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTestVendorId = 0xFFF1u;
static const NSTimeInterval kTimeoutInSeconds = 10;

@interface MTRDeviceLifecycleTestsControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (atomic, readonly) NSUInteger devicesChangedCount;
@end

@implementation MTRDeviceLifecycleTestsControllerDelegate {
    NSUInteger _devicesChangedCount;
}

- (NSUInteger)devicesChangedCount
{
    @synchronized(self) {
        return _devicesChangedCount;
    }
}

- (void)devicesChangedForController:(MTRDeviceController *)controller
{
    @synchronized(self) {
        ++_devicesChangedCount;
    }
}

@end

@interface MTRDeviceLifecycleTestsDeviceDelegate : NSObject <MTRDeviceDelegate>
@property (atomic, readonly) NSUInteger attributeReportCount;
@property (atomic, readonly) NSUInteger becameActiveCount;
@end

@implementation MTRDeviceLifecycleTestsDeviceDelegate {
    NSUInteger _attributeReportCount;
    NSUInteger _becameActiveCount;
}

- (NSUInteger)attributeReportCount
{
    @synchronized(self) {
        return _attributeReportCount;
    }
}

- (NSUInteger)becameActiveCount
{
    @synchronized(self) {
        return _becameActiveCount;
    }
}

- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<MTRDeviceResponseValueDictionary> *)attributeReport
{
    @synchronized(self) {
        ++_attributeReportCount;
    }
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<MTRDeviceResponseValueDictionary> *)eventReport
{
}

- (void)deviceBecameActive:(MTRDevice *)device
{
    @synchronized(self) {
        ++_becameActiveCount;
    }
}

@end

@interface MTRDeviceLifecycleTestsRejectingListenerDelegate : NSObject <NSXPCListenerDelegate>
@end

@implementation MTRDeviceLifecycleTestsRejectingListenerDelegate
- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    return NO;
}
@end

@interface MTRDeviceController (DeviceLifecycleTests)
- (BOOL)definitelyUsesThreadForDevice:(uint64_t)nodeID;
@end

@interface MTRDeviceControllerDeviceLifecycleTests : XCTestCase
@end

@implementation MTRDeviceControllerDeviceLifecycleTests {
    dispatch_queue_t _storageQueue;
    BOOL _localTestStorageEnabledBeforeUnitTest;
    MTRDeviceLifecycleTestsRejectingListenerDelegate * _listenerDelegate;
}

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
    _localTestStorageEnabledBeforeUnitTest = MTRDeviceControllerLocalTestStorage.localTestStorageEnabled;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = NO;
    _storageQueue = dispatch_queue_create("test.storage.queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    _listenerDelegate = [[MTRDeviceLifecycleTestsRejectingListenerDelegate alloc] init];
}

- (void)tearDown
{
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
    _storageQueue = nil;
    MTRDeviceControllerLocalTestStorage.localTestStorageEnabled = _localTestStorageEnabledBeforeUnitTest;
    [super tearDown];
}

- (void)waitForQueue:(dispatch_queue_t)queue
{
    XCTestExpectation * drained = [self expectationWithDescription:@"Queue drained"];
    dispatch_async(queue, ^{
        [drained fulfill];
    });
    [self waitForExpectations:@[ drained ] timeout:kTimeoutInSeconds];
}

- (void)waitForNoDevicesOnController:(MTRDeviceController *)controller
{
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:kTimeoutInSeconds];
    NSUInteger deviceCount;
    while (true) {
        @autoreleasepool {
            deviceCount = controller.devices.count;
        }
        if (deviceCount == 0 || deadline.timeIntervalSinceNow <= 0) {
            break;
        }
        [NSThread sleepForTimeInterval:0.01];
    }
    XCTAssertEqual(deviceCount, 0);
}

- (MTRDeviceController *)startControllerWithStorage:(MTRTestPerControllerStorage *)storage
{
    __auto_type * rootKeys = [[MTRTestKeys alloc] init];
    __auto_type * operationalKeys = [[MTRTestKeys alloc] init];

    NSError * error;
    __auto_type * root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:&error];
    XCTAssertNil(error);
    __auto_type publicKey = operationalKeys.copyPublicKey;
    CFAutorelease(publicKey);
    __auto_type * operational = [MTRCertificates createOperationalCertificate:rootKeys
                                                           signingCertificate:root
                                                         operationalPublicKey:publicKey
                                                                     fabricID:@(456)
                                                                       nodeID:@(123)
                                                        caseAuthenticatedTags:nil
                                                                        error:&error];
    XCTAssertNil(error);

    __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:storage
                                                                                        storageDelegateQueue:_storageQueue
                                                                                            uniqueIdentifier:storage.controllerID
                                                                                                         ipk:rootKeys.ipk
                                                                                                    vendorID:@(kTestVendorId)
                                                                                          operationalKeypair:operationalKeys
                                                                                      operationalCertificate:operational
                                                                                     intermediateCertificate:nil
                                                                                             rootCertificate:root];
    params.shouldAdvertiseOperational = NO;

    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNil(error);
    XCTAssertTrue(controller.isRunning);
    return controller;
}

// A session request for a node with no MTRDevice must not create one.
- (void)testSessionRequestForNodeWithoutDeviceDoesNotCreateDevice
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    __auto_type * baseDevice = [MTRBaseDevice deviceWithNodeID:@(17) controller:controller];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;

    [baseDevice subscribeWithQueue:delegateQueue
                            params:params
        clusterStateCacheContainer:nil
            attributeReportHandler:nil
                eventReportHandler:nil
                      errorHandler:^(NSError * error) {
                      }
           subscriptionEstablished:nil
           resubscriptionScheduled:nil];
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(controller.devices.count, 0);
    XCTAssertEqual(delegate.devicesChangedCount, 0);
    [controller shutdown];
}

// Thread detection for a node with no MTRDevice reads the stored FeatureMap.
- (void)testThreadStatusForNodeWithoutDeviceComesFromStorage
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    __auto_type * networkCommissioningData = [[MTRDeviceClusterData alloc] initWithDataVersion:@(1) attributes:@{
        @(MTRAttributeIDTypeGlobalAttributeFeatureMapID) : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(MTRNetworkCommissioningFeatureThreadNetworkInterface) },
    }];
    [controller.controllerDataStore storeClusterData:@{
        [MTRClusterPath clusterPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeNetworkCommissioningID)] : networkCommissioningData,
    }
                                           forNodeID:@(17)];

    @autoreleasepool {
        XCTAssertTrue([controller definitelyUsesThreadForDevice:17]);
        XCTAssertFalse([controller definitelyUsesThreadForDevice:18]);
    }
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(controller.devices.count, 0);
    XCTAssertEqual(delegate.devicesChangedCount, 0);
    [controller shutdown];
}

// Create and dealloc report once each; a second lookup of a live device reports nothing.
- (void)testDevicesChangedFiresOnceForCreateAndOnceForDealloc
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    @autoreleasepool {
        MTRDevice * device = [MTRDevice deviceWithNodeID:@(17) controller:controller];
        XCTAssertEqual([MTRDevice deviceWithNodeID:@(17) controller:controller], device);
        [self waitForQueue:delegateQueue];
        XCTAssertEqual(delegate.devicesChangedCount, 1);
        device = nil;
    }
    [self waitForNoDevicesOnController:controller];
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(delegate.devicesChangedCount, 2);
    [controller shutdown];
}

// A queued devicesChangedForController: does not keep the delegate alive.
- (void)testQueuedDevicesChangedDoesNotRetainDelegate
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_suspend(delegateQueue);

    __weak MTRDeviceLifecycleTestsControllerDelegate * weakDelegate;
    @autoreleasepool {
        __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
        weakDelegate = delegate;
        [controller addDeviceControllerDelegate:delegate queue:delegateQueue];
        XCTAssertNotNil([MTRDevice deviceWithNodeID:@(17) controller:controller]);
    }
    XCTAssertNil(weakDelegate);

    dispatch_resume(delegateQueue);
    [self waitForQueue:delegateQueue];
    [controller shutdown];
}

- (MTRDeviceController *)startXPCControllerWithListener:(NSXPCListener *)listener
{
    listener.delegate = _listenerDelegate;
    [listener resume];

    __auto_type * params = [[MTRXPCDeviceControllerParameters alloc] initWithXPConnectionBlock:^NSXPCConnection * {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:listener.endpoint];
    } uniqueIdentifier:[NSUUID UUID]];
    NSError * error;
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNil(error);
    XCTAssertNotNil(controller);
    return controller;
}

// XPC reports for a node with no MTRDevice are dropped without creating one.
- (void)testXPCReportForNodeWithoutDeviceDoesNotCreateDevice
{
    NSXPCListener * listener = [NSXPCListener anonymousListener];
    MTRDeviceController * controller = [self startXPCControllerWithListener:listener];

    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    id<MTRXPCClientProtocol> client = (id<MTRXPCClientProtocol>) controller;
    @autoreleasepool {
        [client device:@(17) stateChanged:MTRDeviceStateReachable];
        [client device:@(17) receivedAttributeReport:@[]];
        [client device:@(17) receivedEventReport:@[]];
        [client deviceBecameActive:@(17)];
        [client deviceCachePrimed:@(17)];
        [client deviceConfigurationChanged:@(17)];
        [client device:@(17) internalStateUpdated:@{}];
    }
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(controller.devices.count, 0);
    XCTAssertEqual(delegate.devicesChangedCount, 0);
    [controller shutdown];
    [listener invalidate];
}

// XPC reports reach a device the client created.
- (void)testXPCReportIsDeliveredOnceClientCreatesDevice
{
    NSXPCListener * listener = [NSXPCListener anonymousListener];
    MTRDeviceController * controller = [self startXPCControllerWithListener:listener];
    id<MTRXPCClientProtocol> client = (id<MTRXPCClientProtocol>) controller;
    NSArray<MTRDeviceResponseValueDictionary> * report = @[ @{
        MTRAttributePathKey : [MTRAttributePath attributePathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeBasicInformationID) attributeID:@(MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID)],
        MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(17) },
    } ];

    [client device:@(17) receivedAttributeReport:report];
    [client deviceBecameActive:@(17)];
    @autoreleasepool {
        XCTAssertEqual(controller.devices.count, 0);
    }

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(17) controller:controller];
    __auto_type * deviceDelegate = [[MTRDeviceLifecycleTestsDeviceDelegate alloc] init];
    dispatch_queue_t deviceDelegateQueue = dispatch_queue_create("test.device.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [device addDelegate:deviceDelegate queue:deviceDelegateQueue];
    [self waitForQueue:deviceDelegateQueue];
    XCTAssertEqual(deviceDelegate.attributeReportCount, 0);
    XCTAssertEqual(deviceDelegate.becameActiveCount, 0);

    [client device:@(17) receivedAttributeReport:report];
    [client deviceBecameActive:@(17)];
    [self waitForQueue:deviceDelegateQueue];

    XCTAssertEqual(deviceDelegate.attributeReportCount, 1);
    XCTAssertEqual(deviceDelegate.becameActiveCount, 1);
    @autoreleasepool {
        XCTAssertEqualObjects(controller.devices, @[ device ]);
    }
    [controller shutdown];
    [listener invalidate];
}

@end
