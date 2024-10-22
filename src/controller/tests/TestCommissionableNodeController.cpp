/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/CHIPCommissionableNodeController.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>

using namespace chip;
using namespace chip::Dnssd;
using namespace chip::Controller;

namespace chip {
namespace Dnssd {
namespace {

class MockResolver : public Resolver
{
public:
    CHIP_ERROR Init(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager) override { return InitStatus; }
    bool IsInitialized() override { return true; }
    void Shutdown() override {}
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override {}
    CHIP_ERROR ResolveNodeId(const PeerId & peerId) override { return ResolveNodeIdStatus; }
    void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) override {}
    CHIP_ERROR StartDiscovery(DiscoveryType type, DiscoveryFilter filter, DiscoveryContext &) override
    {
        if (DiscoveryType::kCommissionerNode == type)
            return DiscoverCommissionersStatus;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR StopDiscovery(DiscoveryContext &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR InitStatus                  = CHIP_NO_ERROR;
    CHIP_ERROR ResolveNodeIdStatus         = CHIP_NO_ERROR;
    CHIP_ERROR DiscoverCommissionersStatus = CHIP_NO_ERROR;
};

} // namespace
} // namespace Dnssd
} // namespace chip

namespace {

class TestCommissionableNodeController : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

#if INET_CONFIG_ENABLE_IPV4
TEST_F(TestCommissionableNodeController, TestGetDiscoveredCommissioner_HappyCase)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData discNodeData;
    discNodeData.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData = discNodeData.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData.hostName, "mockHostName");
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.ipAddress[0]);
    inNodeData.numIPs++;
    inNodeData.port = 5540;

    controller.OnNodeDiscovered(discNodeData);

    ASSERT_NE(controller.GetDiscoveredCommissioner(0), nullptr);
    EXPECT_STREQ(inNodeData.hostName, controller.GetDiscoveredCommissioner(0)->hostName);
    EXPECT_EQ(inNodeData.ipAddress[0], controller.GetDiscoveredCommissioner(0)->ipAddress[0]);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->port, 5540);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->numIPs, 1u);
}

TEST_F(TestCommissionableNodeController, TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData discNodeData;
    discNodeData.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData = discNodeData.Get<chip::Dnssd::CommissionNodeData>();
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.ipAddress[0]);
    inNodeData.numIPs++;
    inNodeData.port = 5540;

    controller.OnNodeDiscovered(discNodeData);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        EXPECT_EQ(controller.GetDiscoveredCommissioner(i), nullptr);
    }
}

TEST_F(TestCommissionableNodeController, TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData invalidDiscNodeData, validDiscNodeData;
    invalidDiscNodeData.Set<chip::Dnssd::CommissionNodeData>();
    validDiscNodeData.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & invalidNodeData = invalidDiscNodeData.Get<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & validNodeData   = validDiscNodeData.Get<chip::Dnssd::CommissionNodeData>();
    Inet::IPAddress::FromString("192.168.1.10", invalidNodeData.ipAddress[0]);
    invalidNodeData.numIPs++;
    invalidNodeData.port = 5540;

    Platform::CopyString(validNodeData.hostName, "mockHostName2");
    Inet::IPAddress::FromString("192.168.1.11", validNodeData.ipAddress[0]);
    validNodeData.numIPs++;
    validNodeData.port = 5540;

    controller.OnNodeDiscovered(validDiscNodeData);
    controller.OnNodeDiscovered(invalidDiscNodeData);

    ASSERT_NE(controller.GetDiscoveredCommissioner(0), nullptr);
    EXPECT_STREQ(validNodeData.hostName, controller.GetDiscoveredCommissioner(0)->hostName);
    EXPECT_EQ(validNodeData.ipAddress[0], controller.GetDiscoveredCommissioner(0)->ipAddress[0]);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->port, 5540);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->numIPs, 1u);

    EXPECT_EQ(controller.GetDiscoveredCommissioner(1), nullptr);
}

#endif // INET_CONFIG_ENABLE_IPV4

TEST_F(TestCommissionableNodeController, TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        EXPECT_EQ(controller.GetDiscoveredCommissioner(i), nullptr);
    }
}

TEST_F(TestCommissionableNodeController, TestDiscoverCommissioners_HappyCase)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    EXPECT_EQ(controller.DiscoverCommissioners(), CHIP_NO_ERROR);
}

TEST_F(TestCommissionableNodeController, TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    EXPECT_EQ(controller.DiscoverCommissioners(Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, 35)), CHIP_NO_ERROR);
}

TEST_F(TestCommissionableNodeController, TestDiscoverCommissioners_InitError_ReturnsError)
{
    MockResolver resolver;
    resolver.InitStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    EXPECT_NE(controller.DiscoverCommissioners(), CHIP_NO_ERROR);
}

TEST_F(TestCommissionableNodeController, TestDiscoverCommissioners_DiscoverCommissionersError_ReturnsError)
{
    MockResolver resolver;
    resolver.DiscoverCommissionersStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    EXPECT_NE(controller.DiscoverCommissioners(), CHIP_NO_ERROR);
}

TEST_F(TestCommissionableNodeController, TestGetDiscoveredCommissioner_MultipleIPAddressDiscover)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);

    // example 1
    chip::Dnssd::DiscoveredNodeData discNodeData1;
    discNodeData1.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData1 = discNodeData1.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData1.hostName, "mockHostName");
    Inet::IPAddress::FromString("fd11:1111:1122:2222:1111:2222:3333:4444", inNodeData1.ipAddress[0]);
    inNodeData1.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:5555", inNodeData1.ipAddress[1]);
    inNodeData1.numIPs++;
    inNodeData1.port = 5540;

    controller.OnNodeDiscovered(discNodeData1);

    // example 5 - exactly same as example 1
    chip::Dnssd::DiscoveredNodeData discNodeData5;
    discNodeData5.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData5 = discNodeData5.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData1.hostName, "mockHostName");
    Inet::IPAddress::FromString("fd11:1111:1122:2222:1111:2222:3333:4444", inNodeData5.ipAddress[0]);
    inNodeData5.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:5555", inNodeData5.ipAddress[1]);
    inNodeData5.numIPs++;
    inNodeData5.port = 5540;

    controller.OnNodeDiscovered(discNodeData5);

    // example 2 - same as example 1 (IPAdress sequence is only different.)
    chip::Dnssd::DiscoveredNodeData discNodeData2;
    discNodeData2.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData2 = discNodeData2.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData2.hostName, "mockHostName");
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:5555", inNodeData2.ipAddress[0]);
    inNodeData2.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:1111:2222:3333:4444", inNodeData2.ipAddress[1]);
    inNodeData2.numIPs++;
    inNodeData2.port = 5540;

    controller.OnNodeDiscovered(discNodeData2);

    // example 3 - different example
    chip::Dnssd::DiscoveredNodeData discNodeData3;
    discNodeData3.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData3 = discNodeData3.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData3.hostName, "mockHostName");
    Inet::IPAddress::FromString("fd11:1111:1122:2222:1111:2222:3333:4444", inNodeData3.ipAddress[0]);
    inNodeData3.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:6666", inNodeData3.ipAddress[1]);
    inNodeData3.numIPs++;
    inNodeData3.port = 5540;

    controller.OnNodeDiscovered(discNodeData3);

    // example 4 - different example (Different IP count)
    chip::Dnssd::DiscoveredNodeData discNodeData4;
    discNodeData4.Set<chip::Dnssd::CommissionNodeData>();
    chip::Dnssd::CommissionNodeData & inNodeData4 = discNodeData4.Get<chip::Dnssd::CommissionNodeData>();
    Platform::CopyString(inNodeData4.hostName, "mockHostName");
    Inet::IPAddress::FromString("fd11:1111:1122:2222:1111:2222:3333:4444", inNodeData4.ipAddress[0]);
    inNodeData4.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:6666", inNodeData4.ipAddress[1]);
    inNodeData4.numIPs++;
    Inet::IPAddress::FromString("fd11:1111:1122:2222:2222:3333:4444:7777", inNodeData4.ipAddress[2]);
    inNodeData4.numIPs++;
    inNodeData4.port = 5540;

    controller.OnNodeDiscovered(discNodeData4);

    // Example 2 result - example 1 is removed. (reason : duplicate)
    ASSERT_NE(controller.GetDiscoveredCommissioner(0), nullptr);
    EXPECT_STREQ(inNodeData1.hostName, controller.GetDiscoveredCommissioner(0)->hostName);
    EXPECT_EQ(inNodeData2.ipAddress[0], controller.GetDiscoveredCommissioner(0)->ipAddress[0]);
    EXPECT_EQ(inNodeData2.ipAddress[1], controller.GetDiscoveredCommissioner(0)->ipAddress[1]);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->port, 5540);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(0)->numIPs, 2u);

    // Example 3 result
    ASSERT_NE(controller.GetDiscoveredCommissioner(1), nullptr);
    EXPECT_STREQ(inNodeData3.hostName, controller.GetDiscoveredCommissioner(1)->hostName);
    EXPECT_EQ(inNodeData3.ipAddress[0], controller.GetDiscoveredCommissioner(1)->ipAddress[0]);
    EXPECT_EQ(inNodeData3.ipAddress[1], controller.GetDiscoveredCommissioner(1)->ipAddress[1]);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(1)->port, 5540);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(1)->numIPs, 2u);

    // Example 4 result
    ASSERT_NE(controller.GetDiscoveredCommissioner(2), nullptr);
    EXPECT_STREQ(inNodeData4.hostName, controller.GetDiscoveredCommissioner(2)->hostName);
    EXPECT_EQ(inNodeData4.ipAddress[0], controller.GetDiscoveredCommissioner(2)->ipAddress[0]);
    EXPECT_EQ(inNodeData4.ipAddress[1], controller.GetDiscoveredCommissioner(2)->ipAddress[1]);
    EXPECT_EQ(inNodeData4.ipAddress[2], controller.GetDiscoveredCommissioner(2)->ipAddress[2]);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(2)->port, 5540);
    EXPECT_EQ(controller.GetDiscoveredCommissioner(2)->numIPs, 3u);

    // Total is 3. (Not 4)
    ASSERT_EQ(controller.GetDiscoveredCommissioner(3), nullptr);
}

} // namespace
