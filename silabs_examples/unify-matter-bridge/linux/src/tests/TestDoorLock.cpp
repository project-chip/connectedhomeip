#include "ClusterTestContext.h"
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
using TestContext = Test::ClusterContext<DoorLockAttributeAccess, DoorLockClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
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

static void TestDoorLockAttributeLockState(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Locked" })",
        MakeNullable(DlLockState::kLocked));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);   
}

static void TestDoorLockAttributeLockType(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::DoorLock::Attributes::LockType::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockType/Reported", R"({ "value": "DeadBolt" })",
        DlLockType::kDeadBolt);
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
        "NoRFLockOrUnlockModeSupported": false, "NormalModeSupported": true, "PassageModeSupported": false,
        "PrivacyModeSupported": false, "VacationModeSupported": false }})", 1);
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

static void TestDoorLockCommandLockDoor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::DoorLock::Commands::LockDoor::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::DoorLock::Commands::LockDoor::Type>(sSuite,
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/LockDoor", R"({ "PINOrRFIDCode": "" })", request, 1000);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestDoorLockCommandUnlockDoor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::DoorLock::Commands::UnlockDoor::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::DoorLock::Commands::UnlockDoor::Type>(sSuite,
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/UnlockDoor", R"({ "PINOrRFIDCode": "" })", request, 1000);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeFeatureMap", TestDoorLockAttributeFeatureMap),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeClusterRevision", TestDoorLockAttributeClusterRevision),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeLockState", TestDoorLockAttributeLockState),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeLockType", TestDoorLockAttributeLockType),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeActuatorEnabled", TestDoorLockAttributeActuatorEnabled),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeOperatingMode", TestDoorLockAttributeOperatingMode),
    NL_TEST_DEF("DoorLock::TestDoorLockAttributeSupportedOperatingModes", TestDoorLockAttributeSupportedOperatingModes),
    NL_TEST_DEF("DoorLock::TestDoorLockCommandLockDoor", TestDoorLockCommandLockDoor),
    NL_TEST_DEF("DoorLock::TestDoorLockCommandUnlockDoor", TestDoorLockCommandUnlockDoor),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "DoorLockTests", &sTests[0], Initialize, TestContext::Finalize };

int TestDoorLockSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDoorLockSuite)
