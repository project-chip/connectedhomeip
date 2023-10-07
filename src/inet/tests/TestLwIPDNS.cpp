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
 *      This file implements a process to effect a functional test for
 *      LwIP's Domain Name Service (DNS) interface.
 *
 */

#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>

#include <stdint.h>
#include <string.h>

#include <sys/time.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <CHIPVersion.h>

#include <inet/InetArgParser.h>

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"
#include "TestSetupFaultInjection.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;

#define TOOL_NAME "TestLwIPDNS"

static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[]);

// Globals

#if CHIP_SYSTEM_CONFIG_USE_LWIP
static uint8_t sNumIpAddrs = DNS_MAX_ADDRS_PER_NAME;
static ip_addr_t sIpAddrs[DNS_MAX_ADDRS_PER_NAME];
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

static const char * sHostname      = nullptr;
static const char * sDNSServerAddr = nullptr;

// clang-format off
static ArgParser::HelpOptions gHelpOptions(TOOL_NAME,
                                           "Usage: " TOOL_NAME " [<options...>] <hostname> <dns-server-address>\n",
                                           CHIP_VERSION_STRING "\n" CHIP_TOOL_COPYRIGHT);

static ArgParser::OptionSet * gToolOptionSets[] =
{
    &gNetworkOptions,
    &gFaultInjectionOptions,
    &gHelpOptions,
    nullptr
};
// clang-format on

#if CHIP_SYSTEM_CONFIG_USE_LWIP
static void found_multi(const char * aName, ip_addr_t * aIpAddrs, uint8_t aNumIpAddrs, void * callback_arg)
{
    printf("\tfound_multi response\n");
    printf("\tName: %s\n", aName);
    printf("\tnumipaddrs: %d (DNS_MAX_ADDRS_PER_NAME: %d)\n", aNumIpAddrs, DNS_MAX_ADDRS_PER_NAME);

    for (uint8_t i = 0; i < aNumIpAddrs; ++i)
    {
        char addrStr[INET6_ADDRSTRLEN];

        IPAddress(aIpAddrs[i]).ToString(addrStr, sizeof(addrStr));

        printf("\t(%d) IPv4: %s\n", i, addrStr);
    }

    Done = true;
}

static void TestLwIPDNS(void)
{
    uint8_t numdns = 1;
    ip_addr_t dnsserver[1];
    IPAddress DNSServerIPv4Addr;

    IPAddress::FromString(sDNSServerAddr, DNSServerIPv4Addr);

    dnsserver[0] = DNSServerIPv4Addr.ToIPv4();

    dns_setserver(numdns, dnsserver);

    printf("\nStarted dns_gethostbyname_multi test...\n\n");

    // Expected request / response
    printf("Expected request / response #1\n");
    printf("hn: %s, ips: %p, nips: %d, fm: %p, arg: %p\n", hostname, sIpAddrs, sNumIpAddrs, found_multi, NULL);
    printf("ip[0]: %d, ip[1]: %d\n", sIpAddrs[0], sIpAddrs[1]);
    err_t res = dns_gethostbyname_multi(hostname, sIpAddrs, &sNumIpAddrs, found_multi, NULL);
    if (res == ERR_INPROGRESS)
    {
        printf("\tdns_gethostbyname_multi: %d (ERR_INPROGRESS)\n", res);
    }
    else
    {
        printf("\tdns_gethostbyname_multi: %d (expected -5: ERR_INPROGRESS)\n", res);
    }

    while (!Done)
    {
        constexpr uint32_t kSleepTimeMilliseconds = 10;
        ServiceNetwork(kSleepTimeMilliseconds);
    }

    // Expected cached response
    printf("Expected cached response #1\n");
    sNumIpAddrs = DNS_MAX_ADDRS_PER_NAME;
    printf("hn: %s, ips: %p, nips: %d, fm: %p, arg: %p\n", hostname, ipaddrs, sNumIpAddrs, found_multi, NULL);
    printf("ip[0]: %d, ip[1]: %d\n", sIpAddrs[0], sIpAddrs[1]);
    res = dns_gethostbyname_multi(hostname, ipaddrs, &sNumIpAddrs, found_multi, NULL);
    if (res == ERR_OK)
    {
        printf("\tdns_gethostbyname_multi: %d (ERR_OK)\n", res);
        printf("\tlocal DNS cache response\n");
        printf("\tName: %s\n", hostname);
        printf("\tnumipaddrs: %d\n", sNumIpAddrs);
        for (uint8_t i = 0; i < sNumIpAddrs; ++i)
        {
            char addrStr[64];
            IPAddress(sIpAddrs[i]).ToString(addrStr, sizeof(addrStr));
            printf("\t(%d) IPv4: %s\n", i, addrStr);
        }
    }
    else
    {
        printf("\tdns_gethostbyname_multi: %d (expected : ERR_OK)\n", res);
    }

    // Expected cached response
    printf("Expected cached response #2\n");
    sNumIpAddrs = DNS_MAX_ADDRS_PER_NAME - 1;
    printf("hn: %s, ips: %p, nips: %d, fm: %p, arg: %p\n", hostname, ipaddrs, sNumIpAddrs, found_multi, NULL);
    printf("ip[0]: %d, ip[1]: %d\n", sIpAddrs[0], sIpAddrs[1]);

    res = dns_gethostbyname_multi(hostname, sIpAddrs, &sNumIpAddrs, found_multi, NULL);

    if (res == ERR_OK)
    {
        printf("\tdns_gethostbyname_multi: %d (ERR_OK)\n", res);
        printf("\tlocal DNS cache response\n");
        printf("\tName: %s\n", hostname);
        printf("\tnumipaddrs: %d\n", sNumIpAddrs);
        for (i = 0; i < sNumIpAddrs; ++i)
        {
            char addrStr[64];
            IPAddress(sIpAddrs[i]).ToString(addrStr, sizeof(addrStr));
            printf("\t(%d) IPv4: %s\n", i, addrStr);
        }
    }
    else
    {
        printf("\tdns_gethostbyname_multi: %d (expected : ERR_OK)\n", res);
    }

    // Expected cached response
    printf("Expected cached response #3\n");
    sNumIpAddrs = 0;
    printf("hn: %s, ips: %p, nips: %d, fm: %p, arg: %p\n", hostname, ipaddrs, sNumIpAddrs, found_multi, NULL);
    printf("ip[0]: %d, ip[1]: %d\n", sIpAddrs[0], sIpAddrs[1]);

    res = dns_gethostbyname_multi(hostname, ipaddrs, &sNumIpAddrs, found_multi, NULL);

    if (res == ERR_OK)
    {
        printf("\tdns_gethostbyname_multi: %d (ERR_OK)\n", res);
        printf("\tlocal DNS cache response\n");
        printf("\tName: %s\n", hostname);
        printf("\tnumipaddrs: %d\n", sNumIpAddrs);
        for (i = 0; i < sNumIpAddrs; ++i)
        {
            char addrStr[64];
            IPAddress(sIpAddrs[i]).ToString(addrStr, sizeof(addrStr));
            printf("\t(%d) IPv4: %s\n", i, addrStr);
        }
    }
    else
    {
        printf("\tdns_gethostbyname_multi: %d (expected : ERR_OK)\n", res);
    }
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

int main(int argc, char * argv[])
{
    SetSIGUSR1Handler();

    if (argc == 1)
    {
        gHelpOptions.PrintBriefUsage(stderr);
        exit(EXIT_FAILURE);
    }

    if (!ParseArgs(TOOL_NAME, argc, argv, gToolOptionSets, HandleNonOptionArgs))
    {
        exit(EXIT_FAILURE);
    }

    InitSystemLayer();

    InitNetwork();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    TestLwIPDNS();
#else
    fprintf(stderr, "Please assert CHIP_SYSTEM_CONFIG_USE_LWIP to use this test.\n");
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    ShutdownNetwork();

    ShutdownSystemLayer();

    return (EXIT_SUCCESS);
}

static bool HandleNonOptionArgs(const char * progName, int argc, char * const argv[])
{
    if (argc < 2)
    {
        printf("TestDNS: Missing %s argument\n", argc == 0 ? "<hostname>" : "<dns-server-address>");
        return false;
    }

    if (argc > 2)
    {
        printf("Unexpected argument: %s\n", argv[1]);
    }

    sHostname      = argv[0];
    sDNSServerAddr = argv[1];

    return true;
}
