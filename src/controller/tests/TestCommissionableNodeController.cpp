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

#include <gtest/gtest.h>

#include <controller/CHIPCommissionableNodeController.h>
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
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::Platform::MemoryShutdown();
    }
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

} // namespace
