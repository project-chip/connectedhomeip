/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <controller/CHIPCommissionableNodeController.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

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
    void Shutdown() override {}
    void SetOperationalDelegate(OperationalResolveDelegate * delegate) override {}
    void SetCommissioningDelegate(CommissioningResolveDelegate * delegate) override {}
    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) override { return ResolveNodeIdStatus; }
    CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override { return DiscoverCommissionersStatus; }
    CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override
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

#if INET_CONFIG_ENABLE_IPV4
void TestGetDiscoveredCommissioner_HappyCase(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData inNodeData;
    strncpy(inNodeData.resolutionData.hostName, "mockHostName", sizeof(inNodeData.resolutionData.hostName));
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.resolutionData.ipAddress[0]);
    inNodeData.resolutionData.numIPs++;
    inNodeData.resolutionData.port = 5540;

    controller.OnNodeDiscovered(inNodeData);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0) != nullptr);
    NL_TEST_ASSERT(
        inSuite, strcmp(inNodeData.resolutionData.hostName, controller.GetDiscoveredCommissioner(0)->resolutionData.hostName) == 0);
    NL_TEST_ASSERT(inSuite,
                   inNodeData.resolutionData.ipAddress[0] == controller.GetDiscoveredCommissioner(0)->resolutionData.ipAddress[0]);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->resolutionData.port == 5540);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->resolutionData.numIPs == 1);
}

void TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData inNodeData;
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.resolutionData.ipAddress[0]);
    inNodeData.resolutionData.numIPs++;
    inNodeData.resolutionData.port = 5540;

    controller.OnNodeDiscovered(inNodeData);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(i) == nullptr);
    }
}

void TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Dnssd::DiscoveredNodeData invalidNodeData, validNodeData;
    // strncpy(inNodeData1.hostName, "mockHostName1", sizeof inNodeData1.hostName);
    Inet::IPAddress::FromString("192.168.1.10", invalidNodeData.resolutionData.ipAddress[0]);
    invalidNodeData.resolutionData.numIPs++;
    invalidNodeData.resolutionData.port = 5540;

    strncpy(validNodeData.resolutionData.hostName, "mockHostName2", sizeof validNodeData.resolutionData.hostName);
    Inet::IPAddress::FromString("192.168.1.11", validNodeData.resolutionData.ipAddress[0]);
    validNodeData.resolutionData.numIPs++;
    validNodeData.resolutionData.port = 5540;

    controller.OnNodeDiscovered(validNodeData);
    controller.OnNodeDiscovered(invalidNodeData);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0) != nullptr);
    NL_TEST_ASSERT(
        inSuite,
        strcmp(validNodeData.resolutionData.hostName, controller.GetDiscoveredCommissioner(0)->resolutionData.hostName) == 0);
    NL_TEST_ASSERT(
        inSuite, validNodeData.resolutionData.ipAddress[0] == controller.GetDiscoveredCommissioner(0)->resolutionData.ipAddress[0]);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->resolutionData.port == 5540);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->resolutionData.numIPs == 1);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(1) == nullptr);
}

#endif // INET_CONFIG_ENABLE_IPV4

void TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(i) == nullptr);
    }
}

void TestDiscoverCommissioners_HappyCase(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() == CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite,
                   controller.DiscoverCommissioners(Dnssd::DiscoveryFilter(Dnssd::DiscoveryFilterType::kDeviceType, 35)) ==
                       CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_InitError_ReturnsError(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    resolver.InitStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() != CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_DiscoverCommissionersError_ReturnsError(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    resolver.DiscoverCommissionersStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() != CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("TestGetDiscoveredCommissioner_HappyCase", TestGetDiscoveredCommissioner_HappyCase),
    NL_TEST_DEF("TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode", TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode),
    NL_TEST_DEF("TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr", TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr),
#endif // INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr", TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr),
    NL_TEST_DEF("TestDiscoverCommissioners_HappyCase", TestDiscoverCommissioners_HappyCase),
    NL_TEST_DEF("TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter", TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter),
    NL_TEST_DEF("TestDiscoverCommissioners_InitError_ReturnsError", TestDiscoverCommissioners_InitError_ReturnsError),
    NL_TEST_DEF("TestDiscoverCommissioners_DiscoverCommissionersError_ReturnsError", TestDiscoverCommissioners_DiscoverCommissionersError_ReturnsError),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestCommissionableNodeController_Setup(void * inContext)
{
    if (CHIP_NO_ERROR != chip::Platform::MemoryInit())
    {
        return FAILURE;
    }

    return SUCCESS;
}

int TestCommissionableNodeController_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestCommissionableNodeController()
{
    nlTestSuite theSuite = { "CommissionableNodeController", &sTests[0], TestCommissionableNodeController_Setup,
                             TestCommissionableNodeController_Teardown };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCommissionableNodeController)
