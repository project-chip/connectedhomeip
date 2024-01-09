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
using namespace chip::app::Clusters::Thermostat;

using TestContext = Test::ClusterContext<ThermostatAttributeAccess, ThermostatClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("Thermostat");
    cluster.attributes = {
        "LocalTemperature",
        "OccupiedCoolingSetpoint",
        "OccupiedHeatingSetpoint",
        "MinHeatSetpointLimit",
        "MaxHeatSetpointLimit",
        "MinCoolSetpointLimit",
        "MaxCoolSetpointLimit",
        "ControlSequenceOfOperation",
        "SystemMode",
        "FeatureMap",
    };
    cluster.supported_commands = {
        "SetpointRaiseOrLower",   
    };
    return ctx->register_endpoint(ep);
}

static void TestThermostatAttributeLocalTemprature(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::LocalTemperature::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/LocalTemperature/Reported", R"({ "value": 2000 })", 
        chip::app::DataModel::Nullable<int16_t>(2000));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeOccupiedCoolingSetpoint(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported", R"({ "value": 2000 })", 2000);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeOccupiedHeatingSetpoint(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported", R"({ "value": 2200 })", 2200);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeMinHeatSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::MinHeatSetpointLimit::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MinHeatSetpointLimit/Reported", R"({ "value": 1500 })", 1500);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeMaxHeatSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::MaxHeatSetpointLimit::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MaxHeatSetpointLimit/Reported", R"({ "value": 3500 })", 3500);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeMinCoolSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::MinCoolSetpointLimit::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MinCoolSetpointLimit/Reported", R"({ "value": 1000 })", 1000);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeMaxCoolSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::MaxCoolSetpointLimit::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MaxCoolSetpointLimit/Reported", R"({ "value": 3000 })", 3000);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestThermostatAttributeControlSequenceOfOperation(nlTestSuite * sSuite, void * apContext)
{
    auto & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err     = ctx.attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "CoolingOnly"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kCoolingOnly);
    CHIP_ERROR err1    = ctx.attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "CoolingWithReheat"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kCoolingWithReheat);
    CHIP_ERROR err2    = ctx.attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "HeatingOnly"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kHeatingOnly);
    CHIP_ERROR err3    = ctx.attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "HeatingWithReheat"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kHeatingWithReheat);
    NL_TEST_ASSERT(sSuite, ((err == CHIP_NO_ERROR) && (err1 == CHIP_NO_ERROR) && (err2 == CHIP_NO_ERROR) && 
                            (err3 == CHIP_NO_ERROR)));
}

static void TestThermostatAttributeSystemMode(nlTestSuite * sSuite, void * apContext)
{
    auto & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err1     = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Off"})", SystemModeEnum::kOff);
    CHIP_ERROR err2    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Auto"})", SystemModeEnum::kAuto);
    CHIP_ERROR err3    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Cool"})", SystemModeEnum::kCool);
    CHIP_ERROR err4    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Heat"})", SystemModeEnum::kHeat);
    CHIP_ERROR err5    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "EmergencyHeating"})", SystemModeEnum::kEmergencyHeat);
    CHIP_ERROR err6    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Precooling"})", SystemModeEnum::kPrecooling);    
    CHIP_ERROR err7    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "FanOnly"})", SystemModeEnum::kFanOnly);
    CHIP_ERROR err8    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Dry"})", SystemModeEnum::kDry);    
    CHIP_ERROR err9    = ctx.attribute_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Sleep"})", SystemModeEnum::kSleep);
    NL_TEST_ASSERT(sSuite, ((err1 == CHIP_NO_ERROR) && (err2 == CHIP_NO_ERROR) && (err3 == CHIP_NO_ERROR) && (err4 == CHIP_NO_ERROR) && (err5 == CHIP_NO_ERROR) && 
                            (err6 == CHIP_NO_ERROR) && (err7 == CHIP_NO_ERROR) && (err8 == CHIP_NO_ERROR) && (err9 == CHIP_NO_ERROR)));
}

static void TestThermostatCommandSetpointRaiseOrLower(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Commands::SetpointRaiseLower::Type request;
    CHIP_ERROR err = ctx.command_test<Commands::SetpointRaiseLower::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Heat","Amount":0})", request);
    NL_TEST_ASSERT(sSuite, (err == CHIP_NO_ERROR));
}

static void TestThermostatWriteAttributeOccupiedCoolingSetpoint(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"OccupiedCoolingSetpoint": 2000})", 2000);
}

static void TestThermostatWriteAttributeOccupiedHeatingSetpoint(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"OccupiedHeatingSetpoint": 2200})", 2200);
}

static void TestThermostatWriteAttributeMinHeatSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::MinHeatSetpointLimit::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"MinHeatSetpointLimit": 1000})", 1000);
}

static void TestThermostatWriteAttributeMaxHeatSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::MaxHeatSetpointLimit::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"MaxHeatSetpointLimit": 3000})", 3000);
}

static void TestThermostatWriteAttributeMinCoolSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::MinCoolSetpointLimit::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"MinCoolSetpointLimit": 1500})", 1500);
}

static void TestThermostatWriteAttributeMaxCoolSetpointLimit(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::MaxCoolSetpointLimit::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"MaxCoolSetpointLimit": 2500})", 2500);
}

static void TestThermostatWriteAttributeControlSequenceOfOperation(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"ControlSequenceOfOperation": "CoolingOnly"})",
        ControlSequenceOfOperationEnum::kCoolingOnly);
}

static void TestThermostatWriteAttributeSystemMode(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::Thermostat;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::SystemMode::TypeInfo>(
        sSuite, "Thermostat/Commands/WriteAttributes", R"({"SystemMode":"Auto"})", SystemModeEnum::kAuto);
}

static void TestThermostatAttributeFeatureMap(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::FeatureMap::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/FeatureMap/Reported", R"({ "value": 3 })", 3);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Thermostat::TestThermostatAttributeLocalTemprature", TestThermostatAttributeLocalTemprature),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeOccupiedCoolingSetpoint", TestThermostatAttributeOccupiedCoolingSetpoint),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeOccupiedHeatingSetpoint", TestThermostatAttributeOccupiedHeatingSetpoint),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeMinHeatSetpointLimit", TestThermostatAttributeMinHeatSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeMaxHeatSetpointLimit", TestThermostatAttributeMaxHeatSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeMinCoolSetpointLimit", TestThermostatAttributeMinCoolSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeMaxCoolSetpointLimit", TestThermostatAttributeMaxCoolSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeControlSequenceOfOperation", TestThermostatAttributeControlSequenceOfOperation),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeSystemMode", TestThermostatAttributeSystemMode),
    NL_TEST_DEF("Thermostat::TestThermostatCommandSetpointRaiseOrLower", TestThermostatCommandSetpointRaiseOrLower),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeOccupiedCoolingSetpoint", TestThermostatWriteAttributeOccupiedCoolingSetpoint),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeOccupiedHeatingSetpoint", TestThermostatWriteAttributeOccupiedHeatingSetpoint),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeMinHeatSetpointLimit", TestThermostatWriteAttributeMinHeatSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeMaxHeatSetpointLimit", TestThermostatWriteAttributeMaxHeatSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeMinCoolSetpointLimit", TestThermostatWriteAttributeMinCoolSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeMaxCoolSetpointLimit", TestThermostatWriteAttributeMaxCoolSetpointLimit),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeControlSequenceOfOperation", TestThermostatWriteAttributeControlSequenceOfOperation),
    NL_TEST_DEF("Thermostat::TestThermostatWriteAttributeSystemMode", TestThermostatWriteAttributeSystemMode),
    NL_TEST_DEF("Thermostat::TestThermostatAttributeFeatureMap", TestThermostatAttributeFeatureMap),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "ThermostatTests", &sTests[0], Initialize, TestContext::Finalize };

int TestThermostatSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestThermostatSuite)