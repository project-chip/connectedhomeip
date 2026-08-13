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

#import "MTRDeviceConnectivityMonitor.h"

@interface MTRDeviceConnectivityMonitor (Test)
- (instancetype)initWithInstanceName:(NSString *)instanceName;
- (NSUInteger)monitorID;
#ifdef DEBUG
+ (BOOL)unitTestHasActiveSharedConnection;
#endif
@end

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

@end
