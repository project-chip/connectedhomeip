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

#include <inet/InetError.h>
#include <inet/InetLayer.h>

#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

#include <system/SystemError.h>
#include <system/SystemTimer.h>

#include <nlunit-test.h>

#include "TestInetCommon.h"
#include "TestSetupSignalling.h"

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;

#define TOOL_NAME "TestInetEndPoint"

bool callbackHandlerCalled = false;

void HandleDNSResolveComplete(void * appState, INET_ERROR err, uint8_t addrCount, IPAddress * addrArray)
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

void HandleTimer(Layer * aLayer, void * aAppState, Error aError)
{
    printf("    timer handler\n");
}

// Test before init network, Inet is not initialized
static void TestInetPre(nlTestSuite * inSuite, void * inContext)
{
#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    RawEndPoint * testRawEP = nullptr;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT
#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    UDPEndPoint * testUDPEP = nullptr;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    TCPEndPoint * testTCPEP = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
    INET_ERROR err = INET_NO_ERROR;
#if INET_CONFIG_ENABLE_DNS_RESOLVER
    IPAddress testDestAddr = IPAddress::Any;
    char testHostName[20]  = "www.nest.com";
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
    err = gInet.NewRawEndPoint(kIPVersion_6, kIPProtocol_ICMPv6, &testRawEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    err = gInet.NewUDPEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    err = gInet.NewTCPEndPoint(&testTCPEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    err = gSystemLayer.StartTimer(10, HandleTimer, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_SYSTEM_ERROR_UNEXPECTED_STATE);

#if INET_CONFIG_ENABLE_DNS_RESOLVER
    err = gInet.ResolveHostAddress(testHostName, 1, &testDestAddr, HandleDNSResolveComplete, nullptr);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

    // then init network
    InitSystemLayer();
    InitNetwork();
}

#if INET_CONFIG_ENABLE_DNS_RESOLVER
// Test Inet ResolveHostAddress functionality
static void TestResolveHostAddress(nlTestSuite * inSuite, void * inContext)
{
    char testHostName1[20] = "www.google.com";
    char testHostName2[20] = "127.0.0.1";
    char testHostName3[20] = "";
    char testHostName4[260];
    struct timeval sleepTime;
    IPAddress testDestAddr[1] = { IPAddress::Any };
    INET_ERROR err;

    sleepTime.tv_sec  = 0;
    sleepTime.tv_usec = 10000;

    memset(testHostName4, 'w', sizeof(testHostName4));
    testHostName4[259] = '\0';

    callbackHandlerCalled = false;
    err = gInet.ResolveHostAddress(testHostName1, 1, testDestAddr, HandleDNSResolveComplete, &callbackHandlerCalled);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    if (err == INET_NO_ERROR)
    {
        while (!callbackHandlerCalled)
        {
            ServiceNetwork(sleepTime);
        }
    }

    callbackHandlerCalled = false;
    err = gInet.ResolveHostAddress(testHostName2, 1, testDestAddr, HandleDNSResolveComplete, &callbackHandlerCalled);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    if (err == INET_NO_ERROR)
    {
        while (!callbackHandlerCalled)
        {
            ServiceNetwork(sleepTime);
        }
    }

    callbackHandlerCalled = false;
    err = gInet.ResolveHostAddress(testHostName3, 1, testDestAddr, HandleDNSResolveComplete, &callbackHandlerCalled);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    if (err == INET_NO_ERROR)
    {
        while (!callbackHandlerCalled)
        {
            ServiceNetwork(sleepTime);
        }
    }

    err = gInet.ResolveHostAddress(testHostName2, 0, testDestAddr, HandleDNSResolveComplete, &callbackHandlerCalled);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_NO_MEMORY);

    err = gInet.ResolveHostAddress(testHostName4, 1, testDestAddr, HandleDNSResolveComplete, &callbackHandlerCalled);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_HOST_NAME_TOO_LONG);
}
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER

// Test Inet ParseHostPortAndInterface
static void TestParseHost(nlTestSuite * inSuite, void * inContext)
{
    char correctHostNames[7][30] = {
        "10.0.0.1", "10.0.0.1:3000", "www.google.com", "www.google.com:3000", "[fd00:0:1:1::1]:3000", "[fd00:0:1:1::1]:300%wpan0",
        "%wpan0"
    };
    char invalidHostNames[4][30] = { "[fd00::1]5", "[fd00:0:1:1::1:3000", "10.0.0.1:1234567", "10.0.0.1:er31" };
    const char * host;
    const char * intf;
    uint16_t port, hostlen, intflen;
    INET_ERROR err;

    for (char * correctHostName : correctHostNames)
    {
        err = ParseHostPortAndInterface(correctHostName, uint16_t(strlen(correctHostName)), host, hostlen, port, intf, intflen);
        NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
    }
    for (char * invalidHostName : invalidHostNames)
    {
        err = ParseHostPortAndInterface(invalidHostName, uint16_t(strlen(invalidHostName)), host, hostlen, port, intf, intflen);
        NL_TEST_ASSERT(inSuite, err == INET_ERROR_INVALID_HOST_NAME);
    }
}

static void TestInetError(nlTestSuite * inSuite, void * inContext)
{
    INET_ERROR err = INET_NO_ERROR;

    err = MapErrorPOSIX(EPERM);
    NL_TEST_ASSERT(inSuite, DescribeErrorPOSIX(err));
    NL_TEST_ASSERT(inSuite, IsErrorPOSIX(err));
}

static void TestInetInterface(nlTestSuite * inSuite, void * inContext)
{
    InterfaceIterator intIterator;
    InterfaceAddressIterator addrIterator;
    char intName[chip::Inet::InterfaceIterator::kMaxIfNameLength];
    InterfaceId intId;
    IPAddress addr;
    IPPrefix addrWithPrefix;
    INET_ERROR err;

    err = InterfaceNameToId("0", intId);
    NL_TEST_ASSERT(inSuite, err != INET_NO_ERROR);

    err = GetInterfaceName(INET_NULL_INTERFACEID, intName, 0);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_NO_MEMORY);

    err = GetInterfaceName(INET_NULL_INTERFACEID, intName, sizeof(intName));
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR && intName[0] == '\0');

    err = gInet.GetInterfaceFromAddr(addr, intId);
    NL_TEST_ASSERT(inSuite, intId == INET_NULL_INTERFACEID);

    err = gInet.GetLinkLocalAddr(intId, nullptr);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_BAD_ARGS);

    printf("    Interfaces:\n");
    for (; intIterator.HasCurrent(); intIterator.Next())
    {
        intId = intIterator.GetInterface();
        NL_TEST_ASSERT(inSuite, intId != INET_NULL_INTERFACEID);
        memset(intName, 42, sizeof(intName));
        err = intIterator.GetInterfaceName(intName, sizeof(intName));
        NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
        printf("     interface id: 0x%" PRIxPTR ", interface name: %s, interface state: %s, %s multicast, %s broadcast addr\n",
#if CHIP_SYSTEM_CONFIG_USE_LWIP
               reinterpret_cast<uintptr_t>(intId),
#else
               static_cast<uintptr_t>(intId),
#endif
               intName, intIterator.IsUp() ? "UP" : "DOWN", intIterator.SupportsMulticast() ? "supports" : "no",
               intIterator.HasBroadcastAddress() ? "has" : "no");

        gInet.GetLinkLocalAddr(intId, &addr);
        gInet.MatchLocalIPv6Subnet(addr);
    }
    NL_TEST_ASSERT(inSuite, !intIterator.Next());
    NL_TEST_ASSERT(inSuite, intIterator.GetInterface() == INET_NULL_INTERFACEID);
    NL_TEST_ASSERT(inSuite, intIterator.GetInterfaceName(intName, sizeof(intName)) == INET_ERROR_INCORRECT_STATE);
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
        NL_TEST_ASSERT(inSuite, intId != INET_NULL_INTERFACEID);
        memset(intName, 42, sizeof(intName));
        err = addrIterator.GetInterfaceName(intName, sizeof(intName));
        NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
        NL_TEST_ASSERT(inSuite, intName[0] != '\0' && memchr(intName, '\0', sizeof(intName)) != nullptr);
        printf("     %s/%d, interface id: 0x%" PRIxPTR
               ", interface name: %s, interface state: %s, %s multicast, %s broadcast addr\n",
               addrStr, addrWithPrefix.Length,
#if CHIP_SYSTEM_CONFIG_USE_LWIP
               reinterpret_cast<uintptr_t>(intId),
#else
               static_cast<uintptr_t>(intId),
#endif
               intName, addrIterator.IsUp() ? "UP" : "DOWN", addrIterator.SupportsMulticast() ? "supports" : "no",
               addrIterator.HasBroadcastAddress() ? "has" : "no");
    }
    NL_TEST_ASSERT(inSuite, !addrIterator.Next());
    addrIterator.GetAddressWithPrefix(addrWithPrefix);
    NL_TEST_ASSERT(inSuite, addrWithPrefix.IsZero());
    NL_TEST_ASSERT(inSuite, addrIterator.GetInterface() == INET_NULL_INTERFACEID);
    NL_TEST_ASSERT(inSuite, addrIterator.GetInterfaceName(intName, sizeof(intName)) == INET_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !addrIterator.SupportsMulticast());
    NL_TEST_ASSERT(inSuite, !addrIterator.HasBroadcastAddress());
}

static void TestInetEndPointInternal(nlTestSuite * inSuite, void * inContext)
{
    INET_ERROR err;
    IPAddress addr_any = IPAddress::Any;
    IPAddress addr;
    InterfaceAddressIterator addrIterator;
#if INET_CONFIG_ENABLE_IPV4
    IPAddress addr_v4;
#endif // INET_CONFIG_ENABLE_IPV4
    InterfaceId intId;

    // EndPoint
    RawEndPoint * testRaw6EP = nullptr;
#if INET_CONFIG_ENABLE_IPV4
    RawEndPoint * testRaw4EP = nullptr;
#endif // INET_CONFIG_ENABLE_IPV4
    UDPEndPoint * testUDPEP  = nullptr;
    TCPEndPoint * testTCPEP1 = nullptr;
    PacketBufferHandle buf   = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    bool didBind             = false;
    bool didListen           = false;

    IPAddress tmpAddr    = addrIterator.GetAddress();
    bool IsIPv6Available = tmpAddr.IsIPv6();

    // init all the EndPoints
    err = gInet.NewRawEndPoint(kIPVersion_6, kIPProtocol_ICMPv6, &testRaw6EP);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

#if INET_CONFIG_ENABLE_IPV4
    err = gInet.NewRawEndPoint(kIPVersion_4, kIPProtocol_ICMPv4, &testRaw4EP);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
#endif // INET_CONFIG_ENABLE_IPV4

    err = gInet.NewUDPEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    err = gInet.NewTCPEndPoint(&testTCPEP1);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    err = gInet.GetLinkLocalAddr(INET_NULL_INTERFACEID, &addr);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
    err = gInet.GetInterfaceFromAddr(addr, intId);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);

    // RawEndPoint special cases to cover the error branch
    uint8_t ICMP6Types[2] = { 128, 129 };
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.0.0.1", addr_v4));
#endif // INET_CONFIG_ENABLE_IPV4

    // error bind cases
    err = testRaw6EP->Bind(kIPAddressType_Unknown, addr_any);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testRaw6EP->Bind(kIPAddressType_IPv4, addr);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
    err = testRaw6EP->BindIPv6LinkLocal(intId, addr_v4);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4
    err = testRaw6EP->BindInterface(kIPAddressType_Unknown, INET_NULL_INTERFACEID);
    NL_TEST_ASSERT(inSuite, err != INET_NO_ERROR);

    // A bind should succeed with appropriate permissions but will
    // otherwise fail.
    if (IsIPv6Available)
    {
        err = testRaw6EP->BindIPv6LinkLocal(intId, addr);
        NL_TEST_ASSERT(inSuite, (err == INET_NO_ERROR) || (err == System::MapErrorPOSIX(EPERM)));
    }
    didBind = (err == INET_NO_ERROR);

    // Listen after bind should succeed if the prior bind succeeded.

    err = testRaw6EP->Listen();
    NL_TEST_ASSERT(inSuite, (didBind && (err == INET_NO_ERROR)) || (!didBind && (err == INET_ERROR_INCORRECT_STATE)));

    didListen = (err == INET_NO_ERROR);

    // If the first listen succeeded, then the second listen should be successful.

    err = testRaw6EP->Listen();
    NL_TEST_ASSERT(inSuite, (didBind && didListen && (err == INET_NO_ERROR)) || (!didBind && (err == INET_ERROR_INCORRECT_STATE)));

    didListen = (err == INET_NO_ERROR);

    // A bind-after-listen should result in an incorrect state error;
    // otherwise, it will fail with a permissions error.
    if (IsIPv6Available)
    {
        err = testRaw6EP->Bind(kIPAddressType_IPv6, addr);
        NL_TEST_ASSERT(inSuite,
                       (didListen && (err == INET_ERROR_INCORRECT_STATE)) || (!didListen && (err == System::MapErrorPOSIX(EPERM))));
    }
    // error SetICMPFilter case
    err = testRaw6EP->SetICMPFilter(0, ICMP6Types);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_BAD_ARGS);

#if INET_CONFIG_ENABLE_IPV4
    // We should never be able to send an IPv4-addressed message on an
    // IPv6 raw socket.
    //
    // Ostensibly the address obtained above from
    // gInet.GetLinkLocalAddr(INET_NULL_INTERFACEID, &addr) is an IPv6
    // LLA; however, make sure it actually is.

    NL_TEST_ASSERT(inSuite, addr.Type() == kIPAddressType_IPv6);

    err = testRaw4EP->SendTo(addr, std::move(buf));
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
    testRaw4EP->Free();
#endif // INET_CONFIG_ENABLE_IPV4

    // UdpEndPoint special cases to cover the error branch
    err = testUDPEP->Listen();
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testUDPEP->Bind(kIPAddressType_Unknown, addr_any, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
    err = testUDPEP->Bind(kIPAddressType_Unknown, addr, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(kIPAddressType_IPv4, addr, 3000);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4

    err            = testUDPEP->Bind(kIPAddressType_IPv6, addr, 3000, intId);
    err            = testUDPEP->BindInterface(kIPAddressType_IPv6, intId);
    InterfaceId id = testUDPEP->GetBoundInterface();
    NL_TEST_ASSERT(inSuite, id == intId);

    err = testUDPEP->Listen();
    err = testUDPEP->Listen();
    if (IsIPv6Available)
    {
        err = testUDPEP->Bind(kIPAddressType_IPv6, addr, 3000, intId);
        NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    }
#if HAVE_SO_BINDTODEVICE
    err = testUDPEP->BindInterface(kIPAddressType_IPv6, intId);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif
    testUDPEP->Free();

    err = gInet.NewUDPEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == INET_NO_ERROR);
#if INET_CONFIG_ENABLE_IPV4
    err = testUDPEP->Bind(kIPAddressType_IPv4, addr_v4, 3000, intId);
    NL_TEST_ASSERT(inSuite, err != INET_NO_ERROR);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testUDPEP->SendTo(addr_v4, 3000, std::move(buf));
    testUDPEP->Free();
#endif // INET_CONFIG_ENABLE_IPV4

    // TcpEndPoint special cases to cover the error branch
    err = testTCPEP1->GetPeerInfo(nullptr, nullptr);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    buf = PacketBufferHandle::New(PacketBuffer::kMaxSize);
    err = testTCPEP1->Send(std::move(buf), false);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testTCPEP1->EnableKeepAlive(10, 100);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testTCPEP1->DisableKeepAlive();
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testTCPEP1->AckReceive(10);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    NL_TEST_ASSERT(inSuite, !testTCPEP1->PendingReceiveLength());
    err = testTCPEP1->Listen(4);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testTCPEP1->GetLocalInfo(nullptr, nullptr);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);

    err = testTCPEP1->Bind(kIPAddressType_Unknown, addr_any, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(kIPAddressType_IPv4, addr, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Bind(kIPAddressType_Unknown, addr, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);

    err = testTCPEP1->Bind(kIPAddressType_IPv6, addr_any, 3000, true);
    err = testTCPEP1->Bind(kIPAddressType_IPv6, addr_any, 3000, true);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
    err = testTCPEP1->Listen(4);
#if INET_CONFIG_ENABLE_IPV4
    err = testTCPEP1->Connect(addr_v4, 4000, intId);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_INCORRECT_STATE);
#endif // INET_CONFIG_ENABLE_IPV4

    testTCPEP1->Shutdown();
}

// Test the InetLayer resource limitation
static void TestInetEndPointLimit(nlTestSuite * inSuite, void * inContext)
{
    RawEndPoint * testRawEP = nullptr;
    UDPEndPoint * testUDPEP = nullptr;
    TCPEndPoint * testTCPEP = nullptr;
    INET_ERROR err;
    char numTimersTest[CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1];

    for (int i = 0; i < INET_CONFIG_NUM_RAW_ENDPOINTS + 1; i++)
        err = gInet.NewRawEndPoint(kIPVersion_6, kIPProtocol_ICMPv6, &testRawEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_NO_ENDPOINTS);

    for (int i = 0; i < INET_CONFIG_NUM_UDP_ENDPOINTS + 1; i++)
        err = gInet.NewUDPEndPoint(&testUDPEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_NO_ENDPOINTS);

    for (int i = 0; i < INET_CONFIG_NUM_TCP_ENDPOINTS + 1; i++)
        err = gInet.NewTCPEndPoint(&testTCPEP);
    NL_TEST_ASSERT(inSuite, err == INET_ERROR_NO_ENDPOINTS);

    // Verify same aComplete and aAppState args do not exhaust timer pool
    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
    {
        err = gSystemLayer.StartTimer(10, HandleTimer, nullptr);
        NL_TEST_ASSERT(inSuite, err == CHIP_SYSTEM_NO_ERROR);
    }

    for (int i = 0; i < CHIP_SYSTEM_CONFIG_NUM_TIMERS + 1; i++)
        err = gSystemLayer.StartTimer(10, HandleTimer, &numTimersTest[i]);
    NL_TEST_ASSERT(inSuite, err == CHIP_SYSTEM_ERROR_NO_MEMORY);

    ShutdownNetwork();
    ShutdownSystemLayer();
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("InetEndPoint::PreTest", TestInetPre),
#if INET_CONFIG_ENABLE_DNS_RESOLVER
                                 NL_TEST_DEF("InetEndPoint::ResolveHostAddress", TestResolveHostAddress),
#endif // INET_CONFIG_ENABLE_DNS_RESOLVER
                                 NL_TEST_DEF("InetEndPoint::TestParseHost", TestParseHost),
                                 NL_TEST_DEF("InetEndPoint::TestInetError", TestInetError),
                                 NL_TEST_DEF("InetEndPoint::TestInetInterface", TestInetInterface),
                                 NL_TEST_DEF("InetEndPoint::TestInetEndPoint", TestInetEndPointInternal),
                                 NL_TEST_DEF("InetEndPoint::TestEndPointLimit", TestInetEndPointLimit),
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
