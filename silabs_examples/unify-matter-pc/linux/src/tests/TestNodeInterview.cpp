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
#include "attribute.hpp"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "zap-types.h"

#include "mpc_attribute_parser_fwk.h"
#include "mpc_attribute_resolver.h"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_attribute_store.h"
#include "mpc_attribute_store_defined_attribute_types.h"

// Chip components
#include <lib/core/TLVDebug.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <filesystem>

#include <string>

using namespace unify::mpc::Test;
using namespace chip;
using namespace attribute_store;
using namespace std;

class TestContext : public UnifyMPCContext
{
public:
    nlTestSuite * mTestSuite;
    uint32_t mNumTimersHandled;
    attribute epNode;
    attribute mDevNode;

    static int initialize(void * inContext)
    {
        TestContext * ctxt = static_cast<TestContext *>(inContext);
        
        // Makes sure our required state folders exists
        std::filesystem::create_directories(LOCALSTATEDIR);

        if (CHIP_NO_ERROR != ctxt->Initialize())
            return FAILURE;

        attribute_store_init();
        mpc_attribute_store_init();
        ctxt->mDevNode = attribute::root().add_node(ATTRIBUTE_NODE_ID);
        attribute_store_set_reported_string(ctxt->mDevNode, "mt-01");
        ctxt->epNode = ctxt->mDevNode.add_node(ATTRIBUTE_ENDPOINT_ID).set_reported<EndpointId>(0);

        return SUCCESS;
    }

    static int finalize(void * inContext)
    {

        attribute_store_teardown();
        return TestContext::nlTestTearDownTestSuite(inContext);
    }
};

static void TestMPCInterviewFunctional(nlTestSuite * inSuite, void * aContext)
{
    TestContext * ctxt = static_cast<TestContext *>(aContext);

    auto state = ctxt->mDevNode.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS);
    state.set_reported<NodeStateNetworkStatus>(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING);
    mpc_attribute_resolver_helper_set_resolution_listener(ctxt->mDevNode);

    // setup mock serverList and partsList for ep0
    ctxt->epNode.emplace_node<string>(ATTRIBUTE_SERVERLIST_ID, "29,31,40,42");
    ctxt->epNode.emplace_node<string>(ATTRIBUTE_PARTSLIST_ID, "1");

    // setup mode ep1
    auto appEpNode = ctxt->mDevNode.emplace_node(ATTRIBUTE_ENDPOINT_ID, 1);

    // setup ep1 with onoff and its manditory attributes defined
    appEpNode.emplace_node<string>(ATTRIBUTE_SERVERLIST_ID, "6");
    appEpNode.emplace_node<string>(ONOFF_ATTRIBUTE_LIST, "0");
    appEpNode.emplace_node<bool>(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, false);

    mpc_attribute_resolver_resolution_completion(ctxt->mDevNode);

    NL_TEST_ASSERT(inSuite, (state.reported<NodeStateNetworkStatus>() == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL));
    appEpNode.delete_node();
}

static void TestMPCInterviewNonFunctional(nlTestSuite * inSuite, void * aContext)
{
    TestContext * ctxt = static_cast<TestContext *>(aContext);

    auto state = ctxt->mDevNode.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS);
    state.set_reported<NodeStateNetworkStatus>(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING);

    // setup mock serverList and partsList for ep0
    ctxt->epNode.emplace_node<string>(ATTRIBUTE_SERVERLIST_ID, "29,31,40,42");
    ctxt->epNode.emplace_node<string>(ATTRIBUTE_PARTSLIST_ID, "1");

    // setup mode ep1
    auto appEpNode = ctxt->mDevNode.emplace_node(ATTRIBUTE_ENDPOINT_ID, 1);

    // setup ep1 with onoff and its manditory attributes defined
    appEpNode.emplace_node<string>(ATTRIBUTE_SERVERLIST_ID, "6");
    appEpNode.emplace_node<string>(ONOFF_ATTRIBUTE_LIST, "0");
    appEpNode.emplace_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);

    mpc_attribute_resolver_resolution_completion(ctxt->mDevNode);

    NL_TEST_ASSERT(inSuite, (state.reported<NodeStateNetworkStatus>() == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL));
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestMPCInterviewFunctional",             TestMPCInterviewFunctional),
    NL_TEST_DEF("TestMPCInterviewNonFunctional",             TestMPCInterviewNonFunctional),

    NL_TEST_SENTINEL()
};


// clang-format off
static nlTestSuite kTheSuite =
{
    "TestNodeInterview",
    &sTests[0],
    TestContext::initialize,
    TestContext::finalize,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,

};

int TestNodeInterview(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestNodeInterview)
