/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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

/**
 *    @file
 *      This file tests DNS resolution using the CHIP Inet Layer APIs.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <nlunit-test.h>

#include <CHIPVersion.h>

#include <inet/InetLayer.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

#include <system/SystemClock.h>
#include <system/SystemTimer.h>

#include "TestInetCommon.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;

#if INET_CONFIG_ENABLE_DNS_RESOLVER

#define TOOL_NAME "TestInetLayerDNS"

#define DISABLE_BROKEN_DNS_TESTS 1 // https://github.com/project-chip/connectedhomeip/issues/4670

#define DEFAULT_TEST_DURATION_MILLISECS (20000)
#define DEFAULT_CANCEL_TEST_DURATION_MILLISECS (2000)

static uint32_t sNumResInProgress = 0;
constexpr uint8_t kMaxResults     = 20;

struct DNSResolutionTestCase
{
    const char * hostName;
    uint8_t dnsOptions;
    uint8_t maxResults;
    CHIP_ERROR expectErr;
    bool expectIPv4Addrs;
    bool expectIPv6Addrs;
};

struct DNSResolutionTestContext
{
    nlTestSuite * testSuite;
    DNSResolutionTestCase testCase;
    bool callbackCalled;
    IPAddress resultsBuf[kMaxResults];
};

static void RunTestCase(nlTestSuite * testSuite, const DNSResolutionTestCase & testCase);
static void StartTestCase(DNSResolutionTestContext & testContext);
static void HandleResolutionComplete(void * appState, CHIP_ERROR err, uint8_t addrCount, IPAddress * addrArray);
static void ServiceNetworkUntilDone(uint32_t timeoutMS);
static void HandleSIGUSR1(int sig);

/**
 * Test basic name resolution functionality.
 */
static void TestDNSResolution_Basic(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off

#ifndef DISABLE_BROKEN_DNS_TESTS
    // Test resolving a name with only IPv4 addresses.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "ipv4.google.com",
            kDNSOption_Default,
            kMaxResults,
            CHIP_NO_ERROR,
            true,
            false
        }
    );

    // Test resolving a name with only IPv6 addresses.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "ipv6.google.com",
            kDNSOption_Default,
            kMaxResults,
            CHIP_NO_ERROR,
            false,
            true
        }
    );

    // Test resolving a name with IPv4 and IPv6 addresses.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_Default,
            kMaxResults,
            CHIP_NO_ERROR,
            true,
            true
        }
    );
#endif
    // clang-format on
}

/**
 * Test resolving a name using various address type options.
 */
static void TestDNSResolution_AddressTypeOption(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off
#ifndef DISABLE_BROKEN_DNS_TESTS

    // Test requesting IPv4 addresses only.
#if INET_CONFIG_ENABLE_IPV4
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv4Only,
            kMaxResults,
            CHIP_NO_ERROR,
            true,
            false
        }
    );
#endif // INET_CONFIG_ENABLE_IPV4

    // Test requesting IPv6 addresses only.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv6Only,
            kMaxResults,
            CHIP_NO_ERROR,
            false,
            true
        }
    );

    // Test requesting IPv4 address preferentially.
#if INET_CONFIG_ENABLE_IPV4
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv4Preferred,
            kMaxResults,
            CHIP_NO_ERROR,
            true,
            true
        }
    );
#endif // INET_CONFIG_ENABLE_IPV4

    // Test requesting IPv6 address preferentially.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv6Preferred,
            kMaxResults,
            CHIP_NO_ERROR,
            true,
            true
        }
    );

#endif
    // clang-format on
}

/**
 * Test resolving a name with a limited number of results.
 */
static void TestDNSResolution_RestrictedResults(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off

    // Test requesting 2 IPv4 addresses.  This should result in, at most, 2 IPv4 addresses.
#if INET_CONFIG_ENABLE_IPV4
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv4Only,
            2,
            CHIP_NO_ERROR,
            true,
            false
        }
    );
#endif // INET_CONFIG_ENABLE_IPV4

    // Test requesting 2 IPv6 addresses.  This should result in, at most, 2 IPv6 addresses.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv6Only,
            2,
            CHIP_NO_ERROR,
            false,
            true
        }
    );

    // Test requesting 2 addresses, preferring IPv4.  This should result in 1 IPv4 address
    // followed by 1 IPv6 address.
#if INET_CONFIG_ENABLE_IPV4
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv4Preferred,
            2,
            CHIP_NO_ERROR,
            true,
            true
        }
    );
#endif // INET_CONFIG_ENABLE_IPV4

    // Test requesting 2 addresses, preferring IPv6.  This should result in 1 IPv6 address
    // followed by 1 IPv4 address.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "google.com",
            kDNSOption_AddrFamily_IPv6Preferred,
            2,
            CHIP_NO_ERROR,
            true,
            true
        }
    );
    // clang-format on
}

/**
 * Test resolving a non-existant name.
 */
static void TestDNSResolution_NoRecord(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off
#ifndef DISABLE_BROKEN_DNS_TESTS
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "www.google.invalid.",
            kDNSOption_AddrFamily_Any,
            1,
            INET_ERROR_HOST_NOT_FOUND,
            false,
            false
        }
    );
#endif
    // clang-format on
}

/**
 * Test resolving a name where the resultant DNS entry lacks an A or AAAA record.
 */
static void TestDNSResolution_NoHostRecord(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off
#ifndef DISABLE_BROKEN_DNS_TESTS

    // Test resolving a name that has no host records (A or AAAA).
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "_spf.google.com",
            kDNSOption_AddrFamily_Any,
            kMaxResults,
            INET_ERROR_HOST_NOT_FOUND,
            false,
            false
        }
    );

    // Test resolving a name that has only AAAA records, while requesting IPv4 addresses only.
#if INET_CONFIG_ENABLE_IPV4
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "ipv6.google.com",
            kDNSOption_AddrFamily_IPv4Only,
            kMaxResults,
            INET_ERROR_HOST_NOT_FOUND,
            true,
            false
        }
    );
#endif // INET_CONFIG_ENABLE_IPV4

    // Test resolving a name that has only A records, while requesting IPv6 addresses only.
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "ipv4.google.com",
            kDNSOption_AddrFamily_IPv6Only,
            kMaxResults,
            INET_ERROR_HOST_NOT_FOUND,
            false,
            false
        }
    );
    #endif

    // clang-format on
}

/**
 * Test resolving text form IP addresses.
 */
static void TestDNSResolution_TextForm(nlTestSuite * testSuite, void * testContext)
{
    // clang-format off
    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "216.58.194.174",
            kDNSOption_AddrFamily_Any,
            1,
            CHIP_NO_ERROR,
            true,
            false
        }
    );

    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "2607:f8b0:4005:804::200e",
            kDNSOption_AddrFamily_Any,
            1,
            CHIP_NO_ERROR,
            false,
            true
        }
    );

    // Test resolving text form IPv4 and IPv6 addresses while requesting an
    // incompatible address type.

    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "216.58.194.174",
            kDNSOption_AddrFamily_IPv6Only,
            1,
            INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE,
            false,
            false
        }
    );

    RunTestCase(testSuite,
        DNSResolutionTestCase
        {
            "2607:f8b0:4005:804::200e",
            kDNSOption_AddrFamily_IPv4Only,
            1,
            INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE,
            false,
            false
        }
    );
    // clang-format on
}

static void TestDNSResolution_Cancel(nlTestSuite * testSuite, void * inContext)
{
    DNSResolutionTestContext testContext{
        testSuite, DNSResolutionTestCase{ "www.google.com", kDNSOption_Default, kMaxResults, CHIP_NO_ERROR, true, false }
    };

    // Start DNS resolution.
    StartTestCase(testContext);

    // If address resolution did NOT complete synchronously...
    // (NOTE: If address resolution completes synchronously then this test is effectively
    // void, as there's no opportunity to cancel the request).
    if (!testContext.callbackCalled)
    {
        // Cancel the resolution before it completes.
        gInet.CancelResolveHostAddress(HandleResolutionComplete, &testContext);

        // Service the network for awhile to see what happens (should timeout).
        ServiceNetworkUntilDone(DEFAULT_CANCEL_TEST_DURATION_MILLISECS);

        // Verify that the completion function was NOT called.
        NL_TEST_ASSERT(testSuite, testContext.callbackCalled == false);
    }

    gDone             = true;
    sNumResInProgress = 0;
}

static void TestDNSResolution_Simultaneous(nlTestSuite * testSuite, void * inContext)
{
    // clang-format off
    DNSResolutionTestContext tests[] =
    {
        {
            testSuite,
            DNSResolutionTestCase
            {
                "www.nest.com",
                kDNSOption_Default,
                kMaxResults,
                CHIP_NO_ERROR,
                true,
                false
            }
        },
        {
            testSuite,
            DNSResolutionTestCase
            {
                "10.0.0.1",
                kDNSOption_Default,
                kMaxResults,
                CHIP_NO_ERROR,
                true,
                false
            }
        },
        {
            testSuite,
            DNSResolutionTestCase
            {
                "www.google.com",
                kDNSOption_Default,
                kMaxResults,
                CHIP_NO_ERROR,
                true,
                true
            }
        },
        {
            testSuite,
            DNSResolutionTestCase
            {
                "pool.ntp.org",
                kDNSOption_Default,
                kMaxResults,
                CHIP_NO_ERROR,
                true,
                false
            }
        }
    };
    // clang-format on

    // Start multiple DNS resolutions simultaneously.
    for (DNSResolutionTestContext & testContext : tests)
    {
        StartTestCase(testContext);
    }

    // Service the network until each completes, or a timeout occurs.
    ServiceNetworkUntilDone(DEFAULT_TEST_DURATION_MILLISECS);

    // Verify no timeout occurred.
    NL_TEST_ASSERT(testSuite, gDone == true);

    // Sanity check test logic.
    NL_TEST_ASSERT(testSuite, sNumResInProgress == 0);
}

static void RunTestCase(nlTestSuite * testSuite, const DNSResolutionTestCase & testCase)
{
    DNSResolutionTestContext testContext{ testSuite, testCase };

    // Start DNS resolution.
    StartTestCase(testContext);

    // Service the network until the completion callback is called.
    ServiceNetworkUntilDone(DEFAULT_TEST_DURATION_MILLISECS);

    // Verify no timeout occurred.
    NL_TEST_ASSERT(testSuite, gDone == true);

    // Sanity check test logic.
    NL_TEST_ASSERT(testSuite, sNumResInProgress == 0);
}

static void StartTestCase(DNSResolutionTestContext & testContext)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    DNSResolutionTestCase & testCase = testContext.testCase;
    nlTestSuite * testSuite          = testContext.testSuite;

    gDone = false;
    sNumResInProgress++;

    printf("Resolving hostname %s\n", testCase.hostName);
    err = gInet.ResolveHostAddress(testCase.hostName, strlen(testCase.hostName), testCase.dnsOptions, testCase.maxResults,
                                   testContext.resultsBuf, HandleResolutionComplete, &testContext);

    if (err != CHIP_NO_ERROR)
    {
        printf("ResolveHostAddress failed for %s: %s\n", testCase.hostName, ::chip::ErrorStr(err));

        // Verify the expected error
        NL_TEST_ASSERT(testSuite, err == testCase.expectErr);

        // Verify the callback WASN'T called
        NL_TEST_ASSERT(testSuite, testContext.callbackCalled == false); //

        sNumResInProgress--;
        if (sNumResInProgress == 0)
        {
            gDone = true;
        }
    }
}

static void HandleResolutionComplete(void * appState, CHIP_ERROR err, uint8_t addrCount, IPAddress * addrArray)
{
    DNSResolutionTestContext & testContext = *static_cast<DNSResolutionTestContext *>(appState);
    DNSResolutionTestCase & testCase       = testContext.testCase;
    nlTestSuite * testSuite                = testContext.testSuite;

    if (err == CHIP_NO_ERROR)
    {
        printf("DNS resolution complete for %s: %" PRIu8 " result%s returned\n", testCase.hostName, addrCount,
               (addrCount != 1) ? "s" : "");
        for (uint8_t i = 0; i < addrCount; i++)
        {
            char ipAddrStr[INET6_ADDRSTRLEN];

            printf("    %s\n", addrArray[i].ToString(ipAddrStr, sizeof(ipAddrStr)));
        }
    }
    else
    {
        printf("DNS resolution complete for %s: %s\n", testCase.hostName, ::chip::ErrorStr(err));
    }

    // Verify the expected result.
    NL_TEST_ASSERT(testSuite, err == testCase.expectErr);

    if (err == CHIP_NO_ERROR)
    {
        // Make sure the number of addresses is within the max expected.
        NL_TEST_ASSERT(testSuite, addrCount <= testCase.maxResults);

        // Determine the types of addresses in the response and their relative ordering.
        bool respContainsIPv4Addrs = false;
        bool respContainsIPv6Addrs = false;

        for (uint8_t i = 0; i < addrCount; i++)
        {
            respContainsIPv4Addrs = respContainsIPv4Addrs || (addrArray[i].Type() == kIPAddressType_IPv4);
            respContainsIPv6Addrs = respContainsIPv6Addrs || (addrArray[i].Type() == kIPAddressType_IPv6);
        }

        // Verify the expected address types were returned.
        // The current LwIP DNS implementation returns at most one address.  So if the test expects
        // both IPv4 and IPv6 addresses, relax this to accept either.
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        if (testCase.expectIPv4Addrs && testCase.expectIPv6Addrs)
        {
            NL_TEST_ASSERT(testSuite, respContainsIPv4Addrs || respContainsIPv6Addrs);
        }
        else
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
        {
            if (testCase.expectIPv4Addrs)
            {
                NL_TEST_ASSERT(testSuite, respContainsIPv4Addrs);
            }
            if (testCase.expectIPv6Addrs)
            {
                NL_TEST_ASSERT(testSuite, respContainsIPv6Addrs);
            }
        }

        // Verify that only the requested address types were returned, and that the
        // addresses were returned in the correct order.
        switch (testCase.dnsOptions & kDNSOption_AddrFamily_Mask)
        {
        case kDNSOption_AddrFamily_Any:
            break;
        case kDNSOption_AddrFamily_IPv4Only:
            NL_TEST_ASSERT(testSuite, !respContainsIPv6Addrs);
            break;
        case kDNSOption_AddrFamily_IPv4Preferred:
            if (respContainsIPv4Addrs)
            {
                NL_TEST_ASSERT(testSuite, addrArray[0].Type() == kIPAddressType_IPv4);
            }
            break;
        case kDNSOption_AddrFamily_IPv6Only:
            NL_TEST_ASSERT(testSuite, !respContainsIPv4Addrs);
            break;
        case kDNSOption_AddrFamily_IPv6Preferred:
            if (respContainsIPv6Addrs)
            {
                NL_TEST_ASSERT(testSuite, addrArray[0].Type() == kIPAddressType_IPv6);
            }
            break;
        default:
            constexpr bool UnexpectedAddressTypeValue = true;
            NL_TEST_ASSERT(testSuite, !UnexpectedAddressTypeValue);
        }
    }

    testContext.callbackCalled = true;

    sNumResInProgress--;
    if (sNumResInProgress == 0)
    {
        gDone = true;
    }
}

static void ServiceNetworkUntilDone(uint32_t timeoutMS)
{
    uint64_t timeoutTimeMS = System::Clock::GetMonotonicMilliseconds() + timeoutMS;
    struct timeval sleepTime;
    sleepTime.tv_sec  = 0;
    sleepTime.tv_usec = 10000;

    while (!gDone)
    {
        ServiceNetwork(sleepTime);

        if (System::Clock::GetMonotonicMilliseconds() >= timeoutTimeMS)
        {
            break;
        }
    }
}

static void HandleSIGUSR1(int sig)
{
    gInet.Shutdown();

    exit(0);
}

int TestInetLayerDNSInternal()
{
    // clang-format off
    const nlTest DNSTests[] =
    {
        NL_TEST_DEF("TestDNSResolution:Basic",             TestDNSResolution_Basic),
        NL_TEST_DEF("TestDNSResolution:AddressTypeOption", TestDNSResolution_AddressTypeOption),
        NL_TEST_DEF("TestDNSResolution:RestrictedResults", TestDNSResolution_RestrictedResults),
        NL_TEST_DEF("TestDNSResolution:TextForm",          TestDNSResolution_TextForm),
        NL_TEST_DEF("TestDNSResolution:NoRecord",          TestDNSResolution_NoRecord),
        NL_TEST_DEF("TestDNSResolution:NoHostRecord",      TestDNSResolution_NoHostRecord),
        NL_TEST_DEF("TestDNSResolution:Cancel",            TestDNSResolution_Cancel),
        NL_TEST_DEF("TestDNSResolution:Simultaneous",      TestDNSResolution_Simultaneous),
        NL_TEST_SENTINEL() };

    nlTestSuite DNSTestSuite =
    {
        "DNS",
        &DNSTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitTestInetCommon();

    InitSystemLayer();
    InitNetwork();

    // Run all tests in Suite

    nlTestRunner(&DNSTestSuite, nullptr);

    ShutdownNetwork();
    ShutdownSystemLayer();

    return nlTestRunnerStats(&DNSTestSuite);
}

CHIP_REGISTER_TEST_SUITE(TestInetLayerDNSInternal)
#else // !INET_CONFIG_ENABLE_DNS_RESOLVER

int TestInetLayerDNSInternal(void)
{
    fprintf(stderr, "Please assert INET_CONFIG_ENABLE_DNS_RESOLVER to use this test.\n");

    return (EXIT_SUCCESS);
}

#endif // !INET_CONFIG_ENABLE_DNS_RESOLVER

int TestInetLayerDNS()
{
    SetSignalHandler(HandleSIGUSR1);

    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestInetLayerDNSInternal());
}
