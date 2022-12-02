#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "feature_map_attr_override.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
// Third party library
#include <nlunit-test.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using TestContext = Test::ClusterContext<LevelControlAttributeAccessOverride, LevelControlClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("Level");
    cluster.attributes = {
        "CurrentLevel",
        "RemainingTime",
        "MinLevel",
        "MaxLevel",
        "CurrentFrequency",
        "MinFrequency",
        "MaxFrequency",
        "Options",
        "OnOffTransitionTime",
        "OnLevel",
        "OnTransitionTime",
        "OffTransitionTime",
        "DefaultMoveRate",
        "StartUpCurrentLevel",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "MoveToLevel",           "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff",
        "MoveToClosestFrequency" // UNSUPPORTED_COMMAND
    };

    return ctx->register_endpoint(ep);
}

static void TestLevelControlAttributeWrite(nlTestSuite * sSuite, void * apContext)
{
    using namespace chip::app::Clusters::LevelControl;
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    ctx.attribute_write_test<Attributes::OnLevel::TypeInfo>(sSuite,"Level/Attributes/OnLevel",R"({ "value": 3 })",Attributes::OnLevel::TypeInfo::Type(3));
}


static void TestLevelControlAttributeRemainingTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::RemainingTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/RemainingTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeMinLevel(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::MinLevel::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MinLevel/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeMaxLevel(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::MaxLevel::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MaxLevel/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeCurrentFrequency(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::CurrentFrequency::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/CurrentFrequency/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeMinFrequency(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::MinFrequency::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MinFrequency/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeMaxFrequency(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::MaxFrequency::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MaxFrequency/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeOptions(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    // Bitmask
    err = ctx.attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported",
        R"({ "value": {"ExecuteIfOff":false,"CoupleColorTempToLevel":false } })", 0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);

    err = ctx.attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported",
        R"({ "value": {"ExecuteIfOff":true,"CoupleColorTempToLevel":true } })", 3);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);

    // it is also ok to skip parameters
    err = ctx.attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported", R"({ "value": { } })", 0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeOnOffTransitionTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::LevelControl::Attributes::OnOffTransitionTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnOffTransitionTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeOnLevel(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::LevelControl::Attributes::OnLevel::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnLevel/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeOnTransitionTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::LevelControl::Attributes::OnTransitionTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnTransitionTime/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeOffTransitionTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::LevelControl::Attributes::OffTransitionTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OffTransitionTime/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeDefaultMoveRate(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::LevelControl::Attributes::DefaultMoveRate::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/DefaultMoveRate/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeStartUpCurrentLevel(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/StartUpCurrentLevel/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlAttributeGeneratedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestLevelControlAttributeAcceptedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestLevelControlAttributeAttributeList(nlTestSuite * sSuite, void * apContext) {}
static void TestLevelControlAttributeFeatureMap(nlTestSuite * sSuite, void * apContext) {}
static void TestLevelControlAttributeClusterRevision(nlTestSuite * sSuite, void * apContext) {}

static void TestLevelControlCommandMoveToLevel(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::MoveToLevel::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::MoveToLevel::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToLevel",
        R"({"Level":0,"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"TransitionTime":null})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandMove(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::Move::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::Move::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Move",
        R"({"MoveMode":"Up","OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"Rate":null})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandStep(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::Step::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::Step::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Step",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"StepMode":"Up","StepSize":0,"TransitionTime":null})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandStop(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::Stop::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::Stop::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Stop",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false}})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandMoveToLevelWithOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToLevelWithOnOff",
        R"({"Level":0,"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"TransitionTime":null})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandMoveWithOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::MoveWithOnOff::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::MoveWithOnOff::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveWithOnOff",
        R"({"MoveMode":"Up","OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"Rate":null})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandStepWithOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::StepWithOnOff::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::StepWithOnOff::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/StepWithOnOff",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"StepMode":"Up","StepSize":0,"TransitionTime":null})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandStopWithOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::LevelControl::Commands::StopWithOnOff::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::LevelControl::Commands::StopWithOnOff::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/StopWithOnOff",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false}})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestLevelControlCommandMoveToClosestFrequency(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Type request;
    request.frequency = 142;
    ctx.command_test(sSuite, "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToClosestFrequency",
        R"({"Frequency": 142})",
        request
    );
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeRemainingTime", TestLevelControlAttributeRemainingTime),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeMinLevel", TestLevelControlAttributeMinLevel),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeMaxLevel", TestLevelControlAttributeMaxLevel),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeCurrentFrequency", TestLevelControlAttributeCurrentFrequency),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeMinFrequency", TestLevelControlAttributeMinFrequency),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeMaxFrequency", TestLevelControlAttributeMaxFrequency),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeOptions", TestLevelControlAttributeOptions),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeOnOffTransitionTime", TestLevelControlAttributeOnOffTransitionTime),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeOnLevel", TestLevelControlAttributeOnLevel),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeOnTransitionTime", TestLevelControlAttributeOnTransitionTime),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeOffTransitionTime", TestLevelControlAttributeOffTransitionTime),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeDefaultMoveRate", TestLevelControlAttributeDefaultMoveRate),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeStartUpCurrentLevel", TestLevelControlAttributeStartUpCurrentLevel),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeGeneratedCommandList", TestLevelControlAttributeGeneratedCommandList),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeAcceptedCommandList", TestLevelControlAttributeAcceptedCommandList),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeAttributeList", TestLevelControlAttributeAttributeList),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeFeatureMap", TestLevelControlAttributeFeatureMap),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeClusterRevision", TestLevelControlAttributeClusterRevision),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMoveToClosestFrequency", TestLevelControlCommandMoveToClosestFrequency),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMoveToLevel", TestLevelControlCommandMoveToLevel),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMove", TestLevelControlCommandMove),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandStep", TestLevelControlCommandStep),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandStop", TestLevelControlCommandStop),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMoveToLevelWithOnOff", TestLevelControlCommandMoveToLevelWithOnOff),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMoveWithOnOff", TestLevelControlCommandMoveWithOnOff),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandStepWithOnOff", TestLevelControlCommandStepWithOnOff),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandStopWithOnOff", TestLevelControlCommandStopWithOnOff),
    NL_TEST_DEF("LevelControl::TestLevelControlCommandMoveToClosestFrequency", TestLevelControlCommandMoveToClosestFrequency),
    NL_TEST_DEF("LevelControl::TestLevelControlAttributeWrite", TestLevelControlAttributeWrite),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "LevelControlTests", &sTests[0], Initialize, TestContext::Finalize };

int TestLevelControlSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestLevelControlSuite)