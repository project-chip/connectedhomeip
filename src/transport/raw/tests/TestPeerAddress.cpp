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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <inet/IPAddress.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/PeerId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <transport/raw/PeerAddress.h>

namespace {

using namespace chip;
using chip::Inet::InterfaceId;
using chip::Inet::IPAddress;
using chip::Transport::PeerAddress;

/**
 *  Test correct identification of IPv6 multicast addresses.
 */
TEST(TestPeerAddress, TestPeerAddressMulticast)
{
    constexpr chip::FabricId fabric = 0xa1a2a4a8b1b2b4b8;
    constexpr chip::GroupId group   = 0xe10f;
    PeerAddress addr                = PeerAddress::Multicast(fabric, group);
    EXPECT_EQ(chip::Transport::Type::kUdp, addr.GetTransportType());
    EXPECT_TRUE(addr.IsMulticast());

    const Inet::IPAddress & ip = addr.GetIPAddress();
    EXPECT_TRUE(ip.IsIPv6Multicast());
    EXPECT_EQ(chip::Inet::IPAddressType::kIPv6, ip.Type());

    constexpr uint8_t expected[NL_INET_IPV6_ADDR_LEN_IN_BYTES] = { 0xff, 0x35, 0x00, 0x40, 0xfd, 0xa1, 0xa2, 0xa4,
                                                                   0xa8, 0xb1, 0xb2, 0xb4, 0xb8, 0x00, 0xe1, 0x0f };
    uint8_t result[NL_INET_IPV6_ADDR_LEN_IN_BYTES];
    uint8_t * p = result;
    ip.WriteAddress(p);
    EXPECT_EQ(0, memcmp(expected, result, NL_INET_IPV6_ADDR_LEN_IN_BYTES));
}

TEST(TestPeerAddress, TestToString)
{
    char buff[PeerAddress::kMaxToStringSize];
    IPAddress ip;
    {
        IPAddress::FromString("::1", ip);
        PeerAddress::UDP(ip, 1122).ToString(buff);

        EXPECT_STREQ(buff, "UDP:[::1]:1122");
    }

    {
        IPAddress::FromString("::1", ip);
        PeerAddress::TCP(ip, 1122).ToString(buff);

        EXPECT_STREQ(buff, "TCP:[::1]:1122");
    }

    {
        PeerAddress::BLE().ToString(buff);
        EXPECT_STREQ(buff, "BLE");
    }

    {
        IPAddress::FromString("1223::3456:789a", ip);
        PeerAddress::UDP(ip, 8080).ToString(buff);
        // IPV6 does not specify case
        int res1 = strcmp(buff, "UDP:[1223::3456:789a]:8080");
        int res2 = strcmp(buff, "UDP:[1223::3456:789A]:8080");

        EXPECT_TRUE(!res1 || !res2);
    }

    {

        PeerAddress udp = PeerAddress(Transport::Type::kUdp);
        udp.SetPort(5840);
        udp.ToString(buff);
        EXPECT_STREQ(buff, "UDP:[::]:5840");
    }
}

TEST(TestPeerAddress, TestEqualityOperator)
{
    using chip::Inet::InterfaceId;
    using chip::Inet::IPAddress;
    using chip::Transport::PeerAddress;
    using chip::Transport::Type;

    IPAddress ip1, ip2;
    IPAddress::FromString("2001:db8::1", ip1);
    IPAddress::FromString("2001:db8::2", ip2);

    InterfaceId iface1 = InterfaceId::Null();

    // 1. Same UDP address, port, interface ? equal
    PeerAddress udp1 = PeerAddress::UDP(ip1, 1234).SetInterface(iface1);
    PeerAddress udp2 = PeerAddress::UDP(ip1, 1234).SetInterface(iface1);
    EXPECT_TRUE(udp1 == udp2);

    // 2. Different IPv6 address ? not equal
    PeerAddress udp3 = PeerAddress::UDP(ip2, 1234).SetInterface(iface1);
    EXPECT_TRUE(udp1 != udp3);

    // 3. Different port ? not equal
    PeerAddress udp4 = PeerAddress::UDP(ip1, 4321).SetInterface(iface1);
    EXPECT_FALSE(udp1 == udp4);

    // 4. TCP and UDP with same IP, port, interface ? not equal
    PeerAddress tcp1 = PeerAddress::TCP(ip1, 1234).SetInterface(iface1);
    EXPECT_TRUE(udp1 != tcp1);

    // 5. BLE transport (no additional fields) ? equal if same type
    PeerAddress ble1 = PeerAddress::BLE();
    PeerAddress ble2 = PeerAddress::BLE();
    EXPECT_TRUE(ble1 == ble2);

    // 6. NFC transport with same short ID ? equal
    PeerAddress nfc1 = PeerAddress::NFC(100);
    PeerAddress nfc2 = PeerAddress::NFC(100);
    EXPECT_TRUE(nfc1 == nfc2);

    // 7. NFC transport with different short ID ? not equal
    PeerAddress nfc3 = PeerAddress::NFC(101);
    EXPECT_FALSE(nfc1 == nfc3);

    // 8. WiFiPAF transport with same remote ID ? equal
    constexpr chip::NodeId nodeId1 = 0x123456789ABCDEF0;
    constexpr chip::NodeId nodeId2 = 0x123456789ABCDEF0;
    PeerAddress wifi1              = PeerAddress::WiFiPAF(nodeId1);
    PeerAddress wifi2              = PeerAddress::WiFiPAF(nodeId2);
    EXPECT_TRUE(wifi1 == wifi2);

    // 9. WiFiPAF transport with different remote ID ? not equal
    constexpr chip::NodeId nodeId3 = 0x0FEDCBA987654321;
    PeerAddress wifi3              = PeerAddress::WiFiPAF(nodeId3);
    EXPECT_FALSE(wifi1 == wifi3);

    // 10. Cross-type comparisons: BLE != NFC, BLE != UDP, BLE != TCP, NFC != UDP, NFC != TCP, UDP != WiFiPAF
    EXPECT_FALSE(ble1 == nfc1);
    EXPECT_FALSE(ble1 == udp1);
    EXPECT_FALSE(ble1 == tcp1);
    EXPECT_FALSE(nfc1 == udp1);
    EXPECT_FALSE(nfc1 == tcp1);
    EXPECT_FALSE(udp1 == wifi1);
}

} // namespace
