/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file implements constants, globals and interfaces common to
 *      and used by all CHP Inet layer library test applications and
 *      tools.
 *
 *      NOTE: These do not comprise a public part of the CHIP API and
 *            are subject to change without notice.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"

#include <errno.h>
#include <vector>

#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/ScopedBuffer.h>
#include <system/SystemClock.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/dns.h>
#if !(LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
#include <lwip/ip6_route_table.h>
#endif // !(LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <netif/etharp.h>

#if CHIP_TARGET_STYLE_UNIX

// TapAddrAutoconf and TapInterface are only needed for LwIP on
// sockets simulation in which a host tap/tun interface is used to
// proxy the LwIP stack onto a host native network interface.

#include "TapAddrAutoconf.h"
#include "TapInterface.h"
#endif // CHIP_TARGET_STYLE_UNIX
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

using namespace chip;
using namespace chip::Inet;

System::LayerImpl gSystemLayer;

Inet::InetLayer gInet;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
static sys_mbox_t * sLwIPEventQueue   = NULL;
static unsigned int sLwIPAcquireCount = 0;

static void AcquireLwIP(void)
{
    if (sLwIPAcquireCount++ == 0)
    {
        sys_mbox_new(sLwIPEventQueue, 100);
    }
}

static void ReleaseLwIP(void)
{
#if !(LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
    if (sLwIPAcquireCount > 0 && --sLwIPAcquireCount == 0)
    {
#if defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
        // FreeRTOS need to delete the task not return from it.
        tcpip_finish(reinterpret_cast<tcpip_will_finish_fn>(vTaskDelete), NULL);
#else  // defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
        tcpip_finish(NULL, NULL);
#endif // defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
    }
#endif
}

#if CHIP_TARGET_STYLE_UNIX
// TapAddrAutoconf and TapInterface are only needed for LwIP on
// sockets simulation in which a host tap/tun interface is used to
// proxy the LwIP stack onto a host native network interface.
// CollectTapAddresses() is only available on such targets.
static std::vector<TapInterface> sTapIFs;
#endif // CHIP_TARGET_STYLE_UNIX

static std::vector<struct netif> sNetIFs; // interface to filter

static bool NetworkIsReady();
static void OnLwIPInitComplete(void * arg);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

char gDefaultTapDeviceName[32];
bool gDone = false;

void InetFailError(CHIP_ERROR err, const char * msg)
{
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "%s: %s\n", msg, ErrorStr(err));
        exit(-1);
    }
}

static void UseStdoutLineBuffering()
{
    // Set stdout to be line buffered with a buffer of 512 (will flush on new line
    // or when the buffer of 512 is exceeded).
#if CHIP_CONFIG_MEMORY_MGMT_MALLOC
    constexpr char * buf = nullptr;
#else
    static char buf[512];
#endif // CHIP_CONFIG_MEMORY_MGMT_MALLOC
    setvbuf(stdout, buf, _IOLBF, 512);
}

void InitTestInetCommon()
{
    chip::Platform::MemoryInit();
    UseStdoutLineBuffering();
}

void InitSystemLayer()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    AcquireLwIP();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP

    gSystemLayer.Init();
}

void ShutdownSystemLayer()
{
    gSystemLayer.Shutdown();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    ReleaseLwIP();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
static void PrintNetworkState()
{
    char intfName[chip::Inet::InterfaceIterator::kMaxIfNameLength];

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        struct netif * netIF = &(sNetIFs[j]);
#if CHIP_TARGET_STYLE_UNIX
        // TapAddrAutoconf and TapInterface are only needed for LwIP on
        // sockets simulation in which a host tap/tun interface is used to
        // proxy the LwIP stack onto a host native network interface.
        // CollectTapAddresses() is only available on such targets.

        TapInterface * tapIF = &(sTapIFs[j]);
#endif // CHIP_TARGET_STYLE_UNIX
        GetInterfaceName(netIF, intfName, sizeof(intfName));

        printf("LwIP interface ready\n");
        printf("  Interface Name: %s\n", intfName);
        printf("  Tap Device: %s\n", gNetworkOptions.TapDeviceName[j]);
#if CHIP_TARGET_STYLE_UNIX
        // TapAddrAutoconf and TapInterface are only needed for LwIP on
        // sockets simulation in which a host tap/tun interface is used to
        // proxy the LwIP stack onto a host native network interface.
        // CollectTapAddresses() is only available on such targets.

        printf("  MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", tapIF->macAddr[0], tapIF->macAddr[1], tapIF->macAddr[2],
               tapIF->macAddr[3], tapIF->macAddr[4], tapIF->macAddr[5]);
#endif // CHIP_TARGET_STYLE_UNIX

#if INET_CONFIG_ENABLE_IPV4
        printf("  IPv4 Address: %s\n", ipaddr_ntoa(&(netIF->ip_addr)));
        printf("  IPv4 Mask: %s\n", ipaddr_ntoa(&(netIF->netmask)));
        printf("  IPv4 Gateway: %s\n", ipaddr_ntoa(&(netIF->gw)));
#endif // INET_CONFIG_ENABLE_IPV4
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            if (!ip6_addr_isany(netif_ip6_addr(netIF, i)))
            {
                printf("  IPv6 address: %s, 0x%02x\n", ip6addr_ntoa(netif_ip6_addr(netIF, i)), netif_ip6_addr_state(netIF, i));
            }
        }
    }
#if INET_CONFIG_ENABLE_DNS_RESOLVER
    char dnsServerAddrStr[DNS_MAX_NAME_LENGTH];
    printf("  DNS Server: %s\n", gNetworkOptions.DNSServerAddr.ToString(dnsServerAddrStr, sizeof(dnsServerAddrStr)));
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

void InitNetwork()
{
    void * lContext = nullptr;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // If an tap device name hasn't been specified, derive one from the IPv6 interface id.

    if (gNetworkOptions.TapDeviceName.empty())
    {
        for (size_t j = 0; j < gNetworkOptions.LocalIPv6Addr.size(); j++)
        {
            uint64_t iid    = gNetworkOptions.LocalIPv6Addr[j].InterfaceId();
            char * tap_name = (char *) chip::Platform::MemoryAlloc(sizeof(gDefaultTapDeviceName));
            assert(tap_name);
            snprintf(tap_name, sizeof(gDefaultTapDeviceName), "chip-dev-%" PRIx16, static_cast<uint16_t>(iid));
            gNetworkOptions.TapDeviceName.push_back(tap_name);
        }
    }

#if CHIP_TARGET_STYLE_UNIX
    // TapAddrAutoconf and TapInterface are only needed for LwIP on
    // sockets simulation in which a host tap/tun interface is used to
    // proxy the LwIP stack onto a host native network interface.
    // CollectTapAddresses() is only available on such targets.

    sTapIFs.clear();
#endif // CHIP_TARGET_STYLE_UNIX
    sNetIFs.clear();

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
#if CHIP_TARGET_STYLE_UNIX
        // TapAddrAutoconf and TapInterface are only needed for LwIP on
        // sockets simulation in which a host tap/tun interface is used to
        // proxy the LwIP stack onto a host native network interface.
        // CollectTapAddresses() is only available on such targets.

        TapInterface tapIF;
        sTapIFs.push_back(tapIF);
#endif // CHIP_TARGET_STYLE_UNIX
        struct netif netIF;
        sNetIFs.push_back(netIF);
    }

#if CHIP_TARGET_STYLE_UNIX

    // TapAddrAutoconf and TapInterface are only needed for LwIP on
    // sockets simulation in which a host tap/tun interface is used to
    // proxy the LwIP stack onto a host native network interface.
    // CollectTapAddresses() is only available on such targets.

    err_t lwipErr;

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        lwipErr = TapInterface_Init(&(sTapIFs[j]), gNetworkOptions.TapDeviceName[j], NULL);
        if (lwipErr != ERR_OK)
        {
            printf("Failed to initialize tap device %s: %s\n", gNetworkOptions.TapDeviceName[j],
                   ErrorStr(System::MapErrorLwIP(lwipErr)));
            exit(EXIT_FAILURE);
        }
    }
#endif // CHIP_TARGET_STYLE_UNIX
    tcpip_init(OnLwIPInitComplete, NULL);

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        std::vector<char *> addrsVec;

        addrsVec.clear();

#if CHIP_TARGET_STYLE_UNIX

        // TapAddrAutoconf and TapInterface are only needed for LwIP on
        // sockets simulation in which a host tap/tun interface is used to
        // proxy the LwIP stack onto a host native network interface.
        // CollectTapAddresses() is only available on such targets.
        if (gNetworkOptions.TapUseSystemConfig)
        {
            CollectTapAddresses(addrsVec, gNetworkOptions.TapDeviceName[j]);
        }
#endif // CHIP_TARGET_STYLE_UNIX
#if INET_CONFIG_ENABLE_IPV4

        IPAddress ip4Addr = (j < gNetworkOptions.LocalIPv4Addr.size()) ? gNetworkOptions.LocalIPv4Addr[j] : IPAddress::Any;
        for (size_t n = 0; n < addrsVec.size(); n++)
        {
            IPAddress auto_addr;
            if (IPAddress::FromString(addrsVec[n], auto_addr) && auto_addr.IsIPv4())
            {
                ip4Addr = auto_addr;
            }
        }

#if CHIP_TARGET_STYLE_UNIX
        // TapAddrAutoconf and TapInterface are only needed for LwIP on
        // sockets simulation in which a host tap/tun interface is used to
        // proxy the LwIP stack onto a host native network interface.
        // CollectTapAddresses() is only available on such targets.

        IPAddress ip4Gateway = (j < gNetworkOptions.IPv4GatewayAddr.size()) ? gNetworkOptions.IPv4GatewayAddr[j] : IPAddress::Any;

        {
#if LWIP_VERSION_MAJOR > 1
            ip4_addr_t ip4AddrLwIP, ip4NetmaskLwIP, ip4GatewayLwIP;
#else  // LWIP_VERSION_MAJOR <= 1
            ip_addr_t ip4AddrLwIP, ip4NetmaskLwIP, ip4GatewayLwIP;
#endif // LWIP_VERSION_MAJOR <= 1

            ip4AddrLwIP = ip4Addr.ToIPv4();
            IP4_ADDR(&ip4NetmaskLwIP, 255, 255, 255, 0);
            ip4GatewayLwIP = ip4Gateway.ToIPv4();
            netif_add(&(sNetIFs[j]), &ip4AddrLwIP, &ip4NetmaskLwIP, &ip4GatewayLwIP, &(sTapIFs[j]), TapInterface_SetupNetif,
                      tcpip_input);
        }
#endif // CHIP_TARGET_STYLE_UNIX

#endif // INET_CONFIG_ENABLE_IPV4

        netif_create_ip6_linklocal_address(&(sNetIFs[j]), 1);

#if !(LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
        if (j < gNetworkOptions.LocalIPv6Addr.size())
        {
            ip6_addr_t ip6addr = gNetworkOptions.LocalIPv6Addr[j].ToIPv6();
            s8_t index;
            netif_add_ip6_address_with_route(&(sNetIFs[j]), &ip6addr, 64, &index);
            // add ipv6 route for ipv6 address
            if (j < gNetworkOptions.IPv6GatewayAddr.size())
            {
                static ip6_addr_t br_ip6_addr = gNetworkOptions.IPv6GatewayAddr[j].ToIPv6();
                struct ip6_prefix ip6_prefix;
                ip6_prefix.addr       = Inet::IPAddress::Any.ToIPv6();
                ip6_prefix.prefix_len = 0;
                ip6_add_route_entry(&ip6_prefix, &sNetIFs[j], &br_ip6_addr, NULL);
            }
            if (index >= 0)
            {
                netif_ip6_addr_set_state(&(sNetIFs[j]), index, IP6_ADDR_PREFERRED);
            }
        }
        for (size_t n = 0; n < addrsVec.size(); n++)
        {
            IPAddress auto_addr;
            if (IPAddress::FromString(addrsVec[n], auto_addr) && !auto_addr.IsIPv4())
            {
                ip6_addr_t ip6addr = auto_addr.ToIPv6();
                s8_t index;
                if (auto_addr.IsIPv6LinkLocal())
                    continue; // skip over the LLA addresses, LwIP is aready adding those
                if (auto_addr.IsIPv6Multicast())
                    continue; // skip over the multicast addresses from host for now.
                netif_add_ip6_address_with_route(&(sNetIFs[j]), &ip6addr, 64, &index);
                if (index >= 0)
                {
                    netif_ip6_addr_set_state(&(sNetIFs[j]), index, IP6_ADDR_PREFERRED);
                }
            }
        }
#endif

        netif_set_up(&(sNetIFs[j]));
        netif_set_link_up(&(sNetIFs[j]));
    }

    netif_set_default(&(sNetIFs[0]));
    // UnLock LwIP stack

    UNLOCK_TCPIP_CORE();

    while (!NetworkIsReady())
    {
        constexpr uint32_t kSleepTimeMilliseconds = 100;
        ServiceEvents(kSleepTimeMilliseconds);
    }

    // FIXME: this is kinda nasty :(
    // Force new IP address to be ready, bypassing duplicate detection.

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        if (j < gNetworkOptions.LocalIPv6Addr.size())
        {
            netif_ip6_addr_set_state(&(sNetIFs[j]), 2, 0x30);
        }
        else
        {
            netif_ip6_addr_set_state(&(sNetIFs[j]), 1, 0x30);
        }
    }

#if INET_CONFIG_ENABLE_DNS_RESOLVER
    if (gNetworkOptions.DNSServerAddr != IPAddress::Any)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t dnsServerAddr = gNetworkOptions.DNSServerAddr.ToLwIPAddr();
#else // LWIP_VERSION_MAJOR <= 1
#if INET_CONFIG_ENABLE_IPV4
        ip_addr_t dnsServerAddr = gNetworkOptions.DNSServerAddr.ToIPv4();
#else // !INET_CONFIG_ENABLE_IPV4
#error "No support for DNS Resolver without IPv4!"
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5

        dns_setserver(0, &dnsServerAddr);
    }
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    PrintNetworkState();

    AcquireLwIP();
    lContext = sLwIPEventQueue;

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    gInet.Init(gSystemLayer, lContext);
}

void ServiceEvents(uint32_t aSleepTimeMilliseconds)
{
    static bool printed = false;

    if (!printed)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        if (NetworkIsReady())
#endif
        {
            printf("CHIP node ready to service events\n");
            fflush(stdout);
            printed = true;
        }
    }

    // Start a timer (with a no-op callback) to ensure that WaitForEvents() does not block longer than aSleepTimeMilliseconds.
    gSystemLayer.StartTimer(
        aSleepTimeMilliseconds, [](System::Layer *, void *) -> void {}, nullptr);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    gSystemLayer.PrepareEvents();
    gSystemLayer.WaitForEvents();
    gSystemLayer.HandleEvents();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (gSystemLayer.IsInitialized())
    {
        static uint32_t sRemainingSystemLayerEventDelay = 0;

        if (sRemainingSystemLayerEventDelay == 0)
        {
            gSystemLayer.DispatchEvents();
            sRemainingSystemLayerEventDelay = gNetworkOptions.EventDelay;
        }
        else
            sRemainingSystemLayerEventDelay--;

        gSystemLayer.HandlePlatformTimer();
    }
#if CHIP_TARGET_STYLE_UNIX
    // TapAddrAutoconf and TapInterface are only needed for LwIP on
    // sockets simulation in which a host tap/tun interface is used to
    // proxy the LwIP stack onto a host native network interface.
    // CollectTapAddresses() is only available on such targets.

    TapInterface_Select(&(sTapIFs[0]), &(sNetIFs[0]), aSleepTime, gNetworkOptions.TapDeviceName.size());
#endif // CHIP_TARGET_STYLE_UNIX
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
static bool NetworkIsReady()
{
    bool ready = true;

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            if (!ip6_addr_isany(netif_ip6_addr(&(sNetIFs[j]), i)) && ip6_addr_istentative(netif_ip6_addr_state(&(sNetIFs[j]), i)))
            {
                ready = false;
                break;
            }
        }
    }
    return ready;
}

static void OnLwIPInitComplete(void * arg)
{
    printf("Waiting for addresses assignment...\n");
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

void ShutdownNetwork()
{
    gInet.Shutdown();
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    ReleaseLwIP();
#endif
}

void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix, uint32_t rowWidth)
{
    int indexWidth = snprintf(nullptr, 0, "%" PRIX32, len);

    if (indexWidth < 4)
        indexWidth = 4;

    for (uint32_t i = 0; i < len; i += rowWidth)
    {
        printf("%s%0*" PRIX32 ": ", prefix, indexWidth, i);

        uint32_t rowEnd = i + rowWidth;

        uint32_t j = i;
        for (; j < rowEnd && j < len; j++)
            printf("%02X ", mem[j]);

        for (; j < rowEnd; j++)
            printf("   ");

        for (j = i; j < rowEnd && j < len; j++)
            if (isprint(static_cast<char>(mem[j])))
                printf("%c", mem[j]);
            else
                printf(".");

        printf("\n");
    }
}

void DumpMemory(const uint8_t * mem, uint32_t len, const char * prefix)
{
    const uint32_t kRowWidth = 16;

    DumpMemory(mem, len, prefix, kRowWidth);
}
