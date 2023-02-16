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
using TestContext = Test::ClusterContext<ColorControlAttributeAccess, ColorControlClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("ColorControl");
    cluster.attributes = {
        "CurrentHue",
        "CurrentSaturation",
        "RemainingTime",
        "CurrentX",
        "CurrentY",
        "DriftCompensation",
        "CompensationText", // Unify SDK doesn't support it.
        "ColorTemperatureMireds",
        "ColorMode", // Matter doesn't support enum.
        "Options",   // Unify SDK doesn't support it.
        "NumberOfPrimaries",
        "Primary1X",
        "Primary1Y",
        "Primary1Intensity",
        "Primary2X",
        "Primary2Y",
        "Primary2Intensity",
        "Primary3X",
        "Primary3Y",
        "Primary3Intensity",
        "Primary4X",
        "Primary4Y",
        "Primary4Intensity",
        "Primary5X",
        "Primary5Y",
        "Primary5Intensity",
        "Primary6X",
        "Primary6Y",
        "Primary6Intensity",
        "WhitePointX",
        "WhitePointY",
        "ColorPointRX",
        "ColorPointRY",
        "ColorPointRIntensity",
        "ColorPointGX",
        "ColorPointGY",
        "ColorPointGIntensity",
        "ColorPointBX",
        "ColorPointBY",
        "ColorPointBIntensity",
        "EnhancedCurrentHue",
        "EnhancedColorMode", // Matter doesn't support enum.
        "ColorLoopActive",
        "ColorLoopDirection",
        "ColorLoopTime",
        "ColorLoopStartEnhancedHue",
        "ColorLoopStoredEnhancedHue",
        "ColorCapabilities", // Matter doesn't support bitmask.
        "ColorTempPhysicalMinMireds",
        "ColorTempPhysicalMaxMireds",
        "CoupleColorTempToLevelMinMireds",
        "StartUpColorTemperatureMireds",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };

    cluster.supported_commands = {
        // A checklist of UnifySDK handler implementations for ColorControl.
        // For more in detail, see;
        // unify_dotdot_attribute_store_command_callbacks_color_control.c:color_control_cluster_mapper_init
        /* - [X] */ "MoveToHue",
        /* - [X] */ "MoveHue",
        /* - [X] */ "StepHue",
        /* - [X] */ "MoveToSaturation",
        /* - [ ] */ "MoveSaturation", // Handling MoveMode unimplemented.
        /* - [X] */ "StepSaturation",
        /* - [X] */ "MoveToHueAndSaturation",
        /* - [ ] */ "MoveToColor",
        /* - [ ] */ "MoveColor",
        /* - [ ] */ "StepColor",
        /* - [X] */ "MoveToColorTemperature",
        /* - [ ] */ "EnhancedMoveToHue",
        /* - [ ] */ "EnhancedMoveHue",
        /* - [ ] */ "EnhancedStepHue",
        /* - [ ] */ "EnhancedMoveToHueAndSaturation",
        /* - [ ] */ "ColorLoopSet",
        /* - [X] */ "StopMoveStep",
        /* - [X] */ "MoveColorTemperature",
        /* - [X] */ "StepColorTemperature",
    };

    return ctx->register_endpoint(ep);
}

static void TestColorControlAttributeCurrentHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::CurrentHue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentHue/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeCurrentSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::CurrentSaturation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentSaturation/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeRemainingTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::RemainingTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/RemainingTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeCurrentX(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::CurrentX::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentX/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeCurrentY(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::CurrentY::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentY/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeDriftCompensation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::DriftCompensation::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/DriftCompensation/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/*static void TestColorControlAttributeCompensationText(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    auto result       = ctx.attribute_test<Clusters::ColorControl::Attributes::CompensationText::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CompensationText/Reported", R"({ "value": "SomeValue" })");
    NL_TEST_ASSERT(sSuite, result.unwrap().empty());
}*/

static void TestColorControlAttributeColorTemperatureMireds(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorTemperatureMireds::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTemperatureMireds/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/*static void TestColorControlAttributeColorMode(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorMode::TypeInfo, false>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorMode/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}*/

// static void TestColorControlAttributeOptions(nlTestSuite * sSuite, void * apContext)
// {
//     TestContext & ctx = *static_cast<TestContext *>(apContext);
//     CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Options::TypeInfo, false>(
//         sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Options/Reported", R"({ "value": 42 })", 42);
//     NL_TEST_ASSERT(sSuite, err == CHIP_ERROR(0x00000586));
// }

static void TestColorControlAttributeNumberOfPrimaries(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::NumberOfPrimaries::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/NumberOfPrimaries/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary1X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary1X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary1Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary1Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary1Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary1Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary2X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary2X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary2Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary2Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary2Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary2Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary3X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary3X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary3Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary3Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary3Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary3Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary4X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary4X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary4Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary4Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary4Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary4Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary5X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary5X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary5Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary5Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary5Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary5Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary6X(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary6X::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6X/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary6Y(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary6Y::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6Y/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributePrimary6Intensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::Primary6Intensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6Intensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeWhitePointX(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::WhitePointX::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/WhitePointX/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeWhitePointY(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::WhitePointY::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/WhitePointY/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointRX(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointRX::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRX/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointRY(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointRY::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRY/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointRIntensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointRIntensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRIntensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointGX(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointGX::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGX/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointGY(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointGY::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGY/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointGIntensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointGIntensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGIntensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointBX(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointBX::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBX/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointBY(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointBY::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBY/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorPointBIntensity(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorPointBIntensity::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBIntensity/Reported", R"({ "value": 42 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeEnhancedCurrentHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::EnhancedCurrentHue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/EnhancedCurrentHue/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/*static void TestColorControlAttributeEnhancedColorMode(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::EnhancedColorMode::TypeInfo, false>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/EnhancedColorMode/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}*/

static void TestColorControlAttributeColorLoopActive(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorLoopActive::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopActive/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorLoopDirection(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorLoopDirection::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopDirection/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorLoopTime(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorLoopTime::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopTime/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorLoopStartEnhancedHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopStartEnhancedHue/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorLoopStoredEnhancedHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopStoredEnhancedHue/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/*static void TestColorControlAttributeColorCapabilities(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorCapabilities::TypeInfo, false>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorCapabilities/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}*/

static void TestColorControlAttributeColorTempPhysicalMinMireds(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTempPhysicalMinMireds/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeColorTempPhysicalMaxMireds(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTempPhysicalMaxMireds/Reported", R"({ "value": 42 })", 42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeCoupleColorTempToLevelMinMireds(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CoupleColorTempToLevelMinMireds/Reported", R"({ "value": 42 })",
        42);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeStartUpColorTemperatureMireds(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = ctx.attribute_test<Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/StartUpColorTemperatureMireds/Reported", R"({ "value": 42 })",
        value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlAttributeGeneratedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestColorControlAttributeAcceptedCommandList(nlTestSuite * sSuite, void * apContext) {}
static void TestColorControlAttributeAttributeList(nlTestSuite * sSuite, void * apContext) {}
static void TestColorControlAttributeFeatureMap(nlTestSuite * sSuite, void * apContext) {}
static void TestColorControlAttributeClusterRevision(nlTestSuite * sSuite, void * apContext) {}

static void TestColorControlCommandMoveToHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveToHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveToHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToHue",
        R"({"Direction":"ShortestDistance","Hue":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveHue",
        R"({"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandStepHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::StepHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::StepHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepHue",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"{}","StepSize":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveToSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveToSaturation::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveToSaturation::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToSaturation",
        R"({"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveSaturation::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveSaturation::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveSaturation",
        R"({"MoveMode":"Stop FIXME","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandStepSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::StepSaturation::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::StepSaturation::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepSaturation",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"{}","StepSize":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveToHueAndSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToHueAndSaturation",
        R"({"Hue":0,"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveToColor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveToColor::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveToColor::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToColor",
        R"({"ColorX":0,"ColorY":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveColor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveColor::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveColor::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveColor",
        R"({"OptionsMask":0,"OptionsOverride":0,"RateX":0,"RateY":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandStepColor(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::StepColor::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::StepColor::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepColor",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepX":0,"StepY":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveToColorTemperature(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveToColorTemperature::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveToColorTemperature::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToColorTemperature",
        R"({"ColorTemperatureMireds":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandEnhancedMoveToHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::EnhancedMoveToHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::EnhancedMoveToHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveToHue",
        R"({"Direction":"ShortestDistance","EnhancedHue":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandEnhancedMoveHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::EnhancedMoveHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::EnhancedMoveHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveHue",
        R"({"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandEnhancedStepHue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::EnhancedStepHue::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::EnhancedStepHue::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedStepHue",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"{}","StepSize":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandEnhancedMoveToHueAndSaturation(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveToHueAndSaturation",
        R"({"EnhancedHue":0,"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandColorLoopSet(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::ColorLoopSet::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::ColorLoopSet::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/ColorLoopSet",
        R"({"Action":"DeactivateColorLoop","Direction":"DecrementEnhancedCurrentHue","OptionsMask":0,"OptionsOverride":0,"StartHue":0,"Time":0,"UpdateFlags":{"UpdateAction":false,"UpdateDirection":false,"UpdateStartHue":false,"UpdateTime":false}})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandStopMoveStep(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::StopMoveStep::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandMoveColorTemperature(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::MoveColorTemperature::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::MoveColorTemperature::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveColorTemperature",
        R"({"ColorTemperatureMaximumMireds":0,"ColorTemperatureMinimumMireds":0,"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestColorControlCommandStepColorTemperature(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::ColorControl::Commands::StepColorTemperature::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::ColorControl::Commands::StepColorTemperature::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepColorTemperature",
        R"({"ColorTemperatureMaximumMireds":0,"ColorTemperatureMinimumMireds":0,"OptionsMask":0,"OptionsOverride":0,"StepMode":"{}","StepSize":0,"TransitionTime":0})",
        request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sAttributeTests[] = {
    NL_TEST_DEF("ColorControl::TestColorControlAttributeCurrentHue", TestColorControlAttributeCurrentHue),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeCurrentSaturation", TestColorControlAttributeCurrentSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeRemainingTime", TestColorControlAttributeRemainingTime),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeCurrentX", TestColorControlAttributeCurrentX),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeCurrentY", TestColorControlAttributeCurrentY),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeDriftCompensation", TestColorControlAttributeDriftCompensation),
    //NL_TEST_DEF("ColorControl::TestColorControlAttributeCompensationText", TestColorControlAttributeCompensationText),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorTemperatureMireds", TestColorControlAttributeColorTemperatureMireds),
    //NL_TEST_DEF("ColorControl::TestColorControlAttributeColorMode", TestColorControlAttributeColorMode),
    // NL_TEST_DEF("ColorControl::TestColorControlAttributeOptions", TestColorControlAttributeOptions),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeNumberOfPrimaries", TestColorControlAttributeNumberOfPrimaries),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary1X", TestColorControlAttributePrimary1X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary1Y", TestColorControlAttributePrimary1Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary1Intensity", TestColorControlAttributePrimary1Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary2X", TestColorControlAttributePrimary2X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary2Y", TestColorControlAttributePrimary2Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary2Intensity", TestColorControlAttributePrimary2Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary3X", TestColorControlAttributePrimary3X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary3Y", TestColorControlAttributePrimary3Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary3Intensity", TestColorControlAttributePrimary3Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary4X", TestColorControlAttributePrimary4X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary4Y", TestColorControlAttributePrimary4Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary4Intensity", TestColorControlAttributePrimary4Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary5X", TestColorControlAttributePrimary5X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary5Y", TestColorControlAttributePrimary5Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary5Intensity", TestColorControlAttributePrimary5Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary6X", TestColorControlAttributePrimary6X),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary6Y", TestColorControlAttributePrimary6Y),
    NL_TEST_DEF("ColorControl::TestColorControlAttributePrimary6Intensity", TestColorControlAttributePrimary6Intensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeWhitePointX", TestColorControlAttributeWhitePointX),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeWhitePointY", TestColorControlAttributeWhitePointY),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointRX", TestColorControlAttributeColorPointRX),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointRY", TestColorControlAttributeColorPointRY),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointRIntensity", TestColorControlAttributeColorPointRIntensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointGX", TestColorControlAttributeColorPointGX),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointGY", TestColorControlAttributeColorPointGY),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointGIntensity", TestColorControlAttributeColorPointGIntensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointBX", TestColorControlAttributeColorPointBX),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointBY", TestColorControlAttributeColorPointBY),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorPointBIntensity", TestColorControlAttributeColorPointBIntensity),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeEnhancedCurrentHue", TestColorControlAttributeEnhancedCurrentHue),
    //NL_TEST_DEF("ColorControl::TestColorControlAttributeEnhancedColorMode", TestColorControlAttributeEnhancedColorMode),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorLoopActive", TestColorControlAttributeColorLoopActive),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorLoopDirection", TestColorControlAttributeColorLoopDirection),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorLoopTime", TestColorControlAttributeColorLoopTime),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorLoopStartEnhancedHue",
                TestColorControlAttributeColorLoopStartEnhancedHue),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorLoopStoredEnhancedHue",
                TestColorControlAttributeColorLoopStoredEnhancedHue),
    //NL_TEST_DEF("ColorControl::TestColorControlAttributeColorCapabilities", TestColorControlAttributeColorCapabilities),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorTempPhysicalMinMireds",
                TestColorControlAttributeColorTempPhysicalMinMireds),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeColorTempPhysicalMaxMireds",
                TestColorControlAttributeColorTempPhysicalMaxMireds),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeCoupleColorTempToLevelMinMireds",
                TestColorControlAttributeCoupleColorTempToLevelMinMireds),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeStartUpColorTemperatureMireds",
                TestColorControlAttributeStartUpColorTemperatureMireds),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeGeneratedCommandList", TestColorControlAttributeGeneratedCommandList),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeAcceptedCommandList", TestColorControlAttributeAcceptedCommandList),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeAttributeList", TestColorControlAttributeAttributeList),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeFeatureMap", TestColorControlAttributeFeatureMap),
    NL_TEST_DEF("ColorControl::TestColorControlAttributeClusterRevision", TestColorControlAttributeClusterRevision),
    NL_TEST_SENTINEL()
};

static const nlTest sCommandTests[] = {
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveToHue", TestColorControlCommandMoveToHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveHue", TestColorControlCommandMoveHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandStepHue", TestColorControlCommandStepHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveToSaturation", TestColorControlCommandMoveToSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveSaturation", TestColorControlCommandMoveSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlCommandStepSaturation", TestColorControlCommandStepSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveToHueAndSaturation", TestColorControlCommandMoveToHueAndSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveToColor", TestColorControlCommandMoveToColor),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveColor", TestColorControlCommandMoveColor),
    NL_TEST_DEF("ColorControl::TestColorControlCommandStepColor", TestColorControlCommandStepColor),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveToColorTemperature", TestColorControlCommandMoveToColorTemperature),
    NL_TEST_DEF("ColorControl::TestColorControlCommandEnhancedMoveToHue", TestColorControlCommandEnhancedMoveToHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandEnhancedMoveHue", TestColorControlCommandEnhancedMoveHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandEnhancedStepHue", TestColorControlCommandEnhancedStepHue),
    NL_TEST_DEF("ColorControl::TestColorControlCommandEnhancedMoveToHueAndSaturation",
                TestColorControlCommandEnhancedMoveToHueAndSaturation),
    NL_TEST_DEF("ColorControl::TestColorControlCommandColorLoopSet", TestColorControlCommandColorLoopSet),
    NL_TEST_DEF("ColorControl::TestColorControlCommandStopMoveStep", TestColorControlCommandStopMoveStep),
    NL_TEST_DEF("ColorControl::TestColorControlCommandMoveColorTemperature", TestColorControlCommandMoveColorTemperature),
    NL_TEST_DEF("ColorControl::TestColorControlCommandStepColorTemperature", TestColorControlCommandStepColorTemperature),
    NL_TEST_SENTINEL()
};

static nlTestSuite sAttributeSuite = { "ColorControlAttributeTests", &sAttributeTests[0], Initialize, TestContext::Finalize };
static nlTestSuite sCommandSuite   = { "ColorControlCommandTests", &sCommandTests[0], Initialize, TestContext::Finalize };

int TestColorControlAttributeSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sAttributeSuite);
}

int TestColorControlCommandSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sCommandSuite);
}

CHIP_REGISTER_TEST_SUITE(TestColorControlAttributeSuite)
CHIP_REGISTER_TEST_SUITE(TestColorControlCommandSuite)
