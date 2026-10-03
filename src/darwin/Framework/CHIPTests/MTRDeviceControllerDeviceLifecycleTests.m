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
#import <os/lock.h>
#import <stdatomic.h>

#import "MTRDeviceControllerLocalTestStorage.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"

static const uint16_t kTestVendorId = 0xFFF1u;
static const NSUInteger kLookupCount = 500;
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

@interface MTRDeviceLifecycleTestsCountedDelegate : MTRDeviceLifecycleTestsControllerDelegate
+ (NSInteger)liveCount;
@end

static atomic_long sCountedDelegateLiveCount;

@implementation MTRDeviceLifecycleTestsCountedDelegate

+ (NSInteger)liveCount
{
    return atomic_load(&sCountedDelegateLiveCount);
}

- (instancetype)init
{
    if (self = [super init]) {
        atomic_fetch_add(&sCountedDelegateLiveCount, 1);
    }
    return self;
}

- (void)dealloc
{
    atomic_fetch_sub(&sCountedDelegateLiveCount, 1);
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

@interface MTRDeviceLifecycleTestsSilentDelegate : NSObject <MTRDeviceControllerDelegate>
@end

@implementation MTRDeviceLifecycleTestsSilentDelegate
@end

@interface MTRDeviceLifecycleTestsLockProbingDelegate : MTRDeviceLifecycleTestsControllerDelegate
@property (nonatomic, weak) MTRDeviceController * controller;
@property (atomic, readonly) NSUInteger callsUnderDeviceMapLock;
@end

@interface MTRDeviceController (DeviceLifecycleTestsLock)
@property (readonly, assign) os_unfair_lock_t deviceMapLock;
@end

@implementation MTRDeviceLifecycleTestsLockProbingDelegate {
    NSUInteger _callsUnderDeviceMapLock;
}

- (NSUInteger)callsUnderDeviceMapLock
{
    @synchronized(self) {
        return _callsUnderDeviceMapLock;
    }
}

- (BOOL)respondsToSelector:(SEL)selector
{
    MTRDeviceController * controller = self.controller;
    if (selector == @selector(devicesChangedForController:) && controller != nil) {
        if (os_unfair_lock_trylock(controller.deviceMapLock)) {
            os_unfair_lock_unlock(controller.deviceMapLock);
        } else {
            @synchronized(self) {
                ++_callsUnderDeviceMapLock;
            }
        }
    }
    return [super respondsToSelector:selector];
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
    NSUInteger deviceCount = 0;
    for (NSUInteger i = 0; i < 1000; ++i) {
        @autoreleasepool {
            deviceCount = controller.devices.count;
        }
        if (deviceCount == 0) {
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

- (void)testDevicesChangedFiresForEveryTransientDeviceCreateAndDealloc
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_suspend(delegateQueue);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    for (NSUInteger i = 0; i < kLookupCount; ++i) {
        @autoreleasepool {
            XCTAssertNotNil([MTRDevice deviceWithNodeID:@(17) controller:controller]);
        }
        [self waitForNoDevicesOnController:controller];
    }

    dispatch_resume(delegateQueue);
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(delegate.devicesChangedCount, 2 * kLookupCount);
    [controller shutdown];
}

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

- (void)testDevicesChangedIsNotQueuedForDelegateWithoutDevicesChanged
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    dispatch_suspend(delegateQueue);

    __weak MTRDeviceLifecycleTestsSilentDelegate * weakDelegate;
    @autoreleasepool {
        __auto_type * delegate = [[MTRDeviceLifecycleTestsSilentDelegate alloc] init];
        weakDelegate = delegate;
        [controller addDeviceControllerDelegate:delegate queue:delegateQueue];
        XCTAssertNotNil([MTRDevice deviceWithNodeID:@(17) controller:controller]);
    }
    XCTAssertNil(weakDelegate);

    dispatch_resume(delegateQueue);
    [self waitForQueue:delegateQueue];
    [controller shutdown];
}

- (void)testDevicesChangedDelegateIsNotConsultedUnderDeviceMapLock
{
    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    __auto_type * delegate = [[MTRDeviceLifecycleTestsLockProbingDelegate alloc] init];
    delegate.controller = controller;
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    MTRDevice * device = [MTRDevice deviceWithNodeID:@(17) controller:controller];
    XCTAssertNotNil(device);
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(delegate.callsUnderDeviceMapLock, 0);
    XCTAssertEqual(delegate.devicesChangedCount, 1);
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

- (void)testXPCReportForNodeWithoutDeviceDoesNotCreateDevice
{
    NSXPCListener * listener = [NSXPCListener anonymousListener];
    MTRDeviceController * controller = [self startXPCControllerWithListener:listener];

    __auto_type * delegate = [[MTRDeviceLifecycleTestsControllerDelegate alloc] init];
    dispatch_queue_t delegateQueue = dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL);
    [controller addDeviceControllerDelegate:delegate queue:delegateQueue];

    id<MTRXPCClientProtocol> client = (id<MTRXPCClientProtocol>) controller;
    for (NSUInteger i = 0; i < kLookupCount; ++i) {
        @autoreleasepool {
            [client device:@(17) stateChanged:MTRDeviceStateReachable];
            [client device:@(17) receivedAttributeReport:@[]];
            [client device:@(17) receivedEventReport:@[]];
            [client deviceBecameActive:@(17)];
            [client deviceCachePrimed:@(17)];
            [client deviceConfigurationChanged:@(17)];
            [client device:@(17) internalStateUpdated:@{}];
        }
    }
    [self waitForQueue:delegateQueue];

    XCTAssertEqual(controller.devices.count, 0);
    XCTAssertEqual(delegate.devicesChangedCount, 0);
    [controller shutdown];
    [listener invalidate];
}

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

- (void)testDevicesChangedDeliveryUnderConcurrentDelegateAndDeviceChurn
{
    static const size_t kWorkerCount = 6;
    static const NSUInteger kIterationsPerWorker = 1000;
    static const NSUInteger kPersistentDelegateCount = 4;

    __auto_type * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    MTRDeviceController * controller = [self startControllerWithStorage:storage];
    NSMutableArray<dispatch_queue_t> * queues = [NSMutableArray array];
    for (size_t i = 0; i < kWorkerCount; ++i) {
        [queues addObject:dispatch_queue_create("test.delegate.queue", DISPATCH_QUEUE_SERIAL)];
    }

    @autoreleasepool {
        NSMutableArray<MTRDeviceLifecycleTestsCountedDelegate *> * delegates = [NSMutableArray array];
        for (NSUInteger i = 0; i < kPersistentDelegateCount; ++i) {
            __auto_type * delegate = [[MTRDeviceLifecycleTestsCountedDelegate alloc] init];
            [delegates addObject:delegate];
            [controller addDeviceControllerDelegate:delegate queue:queues[i]];
        }

        dispatch_apply(kWorkerCount, DISPATCH_APPLY_AUTO, ^(size_t worker) {
            for (NSUInteger i = 0; i < kIterationsPerWorker; ++i) {
                @autoreleasepool {
                    __auto_type * transientDelegate = [[MTRDeviceLifecycleTestsCountedDelegate alloc] init];
                    [controller addDeviceControllerDelegate:transientDelegate queue:queues[(worker + i) % kWorkerCount]];
                    [MTRDevice deviceWithNodeID:@(100 + (worker + i) % 4) controller:controller];
                    if (i % 2) {
                        [controller removeDeviceControllerDelegate:transientDelegate];
                    }
                }
            }
        });

        [self waitForNoDevicesOnController:controller];
        for (dispatch_queue_t queue in queues) {
            [self waitForQueue:queue];
        }
        XCTAssertEqual([MTRDeviceLifecycleTestsCountedDelegate liveCount], (NSInteger) kPersistentDelegateCount);

        NSArray<NSNumber *> * countsBefore = [delegates valueForKey:@"devicesChangedCount"];
        @autoreleasepool {
            MTRDevice * device = [MTRDevice deviceWithNodeID:@(17) controller:controller];
            for (dispatch_queue_t queue in queues) {
                [self waitForQueue:queue];
            }
            for (NSUInteger i = 0; i < kPersistentDelegateCount; ++i) {
                XCTAssertEqual(delegates[i].devicesChangedCount, countsBefore[i].unsignedIntegerValue + 1);
            }
            device = nil;
        }
        [self waitForNoDevicesOnController:controller];
        for (dispatch_queue_t queue in queues) {
            [self waitForQueue:queue];
        }
        for (NSUInteger i = 0; i < kPersistentDelegateCount; ++i) {
            XCTAssertEqual(delegates[i].devicesChangedCount, countsBefore[i].unsignedIntegerValue + 2);
            [controller removeDeviceControllerDelegate:delegates[i]];
        }
    }

    XCTAssertEqual([MTRDeviceLifecycleTestsCountedDelegate liveCount], 0);
    [controller shutdown];
}

@end
