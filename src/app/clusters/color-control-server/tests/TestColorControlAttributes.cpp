/*
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// Whole-cluster functionality: global/derived attribute reads, writable attributes (Options,
// StartUpColorTemperatureMireds), the feature-gated Attributes() list, and Startup behavior
// (StartUpColorTemperatureMireds forcing color-temperature mode).

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/ColorControl/Attributes.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status = Protocols::InteractionModel::Status;

constexpr EndpointId kEp = 1;

struct TestColorControlAttributes : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    ColorControlDelegate delegate;

    ColorControlCluster::Config CtConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorTemperature);
        c.mColorValue                         = CTColor{ .mireds = 250 };
        c.ctConfig.colorTempPhysicalMinMireds = 100;
        c.ctConfig.colorTempPhysicalMaxMireds = 400;
        return c;
    }

    static bool AttributeListHas(ColorControlCluster & c, AttributeId id)
    {
        ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
        EXPECT_EQ(c.Attributes(ConcreteClusterPath(kEp, ColorControl::Id), builder), CHIP_NO_ERROR);
        for (const auto & entry : builder.TakeBuffer())
        {
            if (entry.attributeId == id)
            {
                return true;
            }
        }
        return false;
    }
};

TEST_F(TestColorControlAttributes, FeatureMapAndColorCapabilities)
{
    ColorControlCluster::Config config = CtConfig();
    config.mFeatures.Set(Feature::kXy); // CT + XY
    ColorControlCluster c(kEp, config);
    Testing::ClusterTester tester(c);
    ASSERT_EQ(c.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t featureMap = 0;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap).IsSuccess());
    EXPECT_EQ(featureMap, static_cast<uint32_t>(config.mFeatures.Raw()));

    // ColorCapabilities mirrors the feature map bit-for-bit.
    BitMask<ColorCapabilitiesBitmap> caps;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::ColorCapabilities::Id, caps).IsSuccess());
    EXPECT_EQ(caps.Raw(), static_cast<uint16_t>(config.mFeatures.Raw()));

    uint16_t revision = 0;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision).IsSuccess());
    EXPECT_GT(revision, 0);
}

TEST_F(TestColorControlAttributes, OptionsWriteRoundTrip)
{
    ColorControlCluster c(kEp, CtConfig());
    Testing::ClusterTester tester(c);
    ASSERT_EQ(c.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    BitMask<OptionsBitmap> options(OptionsBitmap::kExecuteIfOff);
    ASSERT_TRUE(tester.WriteAttribute(Attributes::Options::Id, options).IsSuccess());

    BitMask<OptionsBitmap> readBack;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::Options::Id, readBack).IsSuccess());
    EXPECT_EQ(readBack.Raw(), options.Raw());
}

TEST_F(TestColorControlAttributes, StartUpColorTemperatureWriteRoundTripAndConstraint)
{
    ColorControlCluster c(kEp, CtConfig());
    Testing::ClusterTester tester(c);
    ASSERT_EQ(c.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Valid value round-trips.
    ASSERT_TRUE(tester.WriteAttribute(Attributes::StartUpColorTemperatureMireds::Id,
                                      DataModel::MakeNullable<uint16_t>(300))
                    .IsSuccess());
    DataModel::Nullable<uint16_t> readBack;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::StartUpColorTemperatureMireds::Id, readBack).IsSuccess());
    ASSERT_FALSE(readBack.IsNull());
    EXPECT_EQ(readBack.Value(), 300);

    // Above the spec max (0xFEFF) is rejected.
    EXPECT_EQ(tester.WriteAttribute(Attributes::StartUpColorTemperatureMireds::Id,
                                    DataModel::MakeNullable<uint16_t>(0xFF00)),
              Status::ConstraintError);
}

TEST_F(TestColorControlAttributes, AttributesListIsFeatureGated)
{
    // With color temperature enabled, its attribute is advertised.
    {
        ColorControlCluster c(kEp, CtConfig()); // kColorTemperature
        EXPECT_TRUE(AttributeListHas(c, Attributes::ColorTemperatureMireds::Id));
        EXPECT_FALSE(AttributeListHas(c, Attributes::CurrentX::Id)); // no kXy
    }
    // With XY enabled instead, CurrentX/CurrentY appear and color-temp does not.
    {
        ColorControlCluster::Config config(delegate);
        config.mFeatures.Set(Feature::kXy);
        config.mColorValue = XYColor{ .x = 1, .y = 2 };
        ColorControlCluster c(kEp, config);
        EXPECT_TRUE(AttributeListHas(c, Attributes::CurrentX::Id));
        EXPECT_TRUE(AttributeListHas(c, Attributes::CurrentY::Id));
        EXPECT_FALSE(AttributeListHas(c, Attributes::ColorTemperatureMireds::Id));
    }
}

TEST_F(TestColorControlAttributes, StartupAppliesStartUpColorTemperature)
{
    ColorControlCluster::Config config = CtConfig();
    config.ctConfig.startUpColorTemperatureMireds.SetNonNull(300); // within [100,400]
    ColorControlCluster c(kEp, config);
    Testing::ClusterTester tester(c);
    ASSERT_EQ(c.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // §3.2.11.10: startup forces color-temperature mode at the configured value.
    EXPECT_EQ(c.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(c.ColorTempMireds(), 300);
}

} // namespace
