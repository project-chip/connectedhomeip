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
using TestContext = Test::ClusterContext<OnOffAttributeAccessOverride, OnOffClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("OnOff");
    cluster.attributes = {
        "OnOff",
        "GlobalSceneControl",
        "OnTime",
        "OffWaitTime",
        "StartUpOnOff", // Unify SDK doesn't handle OnOffStartUpOnOff enum properly.
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff",
    };

    auto & identify_cluster = ep.emplace_cluster("Identify");
    identify_cluster.attributes.emplace("IdentifyTime");

    auto & scenes_cluster = ep.emplace_cluster("Scenes");
    scenes_cluster.attributes.emplace("SceneCount");

    auto & groups_cluster = ep.emplace_cluster("Groups");
    groups_cluster.attributes.emplace("NameSupport");

    return ctx->register_endpoint(ep);
}

static void TestOnOffAttributeOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::OnOff::Attributes::OnOff::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OnOff/Reported", R"({ "value": true })", true);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffAttributeGlobalSceneControl(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::OnOff::Attributes::GlobalSceneControl::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/GlobalSceneControl/Reported", R"({ "value": true })", true);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffAttributeOnTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::OnOff::Attributes::OnTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OnTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffAttributeOffWaitTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::OnOff::Attributes::OffWaitTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OffWaitTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffAttributeStartUpOnOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    DataModel::Nullable<Clusters::OnOff::OnOffStartUpOnOff> value;
    value.SetNonNull(Clusters::OnOff::OnOffStartUpOnOff::kOn);
    err = ctx.attribute_test<Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo, false>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/StartUpOnOff/Reported", R"({ "value": "SetOnOffTo1" })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
    value.SetNonNull(Clusters::OnOff::OnOffStartUpOnOff::kOff);
    err = ctx.attribute_test<Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/StartUpOnOff/Reported", R"({ "value": 0 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffAttributeGeneratedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestOnOffAttributeAcceptedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestOnOffAttributeAttributeList(nlTestSuite * sSuite, void * apContext) {}
static void TestOnOffAttributeFeatureMap(nlTestSuite * sSuite, void * apContext) {}
static void TestOnOffAttributeClusterRevision(nlTestSuite * sSuite, void * apContext) {}

static void TestOnOffCommandOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::Off::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::OnOff::Commands::Off::Type>(sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/Off",
                                                                            "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffCommandOn(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::On::Type request;
    CHIP_ERROR err =
        ctx.command_test<Clusters::OnOff::Commands::On::Type>(sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/On", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffCommandToggle(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::Toggle::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::OnOff::Commands::Toggle::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/Toggle", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffCommandOffWithEffect(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::OffWithEffect::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::OnOff::Commands::OffWithEffect::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OffWithEffect",
        R"({"EffectIdentifier":"DelayedAllOff","EffectVariant":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffCommandOnWithRecallGlobalScene(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OnWithRecallGlobalScene", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestOnOffCommandOnWithTimedOff(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::OnOff::Commands::OnWithTimedOff::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::OnOff::Commands::OnWithTimedOff::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OnWithTimedOff",
        R"({"OffWaitTime":0,"OnOffControl":{"AcceptOnlyWhenOn":false},"OnTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("OnOff::TestOnOffAttributeOnOff", TestOnOffAttributeOnOff),
    NL_TEST_DEF("OnOff::TestOnOffAttributeGlobalSceneControl", TestOnOffAttributeGlobalSceneControl),
    NL_TEST_DEF("OnOff::TestOnOffAttributeOnTime", TestOnOffAttributeOnTime),
    NL_TEST_DEF("OnOff::TestOnOffAttributeOffWaitTime", TestOnOffAttributeOffWaitTime),
    NL_TEST_DEF("OnOff::TestOnOffAttributeStartUpOnOff", TestOnOffAttributeStartUpOnOff),
    NL_TEST_DEF("OnOff::TestOnOffAttributeGeneratedCommandList", TestOnOffAttributeGeneratedCommandList),
    NL_TEST_DEF("OnOff::TestOnOffAttributeAcceptedCommandList", TestOnOffAttributeAcceptedCommandList),
    NL_TEST_DEF("OnOff::TestOnOffAttributeAttributeList", TestOnOffAttributeAttributeList),
    NL_TEST_DEF("OnOff::TestOnOffAttributeFeatureMap", TestOnOffAttributeFeatureMap),
    NL_TEST_DEF("OnOff::TestOnOffAttributeClusterRevision", TestOnOffAttributeClusterRevision),
    NL_TEST_DEF("OnOff::TestOnOffCommandOff", TestOnOffCommandOff),
    NL_TEST_DEF("OnOff::TestOnOffCommandOn", TestOnOffCommandOn),
    NL_TEST_DEF("OnOff::TestOnOffCommandToggle", TestOnOffCommandToggle),
    NL_TEST_DEF("OnOff::TestOnOffCommandOffWithEffect", TestOnOffCommandOffWithEffect),
    NL_TEST_DEF("OnOff::TestOnOffCommandOnWithRecallGlobalScene", TestOnOffCommandOnWithRecallGlobalScene),
    NL_TEST_DEF("OnOff::TestOnOffCommandOnWithTimedOff", TestOnOffCommandOnWithTimedOff),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "OnOffTests", &sTests[0], Initialize, TestContext::Finalize };

int TestOnOffSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOnOffSuite)
