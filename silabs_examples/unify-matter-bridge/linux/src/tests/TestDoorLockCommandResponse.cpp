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

#include "ClusterTestContext.h"
#include <app/CommandHandler.h>
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;
using TestContext = unify::matter_bridge::Test::ClusterContext<DoorLockAttributeAccess,DoorLockClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("DoorLock");
    cluster.attributes = {
        "GeneratedCommandList",
        "AcceptedCommandList",
    };
    cluster.supported_commands = {
        "LockDoor", "UnlockDoor",
    };

    return ctx->register_endpoint(ep);
}

static void TestDoorLockCommandLockDoor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::DoorLock::Commands::LockDoor::Type request;

    CHIP_ERROR err = ctx.command_test<Clusters::DoorLock::Commands::LockDoor::Type>(sSuite,
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/LockDoor", R"({ "PINOrRFIDCode": "" })", request, 50000);

    ctx.mqtt_subscribeCb("ucl/by-unid/zw-0x0002/ep2/DoorLock/GeneratedCommands/LockDoorResponse", R"({ "Status": "1" })");

    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();
}

static void TestDoorLockCommandUnlockDoor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::DoorLock::Commands::UnlockDoor::Type request;
        
    CHIP_ERROR err = ctx.command_test<Clusters::DoorLock::Commands::UnlockDoor::Type>(sSuite,
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/UnlockDoor", R"({ "PINOrRFIDCode": "" })", request, 50000);

    ctx.mqtt_subscribeCb("ucl/by-unid/zw-0x0002/ep2/DoorLock/GeneratedCommands/UnlockDoorResponse", R"({ "Status": "1" })");
    
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();
} 

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("DoorLock::TestDoorLockCommandLockDoor", TestDoorLockCommandLockDoor),
    NL_TEST_DEF("DoorLock::TestDoorLockCommandUnlockDoor", TestDoorLockCommandUnlockDoor),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "DoorLockcmdTests", &sTests[0], Initialize, TestContext::Finalize };

int TestDoorLockcmdSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDoorLockcmdSuite)
