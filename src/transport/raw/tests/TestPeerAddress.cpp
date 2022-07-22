/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <inet/IPAddress.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/PeerId.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/raw/PeerAddress.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using chip::Inet::InterfaceId;
using chip::Inet::IPAddress;
using chip::Transport::PeerAddress;

/**
 *  Test correct identification of IPv6 multicast addresses.
 */
void TestPeerAddressMulticast(nlTestSuite * inSuite, void * inContext)
{
    constexpr chip::FabricId fabric = 0xa1a2a4a8b1b2b4b8;
    constexpr chip::GroupId group   = 0xe10f;
    PeerAddress addr                = PeerAddress::Multicast(fabric, group);
    NL_TEST_ASSERT(inSuite, chip::Transport::Type::kUdp == addr.GetTransportType());
    NL_TEST_ASSERT(inSuite, addr.IsMulticast());

    const Inet::IPAddress & ip = addr.GetIPAddress();
    NL_TEST_ASSERT(inSuite, ip.IsIPv6Multicast());
    NL_TEST_ASSERT(inSuite, chip::Inet::IPAddressType::kIPv6 == ip.Type());

    constexpr uint8_t expected[NL_INET_IPV6_ADDR_LEN_IN_BYTES] = { 0xff, 0x35, 0x00, 0x40, 0xfd, 0xa1, 0xa2, 0xa4,
                                                                   0xa8, 0xb1, 0xb2, 0xb4, 0xb8, 0x00, 0xe1, 0x0f };
    uint8_t result[NL_INET_IPV6_ADDR_LEN_IN_BYTES];
    uint8_t * p = result;
    ip.WriteAddress(p);
    NL_TEST_ASSERT(inSuite, !memcmp(expected, result, NL_INET_IPV6_ADDR_LEN_IN_BYTES));
}

void TestToString(nlTestSuite * inSuite, void * inContext)
{
    char buff[PeerAddress::kMaxToStringSize];
    IPAddress ip;
    {
        IPAddress::FromString("::1", ip);
        PeerAddress::UDP(ip, 1122).ToString(buff);

        NL_TEST_ASSERT(inSuite, !strcmp(buff, "UDP:[::1]:1122"));
    }

    {
        IPAddress::FromString("::1", ip);
        PeerAddress::TCP(ip, 1122).ToString(buff);

        NL_TEST_ASSERT(inSuite, !strcmp(buff, "TCP:[::1]:1122"));
    }

    {
        PeerAddress::BLE().ToString(buff);
        NL_TEST_ASSERT(inSuite, !strcmp(buff, "BLE"));
    }

    {
        IPAddress::FromString("1223::3456:789a", ip);
        PeerAddress::UDP(ip, 8080).ToString(buff);

        NL_TEST_ASSERT(inSuite, !strcmp(buff, "UDP:[1223::3456:789a]:8080"));
    }

    {

        PeerAddress udp = PeerAddress(Transport::Type::kUdp);
        udp.SetPort(5840);
        udp.ToString(buff);
        NL_TEST_ASSERT(inSuite, !strcmp(buff, "UDP:[::]:5840"));
    }
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("PeerAddress Multicast", TestPeerAddressMulticast),
    NL_TEST_DEF("ToString", TestToString),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestPeerAddress(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "PeerAddress",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPeerAddress)
