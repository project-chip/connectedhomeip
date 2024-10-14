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

static void test001_MonitorTest_RegisterCallback(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char * name,
    const char * regtype,
    const char * domain,
    void * context)
{
}

- (void)test001_BasicMonitorTest
{
    dispatch_queue_t testQueue = dispatch_queue_create("connectivity-monitor-test-queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    DNSServiceRef testAdvertiser;
    DNSServiceFlags flags = kDNSServiceFlagsNoAutoRename;
    char testInstanceName[] = "testinstance-name";
    char testHostName[] = "localhost";
    uint16_t testPort = htons(15000);
    DNSServiceErrorType dnsError = DNSServiceRegister(&testAdvertiser, flags, 0, testInstanceName, kOperationalType, kLocalDot, testHostName, testPort, 0, NULL, test001_MonitorTest_RegisterCallback, NULL);
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

@end
