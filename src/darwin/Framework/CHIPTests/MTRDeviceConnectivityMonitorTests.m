/**
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

#import <XCTest/XCTest.h>

#import <Network/Network.h>
#import <dns_sd.h>
#import <fcntl.h>
#import <stdatomic.h>
#import <sys/socket.h>

#import "MTRDeviceConnectivityMonitor.h"
#import "MTRDeviceTestDelegate.h"
#import "MTRTestCase+ServerAppRunner.h"
#import "MTRTestCase.h"
#import "MTRTestDeclarations.h"
#import "MTRTestKeys.h"
#import "MTRTestPerControllerStorage.h"
#import "MTRTestStorage.h"

@interface MTRDeviceConnectivityMonitor (Test)
- (instancetype)initWithInstanceName:(NSString *)instanceName;
- (NSUInteger)monitorID;
#ifdef DEBUG
+ (BOOL)unitTestHasActiveSharedConnection;
+ (NSUInteger)unitTestActiveMonitorCount;
+ (NSUInteger)unitTestSharedConnectionGeneration;
+ (int)unitTestSharedConnectionSocket;
+ (void)unitTestDisconnectSharedConnectionFromDaemon;
+ (void)unitTestCloseIdleSharedConnection;
#endif
@end

@interface MTRBaseDevice (ConnectivityMonitorTest)
- (void)_getRemoteMaxPathsPerInvokeWithQueue:(dispatch_queue_t)queue
                                  completion:(void (^)(uint16_t maxPathsPerInvoke, NSError * _Nullable error))completion;
@end

@interface MTRDeviceController (ConnectivityMonitorTest)
- (void)asyncDispatchToMatterQueue:(dispatch_block_t)block errorHandler:(nullable MTRDeviceErrorHandler)errorHandler;
@end

@interface MTRDeviceConnectivityMonitorTests : MTRTestCase
@end

@implementation MTRDeviceConnectivityMonitorTests {
    NSMutableArray<MTRDeviceTestDelegate *> * _deviceDelegates;
}

static DNSServiceRef sSharedConnection;

+ (void)setUp
{
    [super setUp];
    DNSServiceErrorType dnsError = DNSServiceCreateConnection(&sSharedConnection);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);
}

+ (void)tearDown
{
    DNSServiceRefDeallocate(sSharedConnection);
    [super tearDown];
}

- (void)setUp
{
    [super setUp];
    _deviceDelegates = [NSMutableArray array];
}

- (void)tearDown
{
    [super tearDown];
#ifdef DEBUG
    [MTRDeviceConnectivityMonitor unitTestCloseIdleSharedConnection];
#endif
}

static char kLocalDot[] = "local.";
static char kOperationalType[] = "_matter._tcp";

static NSString * DNSServiceErrorTypeToString(DNSServiceErrorType error)
{
    switch (error) {
    case kDNSServiceErr_NoError:
        return @"NoError";
    case kDNSServiceErr_Unknown:
        return @"Unknown";
    case kDNSServiceErr_NoSuchName:
        return @"NoSuchName";
    case kDNSServiceErr_NoMemory:
        return @"NoMemory";
    case kDNSServiceErr_BadParam:
        return @"BadParam";
    case kDNSServiceErr_BadReference:
        return @"BadReference";
    case kDNSServiceErr_BadState:
        return @"BadState";
    case kDNSServiceErr_BadFlags:
        return @"BadFlags";
    case kDNSServiceErr_Unsupported:
        return @"Unsupported";
    case kDNSServiceErr_NotInitialized:
        return @"NotInitialized";
    case kDNSServiceErr_AlreadyRegistered:
        return @"AlreadyRegistered";
    case kDNSServiceErr_NameConflict:
        return @"NameConflict";
    case kDNSServiceErr_Invalid:
        return @"Invalid";
    case kDNSServiceErr_Firewall:
        return @"Firewall";
    case kDNSServiceErr_Incompatible:
        return @"Incompatible";
    case kDNSServiceErr_BadInterfaceIndex:
        return @"BadInterfaceIndex";
    case kDNSServiceErr_Refused:
        return @"Refused";
    case kDNSServiceErr_NoSuchRecord:
        return @"NoSuchRecord";
    case kDNSServiceErr_NoAuth:
        return @"NoAuth";
    case kDNSServiceErr_NoSuchKey:
        return @"NoSuchKey";
    case kDNSServiceErr_NATTraversal:
        return @"NATTraversal";
    case kDNSServiceErr_DoubleNAT:
        return @"DoubleNAT";
    case kDNSServiceErr_BadTime:
        return @"BadTime";
    case kDNSServiceErr_BadSig:
        return @"BadSig";
    case kDNSServiceErr_BadKey:
        return @"BadKey";
    case kDNSServiceErr_Transient:
        return @"Transient";
    case kDNSServiceErr_ServiceNotRunning:
        return @"ServiceNotRunning";
    case kDNSServiceErr_NATPortMappingUnsupported:
        return @"NATPortMappingUnsupported";
    case kDNSServiceErr_NATPortMappingDisabled:
        return @"NATPortMappingDisabled";
    case kDNSServiceErr_NoRouter:
        return @"NoRouter";
    case kDNSServiceErr_PollingMode:
        return @"PollingMode";
    case kDNSServiceErr_Timeout:
        return @"Timeout";
    default:
        return [NSString stringWithFormat:@"Unknown(%d)", error];
    }
}

static void TestRegisterCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char * name,
    const char * regtype,
    const char * domain,
    void * context)
{
    if (errorCode != kDNSServiceErr_NoError) {
        NSLog(@"register callback ERROR: %@ for service %s.%s%s",
            DNSServiceErrorTypeToString(errorCode),
            name ? name : "(null)",
            regtype ? regtype : "(null)",
            domain ? domain : "(null)");
    } else {
        NSLog(@"register callback SUCCESS for service %s.%s%s",
            name ? name : "(null)",
            regtype ? regtype : "(null)",
            domain ? domain : "(null)");
    }
}

- (void)test001_BasicMonitorTest
{
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    DNSServiceRef testAdvertiser;
    DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
    char testInstanceName[] = "testinstance-name";
    char testHostName[] = "localhost";
    uint16_t testPort = htons(15000);
    DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, TestRegisterCallback, NULL);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);

    XCTestExpectation * connectivityMonitorCallbackExpectation = [self expectationWithDescription:@"Got connectivity monitor callback"];
    __block BOOL gotConnectivityMonitorCallback = NO;

    MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:@(testInstanceName)];
    [monitor startMonitoringWithHandler:^{
        if (!gotConnectivityMonitorCallback) {
            gotConnectivityMonitorCallback = YES;
            [connectivityMonitorCallbackExpectation fulfill];
        }
    } queue:testQueue];

    [self waitForExpectations:@[ connectivityMonitorCallbackExpectation ] timeout:5];

    [monitor stopMonitoring];
    DNSServiceRefDeallocate(testAdvertiser);
}

- (void)test002_MonitorStopAndRestart
{
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    DNSServiceRef testAdvertiser;
    DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
    char testInstanceName[] = "stop-restart-test";
    char testHostName[] = "localhost";
    uint16_t testPort = htons(15000);
    DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, TestRegisterCallback, NULL);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);

    MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:@(testInstanceName)];

    // First monitoring session
    XCTestExpectation * firstCallbackExpectation = [self expectationWithDescription:@"First monitoring callback"];
    __block BOOL firstCallbackReceived = NO;
    [monitor startMonitoringWithHandler:^{
        if (!firstCallbackReceived) {
            firstCallbackReceived = YES;
            [firstCallbackExpectation fulfill];
        }
    } queue:testQueue];

    [self waitForExpectations:@[ firstCallbackExpectation ] timeout:5.0];
    XCTAssertTrue(firstCallbackReceived, @"First monitoring session should receive callback");

    // Stop monitoring
    [monitor stopMonitoring];

    // Restart monitoring on the same monitor object
    XCTestExpectation * secondCallbackExpectation = [self expectationWithDescription:@"Second monitoring callback"];
    __block BOOL secondCallbackReceived = NO;
    [monitor startMonitoringWithHandler:^{
        if (!secondCallbackReceived) {
            secondCallbackReceived = YES;
            [secondCallbackExpectation fulfill];
        }
    } queue:testQueue];

    [self waitForExpectations:@[ secondCallbackExpectation ] timeout:5.0];
    XCTAssertTrue(secondCallbackReceived, @"Restarted monitoring should receive callback");

    [monitor stopMonitoring];
    DNSServiceRefDeallocate(testAdvertiser);
}

- (void)test003_MultipleMonitorsUniqueIDs
{
    const NSUInteger kNumMonitors = 12;
    NSMutableArray<MTRDeviceConnectivityMonitor *> * monitors = [NSMutableArray array];
    NSMutableSet<NSNumber *> * monitorIDs = [NSMutableSet set];

    // Create multiple monitors and verify they all get unique IDs
    for (NSUInteger i = 0; i < kNumMonitors; i++) {
        NSString * instanceName = [NSString stringWithFormat:@"test-monitor-%lu", (unsigned long) i];
        MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:instanceName];
        [monitors addObject:monitor];

        uintptr_t monitorID = [monitor monitorID];
        XCTAssertFalse([monitorIDs containsObject:@(monitorID)], @"Monitor ID %lu is not unique", (unsigned long) monitorID);
        [monitorIDs addObject:@(monitorID)];
    }

    // Verify we have the expected number of unique IDs
    XCTAssertEqual(monitorIDs.count, kNumMonitors, @"Should have %lu unique monitor IDs", (unsigned long) kNumMonitors);

    // Clean up - monitors will be deallocated when array is released
    [monitors removeAllObjects];
}

- (void)test004_EarlyMonitorDeallocation
{
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    DNSServiceRef testAdvertiser;
    DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
    char testInstanceName[] = "early-dealloc-test";
    char testHostName[] = "localhost";
    uint16_t testPort = htons(15001);

    DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, TestRegisterCallback, NULL);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);

    @autoreleasepool {
        MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:@(testInstanceName)];
        uintptr_t monitorID = [monitor monitorID];

        // Start monitoring but don't wait for callback
        [monitor startMonitoringWithHandler:^{
            // This callback might fire after monitor is deallocated
            NSLog(@"Callback fired for monitor ID %lu (should be safe)", (unsigned long) monitorID);
        } queue:testQueue];

        // Monitor will be deallocated when this autoreleasepool drains
        // Any subsequent DNS callbacks should safely find nil in the map
    }

    // Give some time for potential callbacks to fire (they should be safely ignored)
    XCTestExpectation * waitExpectation = [self expectationWithDescription:@"Wait for potential callbacks"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [waitExpectation fulfill];
    });
    [self waitForExpectations:@[ waitExpectation ] timeout:2.0];

    DNSServiceRefDeallocate(testAdvertiser);
}

- (void)test005_RapidCreationDestruction
{
    const NSUInteger kNumCycles = 20;
    NSMutableSet<NSNumber *> * usedIDs = [NSMutableSet set];

    // Rapidly create and destroy monitors to test map cleanup
    for (NSUInteger cycle = 0; cycle < kNumCycles; cycle++) {
        @autoreleasepool {
            NSString * instanceName = [NSString stringWithFormat:@"rapid-test-%lu", (unsigned long) cycle];
            MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:instanceName];

            uintptr_t monitorID = [monitor monitorID];
            XCTAssertFalse([usedIDs containsObject:@(monitorID)], @"Monitor ID %lu was reused too quickly", (unsigned long) monitorID);
            [usedIDs addObject:@(monitorID)];

            // Monitor deallocated when autoreleasepool drains
        }
    }

    // All monitors should be deallocated and map entries cleaned up
    // Create one more monitor to verify IDs are still being assigned properly
    MTRDeviceConnectivityMonitor * finalMonitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:@"final-test"];
    uintptr_t finalID = [finalMonitor monitorID];
    XCTAssertNotEqual(finalID, 0, @"Final monitor should have valid ID");

    // Clean up
    finalMonitor = nil;
}

- (void)test006_CallbackSafetyDuringDeallocation
{
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    const NSUInteger kNumMonitors = 5;
    NSMutableArray<NSValue *> * advertisers = [NSMutableArray array];

    // Create multiple DNS services
    for (NSUInteger i = 0; i < kNumMonitors; i++) {
        DNSServiceRef testAdvertiser;
        DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
        char testInstanceName[64];
        snprintf(testInstanceName, sizeof(testInstanceName), "callback-safety-test-%lu", (unsigned long) i);
        char testHostName[] = "localhost";
        uint16_t testPort = htons(15002 + i);

        DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, TestRegisterCallback, NULL);
        XCTAssertEqual(dnsError, kDNSServiceErr_NoError);
        [advertisers addObject:[NSValue valueWithPointer:testAdvertiser]];
    }

    __block NSUInteger callbackCount = 0;
    NSMutableArray<NSNumber *> * monitorIDs = [NSMutableArray array];

    @autoreleasepool {
        // Create monitors and start monitoring
        for (NSUInteger i = 0; i < kNumMonitors; i++) {
            NSString * instanceName = [NSString stringWithFormat:@"callback-safety-test-%lu", (unsigned long) i];
            MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:instanceName];
            uintptr_t monitorID = [monitor monitorID];
            [monitorIDs addObject:@(monitorID)];

            [monitor startMonitoringWithHandler:^{
                // This callback might fire after monitor is deallocated
                @synchronized(self) {
                    callbackCount++;
                }
                NSLog(@"Safe callback fired for monitor ID %lu", (unsigned long) monitorID);
            } queue:testQueue];
        }

        // Monitors will be deallocated when this autoreleasepool drains
        // Some callbacks may still be in-flight and should be safely handled
    }

    // Wait for potential callbacks to fire
    XCTestExpectation * waitExpectation = [self expectationWithDescription:@"Wait for potential callbacks"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (2.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [waitExpectation fulfill];
    });
    [self waitForExpectations:@[ waitExpectation ] timeout:3.0];

    NSLog(@"Callback safety test completed - %lu callbacks fired safely", (unsigned long) callbackCount);

    // Clean up DNS services
    for (NSValue * advertiserValue in advertisers) {
        DNSServiceRef advertiser = (DNSServiceRef)[advertiserValue pointerValue];
        DNSServiceRefDeallocate(advertiser);
    }
}

- (void)test007_SharedConnectionCleanupAfterLinger
{
#ifdef DEBUG
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    DNSServiceRef testAdvertiser;
    DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
    char testInstanceName[] = "cleanup-linger-test";
    char testHostName[] = "localhost";
    uint16_t testPort = htons(15010);

    DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, TestRegisterCallback, NULL);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);

    MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:@(testInstanceName)];

    XCTestExpectation * callbackExpectation = [self expectationWithDescription:@"Monitor callback"];
    __block BOOL gotCallback = NO;
    [monitor startMonitoringWithHandler:^{
        if (!gotCallback) {
            gotCallback = YES;
            [callbackExpectation fulfill];
        }
    } queue:testQueue];

    [self waitForExpectations:@[ callbackExpectation ] timeout:5.0];

    // Stop monitoring - this should trigger the linger timer (10 seconds)
    [monitor stopMonitoring];

    // Shared connection should still exist within linger window
    XCTestExpectation * withinLingerExpectation = [self expectationWithDescription:@"Check within linger window"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertTrue([MTRDeviceConnectivityMonitor unitTestHasActiveSharedConnection],
            @"Shared connection should still exist within linger window");
        [withinLingerExpectation fulfill];
    });
    [self waitForExpectations:@[ withinLingerExpectation ] timeout:2.0];

    // After linger interval (10 sec) + buffer, shared connection should be cleaned up
    XCTestExpectation * afterLingerExpectation = [self expectationWithDescription:@"Check after linger interval"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (11.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        XCTAssertFalse([MTRDeviceConnectivityMonitor unitTestHasActiveSharedConnection],
            @"Shared connection should be cleaned up after linger interval");
        [afterLingerExpectation fulfill];
    });
    [self waitForExpectations:@[ afterLingerExpectation ] timeout:13.0];

    DNSServiceRefDeallocate(testAdvertiser);
#else
    XCTSkip(@"Test requires DEBUG build for accessing shared connection state");
#endif
}

#ifdef DEBUG
static const NSTimeInterval kMonitorWaitSeconds = 2;
static const NSTimeInterval kPromptSeconds = 5;
static const NSTimeInterval kLingerAndMarginSeconds = 15;
static const NSTimeInterval kShortMonitorWaitSeconds = 0.01;
static const NSTimeInterval kShortMonitorWaitElapsedSeconds = 0.5;

- (BOOL)waitUntil:(BOOL (^)(void))condition timeout:(NSTimeInterval)timeout description:(NSString *)description
{
    NSPredicate * predicate = [NSPredicate predicateWithBlock:^BOOL(id _Nullable object, NSDictionary * _Nullable bindings) {
        return condition();
    }];
    XCTNSPredicateExpectation * expectation = [[XCTNSPredicateExpectation alloc] initWithPredicate:predicate object:nil];
    expectation.expectationDescription = description;
    if ([XCTWaiter waitForExpectations:@[ expectation ] timeout:timeout] != XCTWaiterResultCompleted) {
        XCTFail(@"Timed out after %.0fs: %@", timeout, description);
        return NO;
    }
    return YES;
}

- (BOOL)stays:(BOOL (^)(void))condition duration:(NSTimeInterval)duration
{
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:duration];
    while ([deadline timeIntervalSinceNow] > 0) {
        if (!condition()) {
            return NO;
        }
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
    }
    return YES;
}

- (DNSServiceRef)advertiseInstance:(NSString *)instanceName port:(uint16_t)port
{
    DNSServiceRef advertiser = NULL;
    DNSServiceErrorType dnsError = DNSServiceRegister(&advertiser, kDNSServiceFlagsNoAutoRename, 0, instanceName.UTF8String, kOperationalType, kLocalDot, "localhost", htons(port), 0, NULL, TestRegisterCallback, NULL);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);
    return advertiser;
}

- (MTRDeviceConnectivityMonitor *)startedMonitorForInstance:(NSString *)instanceName reports:(NSMutableArray *)reports
{
    NSUInteger countBefore = [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount];
    MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:instanceName];
    XCTAssertTrue([monitor startMonitoringWithHandler:^{
        [reports addObject:@YES];
    } queue:dispatch_get_main_queue()]);
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount], countBefore + 1);
    return monitor;
}

- (BOOL)monitorReportsForAdvertisedInstance:(NSString *)instanceName port:(uint16_t)port
{
    DNSServiceRef advertiser = [self advertiseInstance:instanceName port:port];
    NSMutableArray * reports = [NSMutableArray array];
    MTRDeviceConnectivityMonitor * monitor = [self startedMonitorForInstance:instanceName reports:reports];
    BOOL result = [self waitUntil:^{ return (BOOL) (reports.count > 0); } timeout:kPromptSeconds description:[NSString stringWithFormat:@"monitor reports for advertised %@", instanceName]];
    [monitor stopMonitoring];
    DNSServiceRefDeallocate(advertiser);
    return result;
}

- (int)closedIdleSharedConnectionSocket
{
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"idle-disconnect-precondition" port:15030]);
    int fd = [MTRDeviceConnectivityMonitor unitTestSharedConnectionSocket];
    XCTAssertGreaterThanOrEqual(fd, 0);
    [MTRDeviceConnectivityMonitor unitTestDisconnectSharedConnectionFromDaemon];
    [self waitUntil:^{ return (BOOL) (fcntl(fd, F_GETFD) == -1); } timeout:kPromptSeconds description:@"dns_sd closed the idle connection's socket"];
    return fd;
}

- (MTRDeviceController *)controllerWithShortMonitorWait
{
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    [controller unitTestSetConnectivityMonitorWaitSeconds:kMonitorWaitSeconds];
    return controller;
}

- (MTRDevice *)deviceWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller usesThread:(BOOL)usesThread
{
    __auto_type * delegate = [[MTRDeviceTestDelegateWithSubscriptionSetupOverride alloc] init];
    delegate.skipSetupSubscription = YES;
    delegate.pretendThreadEnabled = usesThread;
    MTRDevice * device = [MTRDevice deviceWithNodeID:nodeID controller:controller];
    [device setDelegate:delegate queue:dispatch_get_main_queue()];
    [_deviceDelegates addObject:delegate];
    return device;
}

- (void)invokeToggleOnDevice:(MTRDevice *)device completion:(dispatch_block_t)completion
{
    [device invokeCommandWithEndpointID:@(1)
                              clusterID:@(MTRClusterIDTypeOnOffID)
                              commandID:@(MTRCommandIDTypeClusterOnOffCommandToggleID)
                          commandFields:@{ MTRTypeKey : MTRStructureValueType, MTRValueKey : @[] }
                         expectedValues:nil
                  expectedValueInterval:nil
                                  queue:dispatch_get_main_queue()
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 completion();
                             }];
}

- (MTRDevice *)startThreadSessionRequestOnController:(MTRDeviceController *)controller nodeID:(NSNumber *)nodeID invokeCompletions:(NSMutableArray *)invokeCompletions
{
    NSUInteger countBefore = [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount];
    MTRDevice * device = [self deviceWithNodeID:nodeID controller:controller usesThread:YES];
    [self invokeToggleOnDevice:device completion:^{
        [invokeCompletions addObject:@YES];
    }];
    [self waitUntil:^{ return (BOOL) ([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount] > countBefore || controller.concurrentSubscriptionPool.itemCount > 0); }
            timeout:kPromptSeconds
        description:@"session request started its connectivity monitor"];
    return device;
}
#endif

- (void)test008_ThreadSessionProceedsWhenMonitorNeverReports
{
#ifdef DEBUG
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"session-precondition-008" port:15020]);
    MTRDeviceController * controller = [self controllerWithShortMonitorWait];
    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:@(0x1001) invokeCompletions:invokeCompletions];

    XCTAssertEqual(controller.concurrentSubscriptionPool.itemCount, 0, @"session request should be waiting on the monitor");
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (controller.concurrentSubscriptionPool.itemCount > 0); }
                          timeout:kMonitorWaitSeconds + kPromptSeconds
                      description:@"session work item enqueued although the connectivity monitor never reported"]);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test009_StuckMonitorIsStoppedAndSharedConnectionLingersOut
{
#ifdef DEBUG
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"session-precondition-009" port:15021]);
    MTRDeviceController * controller = [self controllerWithShortMonitorWait];
    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:@(0x1002) invokeCompletions:invokeCompletions];
    NSUInteger generation = [MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration];
    XCTAssertNotEqual(generation, 0);

    XCTAssertTrue([self waitUntil:^{ return (BOOL) ([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount] == 0); }
                          timeout:kMonitorWaitSeconds + kPromptSeconds
                      description:@"monitor stopped after the wait timed out"]);
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration], generation, @"timeout must not replace the shared connection");
    XCTAssertTrue([self waitUntil:^{ return (BOOL) ![MTRDeviceConnectivityMonitor unitTestHasActiveSharedConnection]; }
                          timeout:kLingerAndMarginSeconds
                      description:@"shared resolver connection released by the linger timer"]);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test010_ThreadSessionProceedsPromptlyWhenMonitorReports
{
#ifdef DEBUG
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    NSNumber * nodeID = @(0x1003);
    NSString * instanceName = [NSString stringWithFormat:@"%016llX-%016llX", controller.compressedFabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue];
    DNSServiceRef advertiser = [self advertiseInstance:instanceName port:15022];

    MTRDevice * device = [self deviceWithNodeID:nodeID controller:controller usesThread:YES];
    __block BOOL invokeCompleted = NO;
    [self invokeToggleOnDevice:device completion:^{
        invokeCompleted = YES;
    }];

    XCTAssertTrue([self waitUntil:^{ return (BOOL) (controller.concurrentSubscriptionPool.itemCount > 0 || invokeCompleted); }
                          timeout:kPromptSeconds
                      description:@"session work item enqueued as soon as the monitor reported"]);
    DNSServiceRefDeallocate(advertiser);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test011_NonThreadSessionBypassesMonitor
{
#ifdef DEBUG
    MTRDeviceController * controller = [self controllerWithShortMonitorWait];
    NSUInteger countBefore = [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount];
    MTRDevice * device = [self deviceWithNodeID:@(0x1004) controller:controller usesThread:NO];
    [self invokeToggleOnDevice:device completion:^ {}];

    XCTAssertTrue([self stays:^{ return (BOOL) (controller.concurrentSubscriptionPool.itemCount == 0 && [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount] == countBefore); } duration:kMonitorWaitSeconds + 1], @"non-Thread session request must not use the monitor or the subscription pool");
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test012_DaemonDisconnectUnblocksWaitersAndRebuildsSharedConnection
{
#ifdef DEBUG
    MTRDeviceController * controller = [self createControllerOnTestFabric];
    NSMutableArray * waitingMonitorReports = [NSMutableArray array];
    MTRDeviceConnectivityMonitor * waitingMonitor = [self startedMonitorForInstance:@"daemon-disconnect-waiting" reports:waitingMonitorReports];
    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:@(0x1006) invokeCompletions:invokeCompletions];
    NSUInteger generation = [MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration];
    XCTAssertEqual(controller.concurrentSubscriptionPool.itemCount, 0);
    XCTAssertEqual(waitingMonitorReports.count, 0);

    [MTRDeviceConnectivityMonitor unitTestDisconnectSharedConnectionFromDaemon];

    XCTAssertTrue([self waitUntil:^{ return (BOOL) (waitingMonitorReports.count > 0); } timeout:kPromptSeconds description:@"waiting monitor unblocked by daemon disconnect"]);
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (controller.concurrentSubscriptionPool.itemCount > 0); }
                          timeout:kPromptSeconds
                      description:@"waiting Thread session request proceeds after daemon disconnect"]);
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount], 0, @"monitors on the lost connection are stopped");
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"daemon-disconnect-rebuilt" port:15024], @"shared resolver connection rebuilt after daemon disconnect");
    XCTAssertGreaterThan([MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration], generation);
    XCTAssertEqual(waitingMonitorReports.count, 1);
    [waitingMonitor stopMonitoring];
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test013_OfflineNodeTimeoutKeepsHealthySharedConnection
{
#ifdef DEBUG
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"offline-node-precondition" port:15026]);
    NSString * otherInstanceName = [NSString stringWithFormat:@"offline-node-other-%@", NSUUID.UUID.UUIDString];
    NSMutableArray * otherReports = [NSMutableArray array];
    MTRDeviceConnectivityMonitor * otherMonitor = [self startedMonitorForInstance:otherInstanceName reports:otherReports];
    NSUInteger generation = [MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration];
    MTRDeviceController * controller = [self controllerWithShortMonitorWait];

    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:@(0x1005) invokeCompletions:invokeCompletions];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (controller.concurrentSubscriptionPool.itemCount > 0); }
                          timeout:kMonitorWaitSeconds + kPromptSeconds
                      description:@"session work item enqueued after the wait timed out"]);

    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration], generation, @"offline node must not replace a healthy shared connection");
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount], 1, @"other monitor keeps its resolves");
    XCTAssertEqual(otherReports.count, 0);
    DNSServiceRef otherAdvertiser = [self advertiseInstance:otherInstanceName port:15027];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (otherReports.count > 0); } timeout:kPromptSeconds description:@"other monitor still resolving on the same connection"]);
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration], generation);
    [otherMonitor stopMonitoring];
    DNSServiceRefDeallocate(otherAdvertiser);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test014_MonitorsDeallocatedDuringDaemonDisconnect
{
#ifdef DEBUG
    dispatch_queue_t releaseQueue = dispatch_queue_create("connectivity-monitor-release", DISPATCH_QUEUE_CONCURRENT);
    for (NSUInteger iteration = 0; iteration < 10; iteration++) {
        dispatch_group_t group = dispatch_group_create();
        @autoreleasepool {
            for (NSUInteger i = 0; i < 8; i++) {
                __block MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:[NSString stringWithFormat:@"dealloc-disconnect-%lu-%lu", (unsigned long) iteration, (unsigned long) i]];
                XCTAssertTrue([monitor startMonitoringWithHandler:^{
                } queue:dispatch_get_main_queue()]);
                dispatch_group_async(group, releaseQueue, ^{
                    monitor = nil;
                });
            }
            [MTRDeviceConnectivityMonitor unitTestDisconnectSharedConnectionFromDaemon];
        }
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
        XCTAssertTrue([self waitUntil:^{ return (BOOL) ([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount] == 0); } timeout:kPromptSeconds description:@"all monitors gone"]);
    }
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"dealloc-disconnect-after" port:15029]);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test015_IdleSharedConnectionDiscardedAfterDaemonDisconnect
{
#ifdef DEBUG
    [self closedIdleSharedConnectionSocket];
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"idle-disconnect-after" port:15031], @"monitor started on an idle connection whose daemon went away still resolves");
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test016_IdleSharedConnectionDiscardedWhenItsDescriptorIsReused
{
#ifdef DEBUG
    int fd = [self closedIdleSharedConnectionSocket];
    NSMutableArray<NSNumber *> * sockets = [NSMutableArray array];
    int reusingPair[2] = { -1, -1 };
    while (sockets.count < 512 && reusingPair[0] != fd && reusingPair[1] != fd) {
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, reusingPair) != 0) {
            XCTFail(@"socketpair failed: %d", errno);
            break;
        }
        [sockets addObject:@(reusingPair[0])];
        [sockets addObject:@(reusingPair[1])];
    }
    XCTAssertTrue(reusingPair[0] == fd || reusingPair[1] == fd, @"descriptor of the closed connection reused by another socket");

    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"descriptor-reused-after" port:15032], @"monitor does not use a connection whose descriptor now belongs to another socket");
    for (NSNumber * socket in sockets) {
        close(socket.intValue);
    }
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test017_ExistingSessionUsedWhenMonitorNeverReports
{
#if defined(DEBUG) && HAVE_NSTASK
    const NSTimeInterval kInvokeOnDeadPeerSeconds = 60;
    MTRDeviceController * controller = [self controllerWithShortMonitorWait];
    NSNumber * nodeID = @(0x1007);
    MTRTestCaseServerApp * app = [self startCommissionedAppWithName:@"all-clusters" arguments:@[] controller:controller nodeID:nodeID];
    XCTAssertNotNil(app);
    [app terminate];

    __block MTRAsyncWorkCompletionBlock releasePoolSlot;
    MTRAsyncWorkItem * poolSlotHolder = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_main_queue()];
    [poolSlotHolder setReadyHandler:^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        releasePoolSlot = completion;
    }];
    [controller.concurrentSubscriptionPool enqueueWorkItem:poolSlotHolder description:@"hold the only subscription pool slot"];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (releasePoolSlot != nil); } timeout:kPromptSeconds description:@"subscription pool slot held"]);

    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:nodeID invokeCompletions:invokeCompletions];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (invokeCompletions.count > 0); }
                          timeout:kInvokeOnDeadPeerSeconds
                      description:@"invoke sent on the existing session without waiting for a subscription pool slot"]);
    XCTAssertNotNil(releasePoolSlot);
    if (releasePoolSlot) {
        releasePoolSlot(MTRAsyncWorkComplete);
    }
#else
    XCTSkip(@"Requires DEBUG test hooks and NSTask");
#endif
}

#ifdef DEBUG
static const uint16_t kControllerPeerPort = 5547;
static const NSTimeInterval kCASEEstablishmentSeconds = 30;

- (MTRDeviceController *)startControllerWithRootKeys:(MTRTestKeys *)rootKeys nodeID:(NSNumber *)nodeID poolSize:(NSUInteger)poolSize
{
    NSError * error;
    NSData * root = [MTRCertificates createRootCertificate:rootKeys issuerID:@(1) fabricID:nil error:&error];
    XCTAssertNil(error);
    MTRTestKeys * operationalKeys = [[MTRTestKeys alloc] init];
    SecKeyRef operationalPublicKey = [operationalKeys copyPublicKey];
    NSData * operational = [MTRCertificates createOperationalCertificate:rootKeys signingCertificate:root operationalPublicKey:operationalPublicKey fabricID:@(1) nodeID:nodeID caseAuthenticatedTags:nil error:&error];
    CFRelease(operationalPublicKey);
    XCTAssertNil(error);

    MTRTestPerControllerStorage * storage = [[MTRTestPerControllerStorage alloc] initWithControllerID:[NSUUID UUID]];
    __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:storage storageDelegateQueue:dispatch_queue_create("connectivity-monitor-test-storage", DISPATCH_QUEUE_SERIAL) uniqueIdentifier:storage.controllerID ipk:rootKeys.ipk vendorID:@(0xFFF1) operationalKeypair:operationalKeys operationalCertificate:operational intermediateCertificate:nil rootCertificate:root];
    if (poolSize) {
        params.concurrentSubscriptionEstablishmentsAllowedOnThread = poolSize;
    }
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
    XCTAssertNil(error);
    XCTAssertTrue(controller.running);
    return controller;
}

- (MTRDeviceController *)controllerWithSessionToControllerPeer:(NSNumber *)peerNodeID
{
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:[[MTRTestStorage alloc] init]];
    // hasStorage is private and initWithoutStorage is direct; per-controller storage, but on a port the test can advertise.
    [factoryParams setValue:@NO forKey:@"hasStorage"];
    factoryParams.shouldStartServer = YES;
    factoryParams.port = @(kControllerPeerPort);
    XCTAssertTrue([MTRDeviceControllerFactory.sharedInstance startControllerFactory:factoryParams error:nil]);
    [self addTeardownBlock:^{
        [MTRDeviceControllerFactory.sharedInstance stopControllerFactory];
    }];
    MTRTestKeys * rootKeys = [[MTRTestKeys alloc] init];
    [self startControllerWithRootKeys:rootKeys nodeID:peerNodeID poolSize:0];
    MTRDeviceController * controller = [self startControllerWithRootKeys:rootKeys nodeID:@(peerNodeID.unsignedLongLongValue + 1) poolSize:1];
    [controller unitTestSetConnectivityMonitorWaitSeconds:kMonitorWaitSeconds];

    NSString * peerInstanceName = [NSString stringWithFormat:@"%016llX-%016llX", controller.compressedFabricID.unsignedLongLongValue, peerNodeID.unsignedLongLongValue];
    DNSServiceRef peerAdvertiser = NULL;
    XCTAssertEqual(DNSServiceRegister(&peerAdvertiser, kDNSServiceFlagsNoAutoRename, 0, peerInstanceName.UTF8String, kOperationalType, kLocalDot, NULL, htons(kControllerPeerPort), 0, NULL, TestRegisterCallback, NULL), kDNSServiceErr_NoError);
    __block BOOL readCompleted = NO;
    [[MTRBaseDevice deviceWithNodeID:peerNodeID controller:controller] readAttributePaths:@[ [MTRAttributeRequestPath requestPathWithEndpointID:@(0) clusterID:@(MTRClusterIDTypeDescriptorID) attributeID:@(MTRAttributeIDTypeClusterDescriptorAttributePartsListID)] ]
                                                                               eventPaths:nil
                                                                                   params:nil
                                                                                    queue:dispatch_get_main_queue()
                                                                               completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                                                                   XCTAssertNil(error);
                                                                                   readCompleted = YES;
                                                                               }];
    XCTAssertTrue([self waitUntil:^{ return readCompleted; } timeout:kCASEEstablishmentSeconds description:@"CASE session to the peer established"]);
    DNSServiceRefDeallocate(peerAdvertiser);
    return controller;
}
#endif

- (void)test018_ExistingSessionToControllerPeerUsedWhenMonitorNeverReports
{
#ifdef DEBUG
    NSNumber * peerNodeID = @(0x1008);
    MTRDeviceController * controller = [self controllerWithSessionToControllerPeer:peerNodeID];
    __block MTRAsyncWorkCompletionBlock releasePoolSlot;
    MTRAsyncWorkItem * poolSlotHolder = [[MTRAsyncWorkItem alloc] initWithQueue:dispatch_get_main_queue()];
    [poolSlotHolder setReadyHandler:^(id context, NSInteger retryCount, MTRAsyncWorkCompletionBlock completion) {
        releasePoolSlot = completion;
    }];
    [controller.concurrentSubscriptionPool enqueueWorkItem:poolSlotHolder description:@"hold the only subscription pool slot"];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (releasePoolSlot != nil); } timeout:kPromptSeconds description:@"subscription pool slot held"]);

    NSMutableArray * invokeCompletions = [NSMutableArray array];
    [self startThreadSessionRequestOnController:controller nodeID:peerNodeID invokeCompletions:invokeCompletions];
    XCTAssertTrue([self waitUntil:^{ return (BOOL) (invokeCompletions.count > 0); }
                          timeout:kMonitorWaitSeconds + kPromptSeconds
                      description:@"invoke answered on the existing session without waiting for a subscription pool slot"]);
    if (releasePoolSlot) {
        releasePoolSlot(MTRAsyncWorkComplete);
    }
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test019_MonitorStartedBeforeDnsSdNoticesDaemonDisconnectRetriesOnNewConnection
{
#ifdef DEBUG
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"resolve-not-running-precondition" port:15033]);
    for (NSUInteger iteration = 0; iteration < 5; iteration++) {
        NSUInteger generation = [MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration];
        XCTAssertNotEqual(generation, 0);
        [MTRDeviceConnectivityMonitor unitTestDisconnectSharedConnectionFromDaemon];
        NSString * instanceName = [NSString stringWithFormat:@"resolve-not-running-%lu", (unsigned long) iteration];
        NSMutableArray * reports = [NSMutableArray array];
        MTRDeviceConnectivityMonitor * monitor = [self startedMonitorForInstance:instanceName reports:reports];
        XCTAssertGreaterThan([MTRDeviceConnectivityMonitor unitTestSharedConnectionGeneration], generation);
        DNSServiceRef advertiser = [self advertiseInstance:instanceName port:(uint16_t) (15034 + iteration)];
        XCTAssertTrue([self waitUntil:^{ return (BOOL) (reports.count > 0); } timeout:kPromptSeconds description:@"monitor started during the disconnect resolves on the new connection"]);
        [monitor stopMonitoring];
        DNSServiceRefDeallocate(advertiser);
    }
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test020_MonitorReleasedWhileItsLostConnectionIsDiscardedDoesNotUnderflowCount
{
#ifdef DEBUG
    dispatch_queue_t releaseQueue = dispatch_queue_create("connectivity-monitor-release", DISPATCH_QUEUE_CONCURRENT);
    dispatch_queue_t contentionQueue = dispatch_queue_create("connectivity-monitor-contention", DISPATCH_QUEUE_CONCURRENT);
    for (NSUInteger iteration = 0; iteration < 20; iteration++) {
        __block atomic_bool contend = true;
        dispatch_group_t contention = dispatch_group_create();
        for (int i = 0; i < 2; i++) {
            dispatch_group_async(contention, contentionQueue, ^{
                while (atomic_load(&contend)) {
                    (void) [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount];
                }
            });
        }
        dispatch_group_t releases = dispatch_group_create();
        @autoreleasepool {
            for (NSUInteger i = 0; i < 32; i++) {
                __block MTRDeviceConnectivityMonitor * monitor = [[MTRDeviceConnectivityMonitor alloc] initWithInstanceName:[NSString stringWithFormat:@"released-during-discard-%lu-%lu", (unsigned long) iteration, (unsigned long) i]];
                XCTAssertTrue([monitor startMonitoringWithHandler:^{
                } queue:dispatch_get_main_queue()]);
                dispatch_group_async(releases, releaseQueue, ^{
                    usleep((useconds_t) (i * 1000));
                    monitor = nil;
                });
            }
            [MTRDeviceConnectivityMonitor unitTestDisconnectSharedConnectionFromDaemon];
        }
        dispatch_group_wait(releases, DISPATCH_TIME_FOREVER);
        atomic_store(&contend, false);
        dispatch_group_wait(contention, DISPATCH_TIME_FOREVER);
        NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:kPromptSeconds];
        while ([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount] != 0 && deadline.timeIntervalSinceNow > 0) {
            usleep(10000);
        }
        XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount], 0);
    }
    XCTAssertTrue([self monitorReportsForAdvertisedInstance:@"released-during-discard-after" port:15040]);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

- (void)test021_ExistingSessionHandedOverInTheSameMatterQueueTurnAsTheTimeout
{
#ifdef DEBUG
    NSNumber * peerNodeID = @(0x100A);
    MTRDeviceController * controller = [self controllerWithSessionToControllerPeer:peerNodeID];
    [controller unitTestSetConnectivityMonitorWaitSeconds:kShortMonitorWaitSeconds];
    [self deviceWithNodeID:peerNodeID controller:controller usesThread:YES];

    // The timeout is queued behind this hold, and the check right behind the timeout.
    dispatch_semaphore_t holding = dispatch_semaphore_create(0);
    dispatch_semaphore_t releaseHold = dispatch_semaphore_create(0);
    dispatch_queue_t resultQueue = dispatch_queue_create("session-result", DISPATCH_QUEUE_SERIAL);
    __block BOOL sessionDelivered = NO;
    __block BOOL checked = NO;
    __block BOOL deliveredByTimeoutTurn = NO;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        [controller syncRunOnWorkQueue:^{
            dispatch_semaphore_signal(holding);
            dispatch_semaphore_wait(releaseHold, DISPATCH_TIME_FOREVER);
            [controller asyncDispatchToMatterQueue:^{
                dispatch_sync(resultQueue, ^{
                    deliveredByTimeoutTurn = sessionDelivered;
                });
                checked = YES;
            } errorHandler:nil];
        } error:nil];
    });
    XCTAssertEqual(dispatch_semaphore_wait(holding, dispatch_time(DISPATCH_TIME_NOW, (int64_t) (kPromptSeconds * NSEC_PER_SEC))), 0);

    NSUInteger countBefore = [MTRDeviceConnectivityMonitor unitTestActiveMonitorCount];
    MTRBaseDevice * baseDevice = [MTRBaseDevice deviceWithNodeID:peerNodeID controller:controller];
    [baseDevice _getRemoteMaxPathsPerInvokeWithQueue:resultQueue completion:^(uint16_t maxPathsPerInvoke, NSError * _Nullable error) {
        XCTAssertNil(error);
        sessionDelivered = YES;
    }];
    XCTAssertEqual([MTRDeviceConnectivityMonitor unitTestActiveMonitorCount], countBefore + 1, @"session request waits on its connectivity monitor");
    [NSThread sleepForTimeInterval:kShortMonitorWaitElapsedSeconds];
    dispatch_semaphore_signal(releaseHold);

    XCTAssertTrue([self waitUntil:^{ return checked; } timeout:kPromptSeconds description:@"Matter queue block after the timeout ran"]);
    XCTAssertTrue(deliveredByTimeoutTurn, @"existing session handed over in the timeout's own Matter queue turn");
    XCTAssertTrue([self waitUntil:^{ return sessionDelivered; } timeout:kPromptSeconds description:@"session delivered"]);
#else
    XCTSkip(@"Requires DEBUG test hooks");
#endif
}

@end
