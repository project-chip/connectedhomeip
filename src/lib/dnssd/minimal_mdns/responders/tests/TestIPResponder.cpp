/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/dnssd/minimal_mdns/responders/IP.h>

#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy_DefaultImpl.h>
#include <lib/support/CHIPMem.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace mdns::Minimal;

constexpr uint16_t kMdnsPort = 5353;

const QNamePart kNames[] = { "some", "test", "local" };

class IPResponseAccumulator : public ResponderDelegate
{
public:
    void AddResponse(const ResourceRecord & record) override
    {

        EXPECT_TRUE((record.GetType() == QType::A) || (record.GetType() == QType::AAAA));
        EXPECT_EQ(record.GetClass(), QClass::IN_FLUSH);
        EXPECT_EQ(record.GetName(), kNames);
    }
};

InterfaceId FindValidInterfaceId()
{
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        if (it.IsUp() && it.HasBroadcastAddress())
        {
            return it.GetInterfaceId();
        }
    }
    return InterfaceId::Null();
}

class TestIPResponder : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        mdns::Minimal::SetDefaultAddressPolicy();
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestIPResponder, TestIPv4)
{
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("10.20.30.40", ipAddress));

    IPv4Responder responder(kNames);

    EXPECT_EQ(responder.GetQClass(), QClass::IN);
    EXPECT_EQ(responder.GetQType(), QType::A);
    EXPECT_EQ(responder.GetQName(), kNames);

    IPResponseAccumulator acc;
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = FindValidInterfaceId();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}
#endif // INET_CONFIG_ENABLE_IPV4

TEST_F(TestIPResponder, TestIPv6)
{
    IPAddress ipAddress;
    EXPECT_TRUE(IPAddress::FromString("fe80::224:32ff:aabb:ccdd", ipAddress));

    IPv6Responder responder(kNames);

    EXPECT_EQ(responder.GetQClass(), QClass::IN);
    EXPECT_EQ(responder.GetQType(), QType::AAAA);
    EXPECT_EQ(responder.GetQName(), kNames);

    IPResponseAccumulator acc;
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = FindValidInterfaceId();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}

} // namespace
