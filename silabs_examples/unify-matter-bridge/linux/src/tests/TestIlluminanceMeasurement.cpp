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
using namespace chip::app::Clusters::IlluminanceMeasurement;

using TestContext = Test::ClusterContext<IlluminanceMeasurementAttributeAccess, IlluminanceMeasurementClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("IlluminanceMeasurement");
    cluster.attributes = {
        "LightSensorType",
    };
    return ctx->register_endpoint(ep);
}

static void TestIlluminanceMeasurementAttributeIllumLightSensorType(nlTestSuite * sSuite, void * apContext)
{
    auto & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Attributes::LightSensorType::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/IlluminanceMeasurement/Attributes/LightSensorType/Reported", R"({ "value": "Photodiode"})", MakeNullable(LightSensorTypeEnum::kPhotodiode));
    CHIP_ERROR err1    = ctx.attribute_test<Attributes::LightSensorType::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/IlluminanceMeasurement/Attributes/LightSensorType/Reported", R"({ "value": "CMOS"})", MakeNullable(LightSensorTypeEnum::kCmos));
    NL_TEST_ASSERT(sSuite, ((err == CHIP_NO_ERROR) && (err1 == CHIP_NO_ERROR)));
}
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("IlluminanceMeasurement::TestIlluminanceMeasurementAttributeIllumLightSensorType", TestIlluminanceMeasurementAttributeIllumLightSensorType),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "IlluminanceMeasurementTests", &sTests[0], Initialize, TestContext::Finalize };

int TestIlluminanceMeasurementSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestIlluminanceMeasurementSuite)