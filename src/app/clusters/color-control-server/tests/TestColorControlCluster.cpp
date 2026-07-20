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

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status = Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

// Spec limits mirrored from ColorControlCluster (§ values):
constexpr uint16_t kMaxColorTempMireds = 0xFEFF; // 65279, ColorTemperatureMireds max
constexpr uint16_t kMaxCieXy           = 0xFEFF; // 65279, CurrentX/CurrentY max
constexpr uint16_t kMaxTransitionTime  = 65534;

// CT + XY features, starting in color-temperature mode.
ColorControlCluster::Config MakeConfig(ColorControlDelegate & delegate)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorTemperature).Set(Feature::kXy);
    config.mColorValue                              = CTColor{ .mireds = 250 };
    config.ctConfig.colorTempPhysicalMinMireds      = 100;
    config.ctConfig.colorTempPhysicalMaxMireds      = 400;
    config.ctConfig.coupleColorTempToLevelMinMireds = 150;
    return config;
}

struct TestColorControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    ColorControlDelegate delegate;
};

TEST_F(TestColorControlCluster, AccessorsReflectConfiguredMode)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate));

    // The active variant (CTColor) is the source of truth for the (Enhanced)ColorMode.
    EXPECT_EQ(cluster.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(cluster.ColorTempMireds(), 250u);

    // Out-of-mode accessors return the neutral default (no cross-mode conversion).
    EXPECT_EQ(cluster.CurrentX(), 0u);
    EXPECT_EQ(cluster.CurrentY(), 0u);
}

TEST_F(TestColorControlCluster, SupportsModeReflectsFeatures)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate)); // CT + XY only

    EXPECT_TRUE(cluster.SupportsMode(EnhancedColorModeEnum::kColorTemperatureMireds));
    EXPECT_TRUE(cluster.SupportsMode(EnhancedColorModeEnum::kCurrentXAndCurrentY));
    EXPECT_FALSE(cluster.SupportsMode(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation));
}


TEST_F(TestColorControlCluster, ColorTemperatureCommandBoundaries)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate));

    // MoveToColorTemperature: mireds and transition-time upper bounds.
    EXPECT_EQ(cluster.moveToColorTemp(kMaxColorTempMireds + 1, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.moveToColorTemp(200, kMaxTransitionTime + 1), Status::ConstraintError);

    // MoveColorTemperature: field-mireds bounds, and rate==0 on a non-stop move is invalid.
    EXPECT_EQ(cluster.moveColorTemp(MoveModeEnum::kUp, 10, kMaxColorTempMireds + 1, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.moveColorTemp(MoveModeEnum::kUp, 10, 0, kMaxColorTempMireds + 1), Status::ConstraintError);
    EXPECT_EQ(cluster.moveColorTemp(MoveModeEnum::kUp, 0, 0, 0), Status::InvalidCommand);
    // Stop is always accepted and starts no transition.
    EXPECT_EQ(cluster.moveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success);

    // StepColorTemperature: field-mireds and transition-time bounds.
    EXPECT_EQ(cluster.stepColorTemp(StepModeEnum::kUp, 10, kMaxTransitionTime + 1, 0, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.stepColorTemp(StepModeEnum::kUp, 10, 0, kMaxColorTempMireds + 1, 0), Status::ConstraintError);
}

TEST_F(TestColorControlCluster, MoveToColorBoundaries)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate));

    // CurrentX / CurrentY above the CIE max (0xFEFF) are rejected.
    EXPECT_EQ(cluster.moveToColor(kMaxCieXy + 1, 100, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.moveToColor(100, kMaxCieXy + 1, 0), Status::ConstraintError);
    // Transition time above the max is rejected.
    EXPECT_EQ(cluster.moveToColor(100, 100, kMaxTransitionTime + 1), Status::ConstraintError);
}

} // namespace
