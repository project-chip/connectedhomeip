/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "UnifyMPCContext.h"

// matter includes
#include <app/MessageDef/ReportDataMessage.h>
#include <app/ReadClient.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

// unify includes
#include "attribute.hpp"
#include "attribute_store_fixt.h"
#include "mpc_attribute_resolver.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_node_monitor.h"
#include "mpc_sessionprovider_mock.h"

#include <filesystem>

#include <string>

using namespace unify::mpc::Test;
using namespace chip;
using namespace chip::app;
using namespace attribute_store;

class TestContext : public UnifyMPCContext
{
public:
    static int Init(void * ctxt)
    {
        // Makes sure our required state folders exists
        std::filesystem::create_directories(LOCALSTATEDIR);
        int ret = TestContext::Initialize(ctxt);
        
        if (ret != SUCCESS)
            return ret;
        if (SL_STATUS_OK != attribute_store_init())
            return FAILURE;
        return SUCCESS;
    }
};

static void TestNodeMonitorInit(nlTestSuite * inSuite, void * aContext)
{
    NL_TEST_ASSERT(inSuite, (SL_STATUS_OK == mpc_node_monitor_init()));
}

static void TestNodeMonitorSetupReport(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    TestSessionProvider testSession(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice(), false);
    auto topNode        = attribute::root().emplace_node<std::string>(ATTRIBUTE_NODE_ID, "mt-alice");
    auto destNodeId     = ctx.GetAliceFabric()->GetNodeId();
    auto destFabric     = ctx.GetAliceFabric()->GetCompressedFabricId();
    std::string nwkList = std::to_string(destFabric) + ":" + std::to_string(destNodeId);
    attribute_store_set_child_reported(topNode, DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST, nwkList.c_str(), nwkList.length());

    mpc_attribute_resolver_helper_set_resolution_listener(topNode);
    auto epNode = topNode.emplace_node<EndpointId>(ATTRIBUTE_ENDPOINT_ID, 0);
    attribute_store_type_t reportNodeType =
        ((Clusters::OnOff::Id & 0xFFFF) << 16) | (Clusters::OnOff::Attributes::OnOff::Id && 0xFFFF);
    // auto reportNode =
    epNode.emplace_node<bool>(reportNodeType, false);

    mpc_node_monitor_initiate_monitoring(topNode);

    NL_TEST_ASSERT(inSuite, (chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 1));

    ctx.GetLoopback().mNumMessagesToDrop = 1;
    ctx.DrainAndServiceIO();
    topNode.delete_node();
    chip::app::InteractionModelEngine::GetInstance()->ShutdownActiveReads();
}

static void TestNodeMonitorSetupReportFail(nlTestSuite * inSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);
    TestSessionProvider testSession(ctx.GetExchangeManager(), ctx.GetSessionBobToAlice(), true);
    auto topNode        = attribute::root().emplace_node<std::string>(ATTRIBUTE_NODE_ID, "mt-alice");
    auto destNodeId     = ctx.GetAliceFabric()->GetNodeId();
    auto destFabric     = ctx.GetAliceFabric()->GetCompressedFabricId();
    std::string nwkList = std::to_string(destFabric) + ":" + std::to_string(destNodeId);
    attribute_store_set_child_reported(topNode, DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST, nwkList.c_str(), nwkList.length());

    mpc_attribute_resolver_helper_set_resolution_listener(topNode);
    auto epNode = topNode.emplace_node<EndpointId>(ATTRIBUTE_ENDPOINT_ID, 0);
    attribute_store_type_t reportNodeType =
        ((Clusters::OnOff::Id & 0xFFFF) << 16) | (Clusters::OnOff::Attributes::OnOff::Id && 0xFFFF);
    // auto reportNode =
    epNode.emplace_node<bool>(reportNodeType, false);

    mpc_node_monitor_initiate_monitoring(topNode);

    NL_TEST_ASSERT(inSuite, (chip::app::InteractionModelEngine::GetInstance()->GetNumActiveReadClients() == 0));
}
/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestNodeMonitorInit", TestNodeMonitorInit),
    NL_TEST_DEF("TestNodeMonitorSetupReport", TestNodeMonitorSetupReport),
    NL_TEST_DEF("TestNodeMonitorSetupReportFail", TestNodeMonitorSetupReportFail),

    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestNodeMonitorInterface",
    &sTests[0],
    TestContext::Init,
    TestContext::Finalize
};

int TestNodeMonitorInterface(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestNodeMonitorInterface)
