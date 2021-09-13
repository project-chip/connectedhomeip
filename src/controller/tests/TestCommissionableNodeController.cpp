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
using namespace chip::Mdns;
using namespace chip::Controller;

namespace chip {
namespace Mdns {
namespace {

class MockResolver : public Resolver
{
public:
    CHIP_ERROR SetResolverDelegate(ResolverDelegate *) override { return SetResolverDelegateStatus; }
    CHIP_ERROR StartResolver(chip::Inet::InetLayer * inetLayer, uint16_t port) override { return StartResolverStatus; }
    CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) override { return ResolveNodeIdStatus; }
    CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) override { return FindCommissionersStatus; }
    CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR SetResolverDelegateStatus = CHIP_NO_ERROR;
    CHIP_ERROR StartResolverStatus       = CHIP_NO_ERROR;
    CHIP_ERROR ResolveNodeIdStatus       = CHIP_NO_ERROR;
    CHIP_ERROR FindCommissionersStatus   = CHIP_NO_ERROR;
};

} // namespace
} // namespace Mdns
} // namespace chip

namespace {

void TestGetDiscoveredCommissioner_HappyCase(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Mdns::DiscoveredNodeData inNodeData;
    strncpy(inNodeData.hostName, "mockHostName", sizeof inNodeData.hostName);
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.ipAddress[0]);
    inNodeData.numIPs++;
    inNodeData.port = 5540;

    controller.OnNodeDiscoveryComplete(inNodeData);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0) != nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(inNodeData.hostName, controller.GetDiscoveredCommissioner(0)->hostName) == 0);
    NL_TEST_ASSERT(inSuite, inNodeData.ipAddress[0] == controller.GetDiscoveredCommissioner(0)->ipAddress[0]);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->port == 5540);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->numIPs == 1);
}

void TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(i) == nullptr);
    }
}

void TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Mdns::DiscoveredNodeData inNodeData;
    Inet::IPAddress::FromString("192.168.1.10", inNodeData.ipAddress[0]);
    inNodeData.numIPs++;
    inNodeData.port = 5540;

    controller.OnNodeDiscoveryComplete(inNodeData);

    for (int i = 0; i < CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES; i++)
    {
        NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(i) == nullptr);
    }
}

void TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    CommissionableNodeController controller(&resolver);
    chip::Mdns::DiscoveredNodeData invalidNodeData, validNodeData;
    // strncpy(inNodeData1.hostName, "mockHostName1", sizeof inNodeData1.hostName);
    Inet::IPAddress::FromString("192.168.1.10", invalidNodeData.ipAddress[0]);
    invalidNodeData.numIPs++;
    invalidNodeData.port = 5540;

    strncpy(validNodeData.hostName, "mockHostName2", sizeof validNodeData.hostName);
    Inet::IPAddress::FromString("192.168.1.11", validNodeData.ipAddress[0]);
    validNodeData.numIPs++;
    validNodeData.port = 5540;

    controller.OnNodeDiscoveryComplete(validNodeData);
    controller.OnNodeDiscoveryComplete(invalidNodeData);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0) != nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(validNodeData.hostName, controller.GetDiscoveredCommissioner(0)->hostName) == 0);
    NL_TEST_ASSERT(inSuite, validNodeData.ipAddress[0] == controller.GetDiscoveredCommissioner(0)->ipAddress[0]);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->port == 5540);
    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(0)->numIPs == 1);

    NL_TEST_ASSERT(inSuite, controller.GetDiscoveredCommissioner(1) == nullptr);
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
                   controller.DiscoverCommissioners(Mdns::DiscoveryFilter(Mdns::DiscoveryFilterType::kDeviceType, 35)) ==
                       CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_SetResolverDelegateError_ReturnsError(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    resolver.SetResolverDelegateStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() != CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_StartResolverError_ReturnsError(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    resolver.StartResolverStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() != CHIP_NO_ERROR);
}

void TestDiscoverCommissioners_FindCommissionersError_ReturnsError(nlTestSuite * inSuite, void * inContext)
{
    MockResolver resolver;
    resolver.FindCommissionersStatus = CHIP_ERROR_INTERNAL;
    CommissionableNodeController controller(&resolver);
    NL_TEST_ASSERT(inSuite, controller.DiscoverCommissioners() != CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestGetDiscoveredCommissioner_HappyCase", TestGetDiscoveredCommissioner_HappyCase),
    NL_TEST_DEF("TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr", TestGetDiscoveredCommissioner_NoNodesDiscovered_ReturnsNullptr),
    NL_TEST_DEF("TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr", TestGetDiscoveredCommissioner_InvalidNodeDiscovered_ReturnsNullptr),
    NL_TEST_DEF("TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode", TestGetDiscoveredCommissioner_HappyCase_OneValidOneInvalidNode),
    NL_TEST_DEF("TestDiscoverCommissioners_HappyCase", TestDiscoverCommissioners_HappyCase),
    NL_TEST_DEF("TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter", TestDiscoverCommissioners_HappyCaseWithDiscoveryFilter),
    NL_TEST_DEF("TestDiscoverCommissioners_SetResolverDelegateError_ReturnsError", TestDiscoverCommissioners_SetResolverDelegateError_ReturnsError),
    NL_TEST_DEF("TestDiscoverCommissioners_StartResolverError_ReturnsError", TestDiscoverCommissioners_StartResolverError_ReturnsError),
    NL_TEST_DEF("TestDiscoverCommissioners_FindCommissionersError_ReturnsError", TestDiscoverCommissioners_FindCommissionersError_ReturnsError),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestCommissionableNodeController()
{
    nlTestSuite theSuite = { "CommissionableNodeController", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCommissionableNodeController)
