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

#import <Matter/Matter.h>
#import <Network/Network.h>
#import <dns_sd.h>

#import "MTRDeviceConnectivityMonitor.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

@interface MTRDeviceConnectivityMonitor (Test)
- (instancetype)initWithInstanceName:(NSString *)instanceName;
- (NSUInteger)monitorID;
#ifdef DEBUG
+ (BOOL)unitTestHasActiveSharedConnection;
#endif
@end

#ifdef DEBUG
// Pure-ObjC view onto the DEBUG-only test seam implemented in MTRDeviceController_Concrete.mm.
// Declared inline (no C++ types) so this .m test file can drive the production getSessionForNode:
// Thread path without importing the C++ MTRDeviceController_Concrete.h.
@interface MTRDeviceController (ConnectivityMonitorUnitTest)
@property (nonatomic, weak, readonly, nullable) MTRDeviceConnectivityMonitor * unitTestLastConnectivityMonitor;
- (void)unitTestSetConnectivityMonitorWatchdogInterval:(NSTimeInterval)interval;
- (void)unitTestForceThreadGetSessionForNode:(uint64_t)nodeID;
- (void)unitTestForceThreadGetSessionForNode:(uint64_t)nodeID completion:(void (^)(BOOL success))completion;
@end
#endif

@interface MTRDeviceConnectivityMonitorTests : XCTestCase
@end

@implementation MTRDeviceConnectivityMonitorTests

static DNSServiceRef sSharedConnection;

+ (void)setUp
{
    DNSServiceErrorType dnsError = DNSServiceCreateConnection(&sSharedConnection);
    XCTAssertEqual(dnsError, kDNSServiceErr_NoError);
}

+ (void)tearDown
{
    DNSServiceRefDeallocate(sSharedConnection);
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

// === Regression coverage for the daemon-leak fix in MTRDeviceController_Concrete.mm ===
//
// These tests drive the *production* getSessionForNode: Thread path (via the DEBUG-only
// -unitTestForceThreadGetSessionForNode: seam) against a node whose DNS-SD never resolves.  test008
// asserts the MTRDeviceConnectivityMonitor the production code created deallocates after the
// watchdog fires (the core leak regression); test009 asserts that shutting the controller down while
// the watchdog is still pending is handled safely (no crash/hang/double-free).
//
// Against unmodified upstream -- which captured the monitor *strongly* in the DNS-SD handler and
// had NO watchdog -- the handler never fires for an unreachable node, -stopMonitoring is never
// called, and the handler->monitor cycle keeps the monitor alive forever (the reported daemon
// leak).  In that world test008 times out waiting for the weak ref to clear and FAILS.  With the
// fix (weak handler capture + a watchdog that stops the monitor and releases its strong reference)
// the monitor deallocates and test008 PASSES.
//
// Note the throttle is intentionally preserved: the watchdog only stops the monitor, it does NOT
// enqueue the CASE work item, so an unreachable node still generates no CASE traffic and the
// caller completion still never fires on this path (matching pre-fix semantics).  We therefore do
// not assert on the completion; we assert on the monitor's lifetime, which is the thing the fix
// changes.

#ifdef DEBUG
- (MTRDeviceController *)startThrowawayControllerForLeakTest
{
    MTRDeviceControllerFactory * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    MTRTestStorage * storage = [[MTRTestStorage alloc] init];
    MTRDeviceControllerFactoryParams * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    XCTAssertTrue([factory startControllerFactory:factoryParams error:NULL]);

    MTRTestKeys * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    MTRDeviceControllerStartupParams * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    XCTAssertNotNil(params);
    params.vendorID = @(0xFFF1);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:NULL];
    XCTAssertNotNil(controller);
    XCTAssertTrue([controller isRunning]);
    return controller;
}

// Spins the run loop until weakRef is nil or the deadline passes; returns whether it cleared.
- (BOOL)waitForWeakReferenceToClear:(MTRDeviceConnectivityMonitor * __weak *)weakRef timeout:(NSTimeInterval)timeout
{
    NSDate * deadline = [NSDate dateWithTimeIntervalSinceNow:timeout];
    while (*weakRef != nil && [deadline timeIntervalSinceNow] > 0) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.05]];
        }
    }
    return (*weakRef == nil);
}
#endif

// The core regression test: the controller's connectivity monitor must deallocate after the
// watchdog fires for an unreachable node.  Fails against unmodified upstream (strong handler
// capture, no watchdog); passes with the fix.
//
// The leak assertion is made robust against autorelease-pool timing in two ways: (1) the
// getSessionForNode: call is wrapped in its own autorelease pool, and we capture the weak ref to
// the production-created monitor *after* that pool drains, so no autoreleased strong reference from
// the call itself can mask a leak; (2) the watchdog block carries its own @autoreleasepool, so the
// monitor is released synchronously when the watchdog fires rather than at the next drain of the
// long-lived work-queue thread's pool.  After the watchdog interval (0.5s) the ONLY thing that
// could keep the monitor alive is the leaked handler->monitor cycle, so a non-nil weak ref here is
// an unambiguous leak rather than a timing artifact.
- (void)test008_GetSessionForNodeMonitorDeallocatesAfterWatchdog
{
#ifdef DEBUG
    MTRDeviceController * controller = [self startThrowawayControllerForLeakTest];

    // Shorten the watchdog so the production lifetime bound runs fast.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0.5];

    // An operational node id that does not exist on the network: DNS-SD never resolves, so only the
    // watchdog can bound the monitor's lifetime.
    uint64_t unreachableNodeID = 0x1122334455667788ULL;

    __weak MTRDeviceConnectivityMonitor * weakMonitor = nil;
    @autoreleasepool {
        [controller unitTestForceThreadGetSessionForNode:unreachableNodeID];
        weakMonitor = controller.unitTestLastConnectivityMonitor;
    }
    XCTAssertNotNil(weakMonitor,
        @"getSessionForNode: should have created a connectivity monitor for the Thread path");

    // With the fix, the 0.5s watchdog stops the monitor and drops the last strong reference inside
    // its own autorelease pool, so the monitor deallocates well within this budget.  Against
    // unmodified upstream nothing ever releases it for an unreachable node, so this FAILS.
    BOOL cleared = [self waitForWeakReferenceToClear:&weakMonitor timeout:6.0];
    XCTAssertTrue(cleared,
        @"The connectivity monitor created by getSessionForNode: must deallocate after the "
        @"watchdog fires.  If it stays alive, the handler is retaining it (or the watchdog is "
        @"missing) and the daemon leak has regressed.");

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
    // Reset the process-global watchdog override so it cannot leak into any future test in the suite.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
#else
    XCTSkip(@"Test requires DEBUG build for the getSessionForNode unit-test seam");
#endif
}

// Companion to test008, covering the shutdown/watchdog race for *safety* (not leak): shutting the
// controller down while a watchdog is still pending must not crash, hang, or double-free.  Both the
// shutdown path and the watchdog later call -stopMonitoring on the same monitor, and the watchdog
// fires against an already-torn-down controller; -stopMonitoring is idempotent and the watchdog
// captures everything weakly, so this must complete cleanly.  (The unbounded-leak regression itself
// is pinned by test008; we do not re-assert deallocation here because post-shutdown teardown
// involves asynchronous DNS-SD cleanup whose exact timing would make a dealloc assertion flaky.)
- (void)test009_GetSessionForNodeShutdownRacingWatchdogIsSafe
{
#ifdef DEBUG
    MTRDeviceController * controller = [self startThrowawayControllerForLeakTest];

    // Short watchdog so it is guaranteed to fire (against the torn-down controller) within the test.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0.5];

    uint64_t unreachableNodeID = 0x99AABBCCDDEEFF00ULL;

    __weak MTRDeviceConnectivityMonitor * weakMonitor = nil;
    @autoreleasepool {
        [controller unitTestForceThreadGetSessionForNode:unreachableNodeID];
        weakMonitor = controller.unitTestLastConnectivityMonitor;
    }
    XCTAssertNotNil(weakMonitor, @"getSessionForNode: should have created a connectivity monitor");

    // Tear the controller down immediately, while the 0.5s watchdog is still pending.  This must not
    // crash or hang via a double -stopMonitoring or a watchdog firing against a torn-down controller.
    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
    // Reset the process-global watchdog override so it cannot leak into any future test in the suite.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0];
    controller = nil;
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];

    // Let the pending watchdog fire (and safely no-op) well past its interval.  Reaching here without
    // a crash/hang is the assertion: the shutdown vs. watchdog race is handled safely.
    XCTestExpectation * settle = [self expectationWithDescription:@"settle past watchdog"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [settle fulfill];
    });
    [self waitForExpectations:@[ settle ] timeout:5.0];
#else
    XCTSkip(@"Test requires DEBUG build for the getSessionForNode unit-test seam");
#endif
}

// Pins the documented post-watchdog semantic delta: once the watchdog has fired and torn the monitor
// down, the Thread getSessionForNode: path has nothing left to resolve the node, so the caller
// completion must NOT have fired for an unreachable node.  This is the deliberate throttle being
// preserved (a node that never resolves generates no CASE traffic and no completion), and it makes
// the otherwise-implicit "after 60s this path stops driving recovery" behavior explicit and tested.
// (Recovery for a node that returns late is driven independently by the device-level
// MTRDevice _connectivityMonitor and the per-device resubscription timer, not by this path.)
- (void)test010_GetSessionForNodeCompletionDoesNotFireForUnreachableNodePostWatchdog
{
#ifdef DEBUG
    MTRDeviceController * controller = [self startThrowawayControllerForLeakTest];

    // Short watchdog so the monitor is torn down quickly.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0.5];

    uint64_t unreachableNodeID = 0x0102030405060708ULL;

    __block BOOL completionFired = NO;
    __weak MTRDeviceConnectivityMonitor * weakMonitor = nil;
    @autoreleasepool {
        [controller unitTestForceThreadGetSessionForNode:unreachableNodeID
                                              completion:^(BOOL success) {
                                                  completionFired = YES;
                                              }];
        weakMonitor = controller.unitTestLastConnectivityMonitor;
    }
    XCTAssertNotNil(weakMonitor, @"getSessionForNode: should have created a connectivity monitor");

    // Wait until the watchdog has torn the monitor down (it deallocates), then a bit more to give any
    // (incorrectly) enqueued work item a chance to drive the completion.  Neither should happen.
    BOOL cleared = [self waitForWeakReferenceToClear:&weakMonitor timeout:6.0];
    XCTAssertTrue(cleared, @"The connectivity monitor must deallocate after the watchdog fires");

    XCTestExpectation * settle = [self expectationWithDescription:@"settle past watchdog"];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [settle fulfill];
    });
    [self waitForExpectations:@[ settle ] timeout:5.0];

    XCTAssertFalse(completionFired,
        @"For an unreachable node the Thread getSessionForNode: completion must never fire: the work "
        @"item is enqueued only on DNS-SD resolve, and once the watchdog tears the monitor down "
        @"nothing can resolve it.  If this fires, the throttle/post-watchdog contract has regressed.");

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);
    // Reset the process-global watchdog override so it cannot leak into any future test in the suite.
    [controller unitTestSetConnectivityMonitorWatchdogInterval:0];
    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
#else
    XCTSkip(@"Test requires DEBUG build for the getSessionForNode unit-test seam");
#endif
}

@end
