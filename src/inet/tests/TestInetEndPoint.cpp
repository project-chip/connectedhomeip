/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2018 Nest Labs, Inc.
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
 *     This file implements a unit test suite for InetLayer EndPoint related features
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <CHIPVersion.h>

#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>

#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>

#include <system/SystemError.h>

#include <nlunit-test.h>

#include "TestInetCommon.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::System::Clock::Literals;

#define TOOL_NAME "TestInetEndPoint"

bool callbackHandlerCalled = false;

void HandleDNSResolveComplete(void * appState, CHIP_ERROR err, uint8_t addrCount, IPAddress * addrArray)
{
    callbackHandlerCalled = true;

    if (addrCount > 0)
    {
        char destAddrStr[64];
        addrArray->ToString(destAddrStr);
        printf("    DNS name resolution complete: %s\n", destAddrStr);
    }
    else
        printf("    DNS name resolution return no addresses\n");
}

void HandleTimer(Layer * aLayer, void * aAppState)
{
    printf("    timer handler\n");
}

// Test before init network, Inet is not initialized
static void TestInetPre(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Deinit system layer and network
    ShutdownNetwork();
    if (gSystemLayer.IsInitialized())
    {
        ShutdownSystemLayer();
    }

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    EndPointManager<UDPEndPoint> * udpEndPointManager = gInet.GetUDPEndPointManager();
    NL_TEST_ASSERT(inSuite, udpEndPointManager == nullptr);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    EndPointManager<TCPEndPoint> * tcpEndPointManager = gInet.GetTCPEndPointManager();
    NL_TEST_ASSERT(inSuite, tcpEndPointManager == nullptr);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    err = gSystemLayer.StartTimer(10_ms32, HandleTimer, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // then init network
    InitSystemLayer();
    InitNetwork();
}

static void TestInetError(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = CHIP_ERROR_POSIX(EPERM);
    NL_TEST_ASSERT(inSuite, DescribeErrorPOSIX(err));
    NL_TEST_ASSERT(inSuite, err.IsRange(ChipError::Range::kPOSIX));
}

static void TestInetInterface(nlTestSuite * inSuite, void * inContext)
{
    InterfaceIterator intIterator;
    InterfaceAddressIterator addrIterator;
    char intName[chip::Inet::InterfaceId::kMaxIfNameLength];
    InterfaceId intId;
    IPAddress addr;
    IPPrefix addrWithPrefix;
    CHIP_ERROR err;

#ifndef __MBED__
    // Mbed interface name has different format
    err = InterfaceId::InterfaceNameToId("0", intId);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
#endif

    err = InterfaceId::Null().GetInterfaceName(intName, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = InterfaceId::Null().GetInterfaceName(intName, sizeof(intName));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR && intName[0] == '\0');

    intId = InterfaceId::FromIPAddress(addr);
    NL_TEST_ASSERT(inSuite, !intId.IsPresent());

    err = intId.GetLinkLocalAddr(nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);

    printf("    Interfaces:\n");
    for (; intIterator.HasCurrent(); intIterator.Next())
    {
        intId = intIterator.GetInterfaceId();
        NL_TEST_ASSERT(inSuite, intId.IsPresent());
        memset(intName, 42, sizeof(intName));
        err = intIterator.GetInterfaceName(intName, sizeof(intName));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        printf("     interface id: 0x%" PRIxPTR ", interface name: %s, interface state: %s, %s multicast, %s broadcast addr\n",
#if CHIP_SYSTEM_CONFIG_USE_LWIP
               reinterpret_cast<uintptr_t>(intId.GetPlatformInterface()),
#else
               static_cast<uintptr_t>(intId.GetPlatformInterface()),
#endif
               intName, intIterator.IsUp() ? "UP" : "DOWN", intIterator.SupportsMulticast() ? "supports" : "no",
               intIterator.HasBroadcastAddress() ? "has" : "no");

        intId.GetLinkLocalAddr(&addr);
        InterfaceId::MatchLocalIPv6Subnet(addr);
    }
    NL_TEST_ASSERT(inSuite, !intIterator.Next());
    NL_TEST_ASSERT(inSuite, intIterator.GetInterfaceId() == InterfaceId::Null());
    NL_TEST_ASSERT(inSuite, intIterator.GetInterfaceName(intName, sizeof(intName)) == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !intIterator.SupportsMulticast());
    NL_TEST_ASSERT(inSuite, !intIterator.HasBroadcastAddress());

    printf("    Addresses:\n");
    for (; addrIterator.HasCurrent(); addrIterator.Next())
    {
        addr = addrIterator.GetAddress();
        addrIterator.GetAddressWithPrefix(addrWithPrefix);
        char addrStr[80];
        addrWithPrefix.IPAddr.ToString(addrStr);
        intId = addrIterator.GetInterfaceId();
        NL_TEST_ASSERT(inSuite, intId.IsPresent());
        memset(intName, 42, sizeof(intName));
        err = addrIterator.GetInterfaceName(intName, sizeof(intName));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, intName[0] != '\0' && memchr(intName, '\0', sizeof(intName)) != nullptr);
        printf("     %s/%d, interface id: 0x%" PRIxPTR
               ", interface name: %s, interface state: %s, %s multicast, %s broadcast addr\n",
               addrStr, addrWithPrefix.Length,
#if CHIP_SYSTEM_CONFIG_USE_LWIP
               reinterpret_cast<uintptr_t>(intId.GetPlatformInterface()),
#else
               static_cast<uintptr_t>(intId.GetPlatformInterface()),
#endif
               intName, addrIterator.IsUp() ? "UP" : "DOWN", addrIterator.SupportsMulticast() ? "supports" : "no",
               addrIterator.HasBroadcastAddress() ? "has" : "no");
    }
    NL_TEST_ASSERT(inSuite, !addrIterator.Next());
    addrIterator.GetAddressWithPrefix(addrWithPrefix);
    NL_TEST_ASSERT(inSuite, addrWithPrefix.IsZero());
    NL_TEST_ASSERT(inSuite, addrIterator.GetInterfaceId() == InterfaceId::Null());
    NL_TEST_ASSERT(inSuite, addrIterator.GetInterfaceName(intName, sizeof(intName)) == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !addrIterator.SupportsMulticast());
    NL_TEST_ASSERT(inSuite, !addrIterator.HasBroadcastAddress());
}

static void TestInetEndPointInternal(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    IPAddress addr_any = IPAddress::Any;
    IPAddress addr;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress addr_v4;
#endif // INET_CONFIG_ENABLE_IPV4
    InterfaceId intId;

    // EndPoint
    UDPEndPoint * testUDPEP  = nullptr;
    TCPEndPoint * testTCPEP1 = nullptr;
    PacketBufferHandle buf   = PacketBufferHandle::New(PacketBuffer::kMaxSize);

    // init all the EndPoints
    err = gInet.GetUDPEndPointManager()->NewEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = gInet.GetTCPEndPointManager()->NewEndPoint(&testTCPEP1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = InterfaceId::Null().GetLinkLocalAddr(&addr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    intId = InterfaceId::FromIPAddress(addr);
    NL_TEST_ASSERT(inSuite, intId.IsPresent());

#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.0.0.1", addr_v4));
#endif // INET_CONFIG_ENABLE_IPV4

    // UdpEndPoint special cases to cover the error branch
    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testUDPEP->Bind(IPAddressType::kUnknown, addr_any, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
    err = testUDPEP->Bind(IPAddressType::kUnknown, addr, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(IPAddressType::kIPv4, addr, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4

    err            = testUDPEP->Bind(IPAddressType::kIPv6, addr, 3000, intId);
    err            = testUDPEP->BindInterface(IPAddressType::kIPv6, intId);
    InterfaceId id = testUDPEP->GetBoundInterface();
    NL_TEST_ASSERT(inSuite, id == intId);

    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    err = testUDPEP->Bind(IPAddressType::kIPv6, addr, 3000, intId);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testUDPEP->BindInterface(IPAddressType::kIPv6, intId);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    testUDPEP->Free();

    err = gInet.GetUDPEndPointManager()->NewEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(IPAddressType::kIPv4, addr_v4, 3000, intId);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testUDPEP->SendTo(addr_v4, 3000, std::move(buf));
    testUDPEP->Free();
#endif // INET_CONFIG_ENABLE_IPV4

    // TcpEndPoint special cases to cover the error branch
    err = testTCPEP1->GetPeerInfo(nullptr, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testTCPEP1->Send(std::move(buf), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->EnableKeepAlive(10, 100);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->DisableKeepAlive();
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->AckReceive(10);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !testTCPEP1->PendingReceiveLength());
    err = testTCPEP1->Listen(4);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->GetLocalInfo(nullptr, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    err = testTCPEP1->Bind(IPAddressType::kUnknown, addr_any, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(IPAddressType::kIPv4, addr, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(IPAddressType::kUnknown, addr, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);

    err = testTCPEP1->Bind(IPAddressType::kIPv6, addr_any, 3000, true);
    err = testTCPEP1->Bind(IPAddressType::kIPv6, addr_any, 3000, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->Listen(4);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Connect(addr_v4, 4000, intId);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_IPV4

    testTCPEP1->Free();
}

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
// Test the InetLayer resource limitation
static void TestInetEndPointLimit(nlTestSuite * inSuite, void * inContext)
{
    UDPEndPoint * testUDPEP[INET_CONFIG_NUM_UDP_ENDPOINTS + 1] = { nullptr };
    TCPEndPoint * testTCPEP[INET_CONFIG_NUM_TCP_ENDPOINTS + 1] = { nullptr };

    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: err is not validated EXCEPT the last call
    for (int i = 0; i < INET_CONFIG_NUM_UDP_ENDPOINTS + 1; i++)
        err = gInet.GetUDPEndPointManager()->NewEndPoint(&testUDPEP[i]);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_ENDPOINT_POOL_FULL);

    // TODO: err is not validated EXCEPT the last call
    for (int i = 0; i < INET_CONFIG_NUM_TCP_ENDPOINTS + 1; i++)
        err = gInet.GetTCPEndPointManager()->NewEndPoint(&testTCPEP[i]);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_ENDPOINT_POOL_FULL);

    // Verify same aComplete and aAppState args do not exhaust timer pool
    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
    {
        err = gSystemLayer.StartTimer(10_ms32, HandleTimer, nullptr);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

#if CHIP_SYSTEM_CONFIG_USE_TIMER_POOL
    char numTimersTest[CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1];
    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
        err = gSystemLayer.StartTimer(10_ms32, HandleTimer, &numTimersTest[i]);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_NO_MEMORY);
#endif // CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

    ShutdownNetwork();
    ShutdownSystemLayer();

    // Release UDP endpoints
    for (int i = 0; i < INET_CONFIG_NUM_UDP_ENDPOINTS; i++)
    {
        if (testUDPEP[i] != nullptr)
        {
            testUDPEP[i]->Free();
        }
    }

    // Release TCP endpoints
    for (int i = 0; i < INET_CONFIG_NUM_TCP_ENDPOINTS; i++)
    {
        if (testTCPEP[i] != nullptr)
        {
            testTCPEP[i]->Free();
        }
    }
}
#endif

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("InetEndPoint::PreTest", TestInetPre),
                                 NL_TEST_DEF("InetEndPoint::TestInetError", TestInetError),
                                 NL_TEST_DEF("InetEndPoint::TestInetInterface", TestInetInterface),
                                 NL_TEST_DEF("InetEndPoint::TestInetEndPoint", TestInetEndPointInternal),
#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
                                 NL_TEST_DEF("InetEndPoint::TestEndPointLimit", TestInetEndPointLimit),
#endif
                                 NL_TEST_SENTINEL() };

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    ShutdownNetwork();
    ShutdownSystemLayer();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

int TestInetEndPointInternal()
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // clang-format off
    nlTestSuite theSuite =
    {
        "inet-endpoint",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
#else  // !CHIP_SYSTEM_CONFIG_USE_SOCKETS
    return (0);
#endif // !CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

CHIP_REGISTER_TEST_SUITE(TestInetEndPointInternal)

int TestInetEndPoint()
{
    SetSIGUSR1Handler();

    // Generate machine-readable, comma-separated value (CSV) output.
    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestInetEndPointInternal());
}
