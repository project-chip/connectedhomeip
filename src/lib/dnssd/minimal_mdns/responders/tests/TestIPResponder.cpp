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

#include <lib/dnssd/minimal_mdns/AddressPolicy_DefaultImpl.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace mdns::Minimal;

constexpr uint16_t kMdnsPort = 5353;

const QNamePart kNames[] = { "some", "test", "local" };

class IPResponseAccumulator : public ResponderDelegate
{
public:
    IPResponseAccumulator(nlTestSuite * suite) : mSuite(suite) {}
    void AddResponse(const ResourceRecord & record) override
    {

        NL_TEST_ASSERT(mSuite, (record.GetType() == QType::A) || (record.GetType() == QType::AAAA));
        NL_TEST_ASSERT(mSuite, record.GetClass() == QClass::IN);
        NL_TEST_ASSERT(mSuite, record.GetName() == kNames);
    }

private:
    nlTestSuite * mSuite;
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

#if INET_CONFIG_ENABLE_IPV4
void TestIPv4(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("10.20.30.40", ipAddress));

    IPv4Responder responder(kNames);

    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::A);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kNames);

    IPResponseAccumulator acc(inSuite);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = FindValidInterfaceId();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}
#endif // INET_CONFIG_ENABLE_IPV4

void TestIPv6(nlTestSuite * inSuite, void * inContext)
{
    IPAddress ipAddress;
    NL_TEST_ASSERT(inSuite, IPAddress::FromString("fe80::224:32ff:aabb:ccdd", ipAddress));

    IPv6Responder responder(kNames);

    NL_TEST_ASSERT(inSuite, responder.GetQClass() == QClass::IN);
    NL_TEST_ASSERT(inSuite, responder.GetQType() == QType::AAAA);
    NL_TEST_ASSERT(inSuite, responder.GetQName() == kNames);

    IPResponseAccumulator acc(inSuite);
    chip::Inet::IPPacketInfo packetInfo;

    packetInfo.SrcAddress  = ipAddress;
    packetInfo.DestAddress = ipAddress;
    packetInfo.SrcPort     = kMdnsPort;
    packetInfo.DestPort    = kMdnsPort;
    packetInfo.Interface   = FindValidInterfaceId();

    responder.AddAllResponses(&packetInfo, &acc, ResponseConfiguration());
}

int Setup(void * inContext)
{
    mdns::Minimal::SetDefaultAddressPolicy();

    CHIP_ERROR error = chip::Platform::MemoryInit();

    return (error == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

int Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

const nlTest sTests[] = {
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("TestIPv4", TestIPv4), //
#endif                                 // INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("TestIPv6", TestIPv6), //
    NL_TEST_SENTINEL()                 //
};

} // namespace

int TestIP(void)
{
    nlTestSuite theSuite = { "IP", sTests, &Setup, &Teardown };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestIP)
