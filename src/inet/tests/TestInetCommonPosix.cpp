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

#include "TestInetCommon.h"
#include "TestInetCommonOptions.h"

#include <assert.h>
#include <errno.h>
#include <vector>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <platform/PlatformManager.h>
#include <system/SystemClock.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/dns.h>
#if (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
#include <lwip/ip6_route_table.h>
#endif // (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#include <netif/etharp.h>

#if CHIP_TARGET_STYLE_UNIX
// NOTE: this used to be supported, so could theoretically be
//       recovered, however the TAP/TUN was never actually used
//       and the underlying files were never compiled in CHIP.
#error "Testing of LWIP on unix systems via TAP not supported."
#endif // CHIP_TARGET_STYLE_UNIX

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

using namespace chip;
using namespace chip::Inet;

System::LayerImpl gSystemLayer;

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
Inet::UDPEndPointManagerImpl gUDP;
#endif

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
Inet::TCPEndPointManagerImpl gTCP;
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
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
#if (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
    if (sLwIPAcquireCount > 0 && --sLwIPAcquireCount == 0)
    {
#if defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
        // FreeRTOS need to delete the task not return from it.
        tcpip_finish(reinterpret_cast<tcpip_will_finish_fn>(vTaskDelete), NULL);
#else  // defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
        tcpip_finish(NULL, NULL);
#endif // defined(INCLUDE_vTaskDelete) && INCLUDE_vTaskDelete
    }
#endif // (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
}

static std::vector<struct netif> sNetIFs; // interface to filter

static bool NetworkIsReady();
static void OnLwIPInitComplete(void * arg);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)

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

void ShutdownTestInetCommon()
{
    chip::Platform::MemoryShutdown();
}

void InitSystemLayer()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // LwIP implementation uses the event loop for servicing events.
    // The CHIP stack initialization is required then.
    chip::DeviceLayer::PlatformMgr().InitChipStack();
#ifndef CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
    AcquireLwIP();
#endif // !CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    gSystemLayer.Init();
}

void ShutdownSystemLayer()
{

    gSystemLayer.Shutdown();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // LwIP implementation uses the event loop for servicing events.
    // The CHIP stack shutdown is required then.
    chip::DeviceLayer::PlatformMgr().Shutdown();
#ifndef CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
    ReleaseLwIP();
#endif // !CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
static void PrintNetworkState()
{
    char intfName[InterfaceId::kMaxIfNameLength];

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        struct netif * netIF = &(sNetIFs[j]);
        InterfaceId(netIF).GetInterfaceName(intfName, sizeof(intfName));

        printf("LwIP interface ready\n");
        printf("  Interface Name: %s\n", intfName);
        printf("  Tap Device: %s\n", gNetworkOptions.TapDeviceName[j]);

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
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)

void InitNetwork()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)

    // If an tap device name hasn't been specified, derive one from the IPv6 interface id.

    if (gNetworkOptions.TapDeviceName.empty())
    {
        for (size_t j = 0; j < gNetworkOptions.LocalIPv6Addr.size(); j++)
        {
            uint64_t iid    = gNetworkOptions.LocalIPv6Addr[j].InterfaceId();
            char * tap_name = (char *) chip::Platform::MemoryAlloc(sizeof(gDefaultTapDeviceName));
            assert(tap_name);
            snprintf(tap_name, sizeof(gDefaultTapDeviceName), "chip-dev-%x", static_cast<uint16_t>(iid));
            gNetworkOptions.TapDeviceName.push_back(tap_name);
        }
    }

    sNetIFs.clear();

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        struct netif netIF;
        sNetIFs.push_back(netIF);
    }

    tcpip_init(OnLwIPInitComplete, NULL);

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    for (size_t j = 0; j < gNetworkOptions.TapDeviceName.size(); j++)
    {
        std::vector<char *> addrsVec;

        addrsVec.clear();

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

#endif // INET_CONFIG_ENABLE_IPV4

        netif_create_ip6_linklocal_address(&(sNetIFs[j]), 1);

#if (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)
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
#endif // (LWIP_VERSION_MAJOR == 2) && (LWIP_VERSION_MINOR == 0)

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

    PrintNetworkState();

    AcquireLwIP();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    gTCP.Init(gSystemLayer);
#endif
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    gUDP.Init(gSystemLayer);
#endif
}

void ServiceEvents(uint32_t aSleepTimeMilliseconds)
{
    static bool printed = false;

    if (!printed)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
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
        System::Clock::Milliseconds32(aSleepTimeMilliseconds), [](System::Layer *, void *) -> void {}, nullptr);

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    gSystemLayer.PrepareEvents();
    gSystemLayer.WaitForEvents();
    gSystemLayer.HandleEvents();
#endif

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    if (gSystemLayer.IsInitialized())
    {
        static uint32_t sRemainingSystemLayerEventDelay = 0;

        if (sRemainingSystemLayerEventDelay == 0)
        {
#if CHIP_DEVICE_LAYER_TARGET_OPEN_IOT_SDK || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
            // We need to terminate event loop after performance single step.
            // Event loop processing work items until StopEventLoopTask is called.
            // Scheduling StopEventLoop task guarantees correct operation of the loop.
            chip::DeviceLayer::PlatformMgr().ScheduleWork(
                [](intptr_t) -> void { chip::DeviceLayer::PlatformMgr().StopEventLoopTask(); }, (intptr_t) nullptr);
#endif // CHIP_DEVICE_LAYER_TARGET_OPEN_IOT_SDK
            chip::DeviceLayer::PlatformMgr().RunEventLoop();
            sRemainingSystemLayerEventDelay = gNetworkOptions.EventDelay;
        }
        else
            sRemainingSystemLayerEventDelay--;

        gSystemLayer.HandlePlatformTimer();
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
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

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)

void ShutdownNetwork()
{

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    gTCP.ForEachEndPoint([](TCPEndPoint * lEndPoint) -> Loop {
        gTCP.ReleaseEndPoint(lEndPoint);
        return Loop::Continue;
    });
    gTCP.Shutdown();
#endif
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    gUDP.ForEachEndPoint([](UDPEndPoint * lEndPoint) -> Loop {
        gUDP.ReleaseEndPoint(lEndPoint);
        return Loop::Continue;
    });
    gUDP.Shutdown();
#endif
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !(CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT)
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
