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
#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include "command_translator_interface.hpp"
#include "matter.h"

// Mocks
#include "MockClusterCommandHandler.hpp"
#include "MockEventInteractionModel.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>


using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;
using TestContext = unify::matter_bridge::Test::ClusterContext<DoorLockAttributeAccess,
                                unify::matter_bridge::Test::MockClusterCommandHandler>;
chip::EndpointId kTestEndpointId   = 2;
    
static int Initialize(void * context)
{
    if (TestContext::Initialize(context, true) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("DoorLock");
    cluster.attributes = {
        "LockState",
        "LockType",
        "ActuatorEnabled",
        "OperatingMode",
        "SupportedOperatingModes", 
        "DoorState",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "LockDoor", "UnlockDoor",
    };
   
    return ctx->register_endpoint(ep);
}

static int Finalize(void * context)
{
    if (TestContext::Finalize(context, true) != SUCCESS)
    {
        return FAILURE;
    }
        
    return SUCCESS;
}

static void TestDoorLockAttributeLockState(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Unlocked" })",
        MakeNullable(DlLockState::kUnlocked));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);   
}

static void TestDoorLockAttributeLockType(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::LockType::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockType/Reported", R"({ "value": "Magnetic" })",
        DlLockType::kMagnetic);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestDoorLockAttributeActuatorEnabled(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::ActuatorEnabled::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/ActuatorEnabled/Reported", R"({ "value": true })",
        true);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestDoorLockAttributeOperatingMode(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::OperatingMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/OperatingMode/Reported", R"({ "value": "Vacation" })",
        OperatingModeEnum::kVacation);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);   
}

static void TestDoorLockAttributeSupportedOperatingModes(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::SupportedOperatingModes::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/SupportedOperatingModes/Reported", R"({ "value": { 
        "NoRFLockOrUnlockModeSupported": false, "NormalModeSupported": false, "PassageModeSupported": true,
        "PrivacyModeSupported": true, "VacationModeSupported": true }})", 22);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestDoorLockAttributeFeatureMap(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::FeatureMap::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/FeatureMap/Reported", R"({ "value": 0 })",
        0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);    
}

static void TestDoorLockAttributeClusterRevision(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::ClusterRevision::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/ClusterRevision/Reported", R"({ "value": 7 })",
        7);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestDoorLockEventErrorJammed(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    Clusters::DoorLock::Events::DoorLockAlarm::DecodableType eventdata;
        
    // Trigger Doorstate event.
    err = ctx.attribute_test<Clusters::DoorLock::Attributes::DoorState::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/DoorState/Reported", R"({ "value": "ErrorJammed" })",
        MakeNullable(DoorStateEnum::kDoorJammed));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    
    err = ctx.event_test<Clusters::DoorLock::Events::DoorLockAlarm::DecodableType>(sSuite, 1, eventdata);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(sSuite, eventdata.alarmCode == Clusters::DoorLock::AlarmCodeEnum::kLockJammed);
    ctx.DrainAndServiceIO();
}

static void TestDoorLockEventLocked(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    Clusters::DoorLock::Events::LockOperation::DecodableType eventdata;

    // Trigger LockOperation Locked event
    err = ctx.attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Locked" })",
        MakeNullable(DlLockState::kLocked));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR); 

    err = ctx.event_test<Clusters::DoorLock::Events::LockOperation::DecodableType>(sSuite, 2, eventdata);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(sSuite, eventdata.lockOperationType == Clusters::DoorLock::LockOperationTypeEnum::kLock);
    ctx.DrainAndServiceIO();
}

static void TestDoorLockEventUnlocked(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    Clusters::DoorLock::Events::LockOperation::DecodableType eventdata;

    // Trigger LockOperation Unlocked event
    err = ctx.attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Unlocked" })",
        MakeNullable(DlLockState::kUnlocked));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR); 
    
    err = ctx.event_test<Clusters::DoorLock::Events::LockOperation::DecodableType>(sSuite, 3, eventdata);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(sSuite, eventdata.lockOperationType == Clusters::DoorLock::LockOperationTypeEnum::kUnlock);
    ctx.DrainAndServiceIO();
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeLockState", TestDoorLockAttributeLockState),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeLockType", TestDoorLockAttributeLockType),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeActuatorEnabled", TestDoorLockAttributeActuatorEnabled),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeOperatingMode", TestDoorLockAttributeOperatingMode),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeSupportedOperatingModes", TestDoorLockAttributeSupportedOperatingModes),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeFeatureMap",TestDoorLockAttributeFeatureMap),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeClusterRevision",TestDoorLockAttributeClusterRevision),
    NL_TEST_DEF("DoorLock::TestDoorLockEventErrorJammed",TestDoorLockEventErrorJammed),
    NL_TEST_DEF("DoorLock::TestDoorLockEventLocked",TestDoorLockEventLocked),
    NL_TEST_DEF("DoorLock::TestDoorLockEventUnlocked",TestDoorLockEventUnlocked),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "DoorLockTests", &sTests[0], Initialize, Finalize };

int TestDoorLockSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDoorLockSuite)
