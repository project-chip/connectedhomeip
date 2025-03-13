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
 *     This file implements a unit test suite for Inet EndPoint related features
 *
 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <CHIPVersion.h>
#include <inet/IPPrefix.h>
#include <inet/InetError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

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

class TestInetEndPoint : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        SetSIGUSR1Handler();
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        ShutdownNetwork();
        ShutdownSystemLayer();
        chip::Platform::MemoryShutdown();
    }
};

// Test before init network, Inet is not initialized
TEST_F(TestInetEndPoint, TestInetPre)
{
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    UDPEndPoint * testUDPEP = nullptr;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint * testTCPEP = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Deinit system layer and network
    ShutdownNetwork();
    if (gSystemLayer.IsInitialized())
    {
        ShutdownSystemLayer();
    }

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    err = gUDP.NewEndPoint(&testUDPEP);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = gTCP.NewEndPoint(&testTCPEP);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    err = gSystemLayer.StartTimer(10_ms32, HandleTimer, nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // then init network
    InitSystemLayer();
    InitNetwork();
}

TEST_F(TestInetEndPoint, TestInetError)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = CHIP_ERROR_POSIX(EPERM);
    EXPECT_TRUE(DescribeErrorPOSIX(err));
    EXPECT_TRUE(err.IsRange(ChipError::Range::kPOSIX));
}

TEST_F(TestInetEndPoint, TestInetInterface)
{
    InterfaceIterator intIterator;
    InterfaceAddressIterator addrIterator;
    char intName[InterfaceId::kMaxIfNameLength];
    InterfaceId intId;
    IPAddress addr;
    InterfaceType intType;
    // 64 bit IEEE MAC address
    const uint8_t kMaxHardwareAddressSize = 8;
    uint8_t intHwAddress[kMaxHardwareAddressSize];
    uint8_t intHwAddressSize;

    CHIP_ERROR err;

#ifndef __MBED__
    // Mbed interface name has different format
    err = InterfaceId::InterfaceNameToId("0", intId);
    EXPECT_NE(err, CHIP_NO_ERROR);
#endif

    err = InterfaceId::Null().GetInterfaceName(intName, 0);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = InterfaceId::Null().GetInterfaceName(intName, sizeof(intName));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(intName[0], '\0');

    intId = InterfaceId::FromIPAddress(addr);
    EXPECT_FALSE(intId.IsPresent());

    err = intId.GetLinkLocalAddr(nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);

    printf("    Interfaces:\n");
    for (; intIterator.HasCurrent(); intIterator.Next())
    {
        intId = intIterator.GetInterfaceId();
        EXPECT_TRUE(intId.IsPresent());
        memset(intName, 42, sizeof(intName));
        err = intIterator.GetInterfaceName(intName, sizeof(intName));
        EXPECT_EQ(err, CHIP_NO_ERROR);
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

        // Not all platforms support getting interface type and hardware address
        err = intIterator.GetInterfaceType(intType);
        EXPECT_TRUE(err == CHIP_NO_ERROR || err == CHIP_ERROR_NOT_IMPLEMENTED);

        err = intIterator.GetHardwareAddress(intHwAddress, intHwAddressSize, sizeof(intHwAddress));
        EXPECT_TRUE(err == CHIP_NO_ERROR || err == CHIP_ERROR_NOT_IMPLEMENTED);
        if (err == CHIP_NO_ERROR)
        {
            EXPECT_TRUE(intHwAddressSize == 6 || intHwAddressSize == 8);
            EXPECT_EQ(intIterator.GetHardwareAddress(nullptr, intHwAddressSize, sizeof(intHwAddress)), CHIP_ERROR_INVALID_ARGUMENT);
            EXPECT_EQ(intIterator.GetHardwareAddress(intHwAddress, intHwAddressSize, 4), CHIP_ERROR_BUFFER_TOO_SMALL);
        }
    }

    EXPECT_FALSE(intIterator.Next());
    EXPECT_EQ(intIterator.GetInterfaceId(), InterfaceId::Null());
    EXPECT_EQ(intIterator.GetInterfaceName(intName, sizeof(intName)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(intIterator.SupportsMulticast());
    EXPECT_FALSE(intIterator.HasBroadcastAddress());

    // Not all platforms support getting interface type and hardware address
    err = intIterator.GetInterfaceType(intType);
    EXPECT_TRUE(err == CHIP_ERROR_INCORRECT_STATE || err == CHIP_ERROR_NOT_IMPLEMENTED);
    err = intIterator.GetHardwareAddress(intHwAddress, intHwAddressSize, sizeof(intHwAddress));
    EXPECT_TRUE(err == CHIP_ERROR_INCORRECT_STATE || err == CHIP_ERROR_NOT_IMPLEMENTED);

    printf("    Addresses:\n");
    for (; addrIterator.HasCurrent(); addrIterator.Next())
    {
        err = addrIterator.GetAddress(addr);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        IPPrefix addrWithPrefix(addr, addrIterator.GetPrefixLength());
        char addrStr[80];
        addrWithPrefix.IPAddr.ToString(addrStr);
        intId = addrIterator.GetInterfaceId();
        EXPECT_TRUE(intId.IsPresent());
        memset(intName, 42, sizeof(intName));
        err = addrIterator.GetInterfaceName(intName, sizeof(intName));
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_TRUE(intName[0] != '\0' && memchr(intName, '\0', sizeof(intName)) != nullptr);
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
    EXPECT_FALSE(addrIterator.Next());
    EXPECT_EQ(addrIterator.GetAddress(addr), CHIP_ERROR_SENTINEL);
    EXPECT_EQ(addrIterator.GetInterfaceId(), InterfaceId::Null());
    EXPECT_EQ(addrIterator.GetInterfaceName(intName, sizeof(intName)), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(addrIterator.SupportsMulticast());
    EXPECT_FALSE(addrIterator.HasBroadcastAddress());
}

TEST_F(TestInetEndPoint, TestInetEndPointInternal)
{
    CHIP_ERROR err;
    IPAddress addr_any = IPAddress::Any;
    IPAddress addr;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress addr_v4;
#endif // INET_CONFIG_ENABLE_IPV4
    InterfaceId intId;

    // EndPoint
    UDPEndPoint * testUDPEP = nullptr;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint * testTCPEP1 = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    PacketBufferHandle buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);

    // init all the EndPoints
    SYSTEM_STATS_RESET(System::Stats::kInetLayer_NumUDPEps);
    err = gUDP.NewEndPoint(&testUDPEP);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, 1));

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    SYSTEM_STATS_RESET(System::Stats::kInetLayer_NumTCPEps);
    err = gTCP.NewEndPoint(&testTCPEP1);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumTCPEps, 1));
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    err = InterfaceId::Null().GetLinkLocalAddr(&addr);

    // We should skip the following checks if the interface does not have the Link local address
    ASSERT_NE(err, INET_ERROR_ADDRESS_NOT_FOUND);

    EXPECT_EQ(err, CHIP_NO_ERROR);
    intId = InterfaceId::FromIPAddress(addr);
    EXPECT_TRUE(intId.IsPresent());

#if INET_CONFIG_ENABLE_IPV4
    EXPECT_TRUE(IPAddress::FromString("10.0.0.1", addr_v4));
#endif // INET_CONFIG_ENABLE_IPV4

    // UdpEndPoint special cases to cover the error branch
    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testUDPEP->Bind(IPAddressType::kUnknown, addr_any, 3000);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);
    err = testUDPEP->Bind(IPAddressType::kUnknown, addr, 3000);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(IPAddressType::kIPv4, addr, 3000);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4

    err            = testUDPEP->Bind(IPAddressType::kIPv6, addr, 3000, intId);
    err            = testUDPEP->BindInterface(IPAddressType::kIPv6, intId);
    InterfaceId id = testUDPEP->GetBoundInterface();
    EXPECT_EQ(id, intId);

    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    err = testUDPEP->Listen(nullptr /*OnMessageReceived*/, nullptr /*OnReceiveError*/);
    err = testUDPEP->Bind(IPAddressType::kIPv6, addr, 3000, intId);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testUDPEP->BindInterface(IPAddressType::kIPv6, intId);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    testUDPEP->Free();
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, 0));
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumUDPEps, 1));

    err = gUDP.NewEndPoint(&testUDPEP);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, 1));
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(IPAddressType::kIPv4, addr_v4, 3000, intId);
    EXPECT_NE(err, CHIP_NO_ERROR);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testUDPEP->SendTo(addr_v4, 3000, std::move(buf));
#endif // INET_CONFIG_ENABLE_IPV4
    testUDPEP->Free();
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, 0));

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // TcpEndPoint special cases to cover the error branch
    err = testTCPEP1->GetPeerInfo(nullptr, nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testTCPEP1->Send(std::move(buf), false);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->EnableKeepAlive(10, 100);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->DisableKeepAlive();
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->AckReceive(10);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(testTCPEP1->PendingReceiveLength());
    err = testTCPEP1->Listen(4);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->GetLocalInfo(nullptr, nullptr);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    err = testTCPEP1->Bind(IPAddressType::kUnknown, addr_any, 3000, true);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(IPAddressType::kIPv4, addr, 3000, true);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(IPAddressType::kUnknown, addr, 3000, true);
    EXPECT_EQ(err, INET_ERROR_WRONG_ADDRESS_TYPE);

    err = testTCPEP1->Bind(IPAddressType::kIPv6, addr_any, 3000, true);
    err = testTCPEP1->Bind(IPAddressType::kIPv6, addr_any, 3000, true);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
    err = testTCPEP1->Listen(4);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Connect(addr_v4, 4000, intId);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_IPV4

    testTCPEP1->Free();
    EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumTCPEps, 0));
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumTCPEps, 1));
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
}

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
// Test the Inet resource limitations.
TEST_F(TestInetEndPoint, TestInetEndPointLimit)
{
    UDPEndPoint * testUDPEP[INET_CONFIG_NUM_UDP_ENDPOINTS + 1] = { nullptr };
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint * testTCPEP[INET_CONFIG_NUM_TCP_ENDPOINTS + 1] = { nullptr };
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    CHIP_ERROR err = CHIP_NO_ERROR;

    int udpCount = 0;
    SYSTEM_STATS_RESET(System::Stats::kInetLayer_NumUDPEps);
    for (int i = INET_CONFIG_NUM_UDP_ENDPOINTS; i >= 0; --i)
    {
        err = gUDP.NewEndPoint(&testUDPEP[i]);
        EXPECT_EQ(err, (i ? CHIP_NO_ERROR : CHIP_ERROR_ENDPOINT_POOL_FULL));
        if (err == CHIP_NO_ERROR)
        {
            ++udpCount;
            EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, udpCount));
        }
    }
    const int udpHighWaterMark = udpCount;
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumUDPEps, udpHighWaterMark));

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    int tcpCount = 0;
    SYSTEM_STATS_RESET(System::Stats::kInetLayer_NumTCPEps);
    for (int i = INET_CONFIG_NUM_TCP_ENDPOINTS; i >= 0; --i)
    {
        err = gTCP.NewEndPoint(&testTCPEP[i]);
        EXPECT_EQ(err, (i ? CHIP_NO_ERROR : CHIP_ERROR_ENDPOINT_POOL_FULL));
        if (err == CHIP_NO_ERROR)
        {
            ++tcpCount;
            EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumTCPEps, tcpCount));
        }
    }
    const int tcpHighWaterMark = tcpCount;
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumTCPEps, tcpHighWaterMark));
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if CHIP_SYSTEM_CONFIG_NUM_TIMERS
    // Verify same aComplete and aAppState args do not exhaust timer pool
    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
    {
        err = gSystemLayer.StartTimer(10_ms32, HandleTimer, nullptr);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    char numTimersTest[CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1];
    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
        err = gSystemLayer.StartTimer(10_ms32, HandleTimer, &numTimersTest[i]);
    EXPECT_EQ(err, CHIP_ERROR_NO_MEMORY);
#endif // CHIP_SYSTEM_CONFIG_NUM_TIMERS

    // Release UDP endpoints
    for (int i = 0; i <= INET_CONFIG_NUM_UDP_ENDPOINTS; i++)
    {
        if (testUDPEP[i] != nullptr)
        {
            testUDPEP[i]->Free();
            --udpCount;
            EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumUDPEps, udpCount));
        }
    }
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumUDPEps, udpHighWaterMark));

    // Release TCP endpoints
    for (int i = 0; i <= INET_CONFIG_NUM_TCP_ENDPOINTS; i++)
    {
        if (testTCPEP[i] != nullptr)
        {
            testTCPEP[i]->Free();
            --tcpCount;
            EXPECT_TRUE(SYSTEM_STATS_TEST_IN_USE(System::Stats::kInetLayer_NumTCPEps, tcpCount));
        }
    }
    EXPECT_TRUE(SYSTEM_STATS_TEST_HIGH_WATER_MARK(System::Stats::kInetLayer_NumTCPEps, tcpHighWaterMark));

    ShutdownNetwork();
    ShutdownSystemLayer();
}
#endif // !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
