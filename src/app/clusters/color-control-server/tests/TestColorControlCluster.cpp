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
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status = Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

// Spec limits mirrored from ColorControlCluster:
constexpr uint16_t kMaxColorTempMireds = 0xFEFF; // 65279, ColorTemperatureMireds max
constexpr uint16_t kMaxCieXy           = 0xFEFF; // 65279, CurrentX/CurrentY max
constexpr uint16_t kMaxTransitionTime  = 65534;

// CT + XY features, starting in color-temperature mode.
ColorControlCluster::Config MakeConfig(ColorControlDelegate & delegate, TimerDelegate & timerDelegate)
{
    ColorControlCluster::Config config(delegate, timerDelegate);
    config.mFeatures.Set(Feature::kColorTemperature).Set(Feature::kXy);
    config.mColorValue                              = CTColor{ 250 };
    config.ctConfig.colorTempPhysicalMinMireds      = 100;
    config.ctConfig.colorTempPhysicalMaxMireds      = 400;
    config.ctConfig.coupleColorTempToLevelMinMireds = 150;
    return config;
}

struct TestColorControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    ColorControlDelegate delegate;
};

TEST_F(TestColorControlCluster, AccessorsReflectConfiguredMode)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate, mockTimer));

    // The active variant (CTColor) is the source of truth for the (Enhanced)ColorMode.
    EXPECT_EQ(cluster.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(cluster.ColorTempMireds(), 250u);

    // Out-of-mode accessors return the neutral default (no cross-mode conversion).
    EXPECT_EQ(cluster.CurrentX(), 0u);
    EXPECT_EQ(cluster.CurrentY(), 0u);
}

TEST_F(TestColorControlCluster, SupportsModeReflectsFeatures)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate, mockTimer)); // CT + XY only

    EXPECT_TRUE(cluster.SupportsMode(EnhancedColorModeEnum::kColorTemperatureMireds));
    EXPECT_TRUE(cluster.SupportsMode(EnhancedColorModeEnum::kCurrentXAndCurrentY));
    EXPECT_FALSE(cluster.SupportsMode(EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation));
}

TEST_F(TestColorControlCluster, ColorTemperatureCommandBoundaries)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate, mockTimer));

    // MoveToColorTemperature: mireds and transition-time upper bounds.
    EXPECT_EQ(cluster.MoveToColorTemp(kMaxColorTempMireds + 1, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.MoveToColorTemp(200, kMaxTransitionTime + 1), Status::ConstraintError);

    // MoveColorTemperature: field-mireds bounds, and rate==0 on a non-stop move is invalid.
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kUp, 10, kMaxColorTempMireds + 1, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kUp, 10, 0, kMaxColorTempMireds + 1), Status::ConstraintError);
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kUp, 0, 0, 0), Status::InvalidCommand);
    // Stop is always accepted and starts no transition.
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success);

    // StepColorTemperature: field-mireds and transition-time bounds.
    EXPECT_EQ(cluster.StepColorTemp(StepModeEnum::kUp, 10, kMaxTransitionTime + 1, 0, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.StepColorTemp(StepModeEnum::kUp, 10, 0, kMaxColorTempMireds + 1, 0), Status::ConstraintError);
}

TEST_F(TestColorControlCluster, MoveToColorBoundaries)
{
    ColorControlCluster cluster(kTestEndpointId, MakeConfig(delegate, mockTimer));

    // CurrentX / CurrentY above the CIE max (65279) are rejected.
    EXPECT_EQ(cluster.MoveToColor(kMaxCieXy + 1, 100, 0), Status::ConstraintError);
    EXPECT_EQ(cluster.MoveToColor(100, kMaxCieXy + 1, 0), Status::ConstraintError);
    // Transition time above the max is rejected.
    EXPECT_EQ(cluster.MoveToColor(100, 100, kMaxTransitionTime + 1), Status::ConstraintError);
}

// Interpolate is the position math every Tick* forwards its axis to: start + delta over durationMs,
// anchored at startTimeMs. Exercised directly here — no cluster, no timer.
constexpr uint64_t kAnchorMs = 1000;

TEST(TestInterpolate, RampReachesExactEndpoint)
{
    // Halfway through a 1000 ms ramp of +100, then arrival, then past the end.
    EXPECT_EQ(Interpolate(20, 100, kAnchorMs, 1000, kAnchorMs + 500).value, 70);
    EXPECT_FALSE(Interpolate(20, 100, kAnchorMs, 1000, kAnchorMs + 500).done);

    // Arrival lands on start + delta exactly, not on the interpolant, and stays there afterwards.
    EXPECT_EQ(Interpolate(20, 100, kAnchorMs, 1000, kAnchorMs + 1000).value, 120);
    EXPECT_TRUE(Interpolate(20, 100, kAnchorMs, 1000, kAnchorMs + 1000).done);
    EXPECT_EQ(Interpolate(20, 100, kAnchorMs, 1000, kAnchorMs + 5000).value, 120);
}

TEST(TestInterpolate, NegativeDeltaRampsDown)
{
    EXPECT_EQ(Interpolate(200, -100, kAnchorMs, 1000, kAnchorMs + 250).value, 175);
    EXPECT_EQ(Interpolate(200, -100, kAnchorMs, 1000, kAnchorMs + 1000).value, 100);
}

TEST(TestInterpolate, ZeroDurationArrivesImmediately)
{
    // transitionTime 0: the first tick is the last one. Also the 0/0 -> NaN guard, since this tick can
    // land at elapsed == 0.
    const auto immediate = Interpolate(20, 100, kAnchorMs, 0, kAnchorMs);
    EXPECT_EQ(immediate.value, 120);
    EXPECT_TRUE(immediate.done);
}

TEST(TestInterpolate, RateMoveWrapsAndNeverArrives)
{
    // kIndefiniteHueMoveMs (MoveHue): delta is units per SECOND, so 2 s at 100/s travels 200 ...
    const auto moving = Interpolate(0x100, 100, kAnchorMs, kIndefiniteHueMoveMs, kAnchorMs + 2000);
    EXPECT_EQ(moving.value, 0x100 + 200);
    EXPECT_FALSE(moving.done); // only StopHueAxis / ColorLoop-Deactivate ends it

    // ... and the position wraps mod 0x10000 rather than growing without bound: 0x10000 units at
    // 0x1000/s takes 16 s, i.e. one full revolution back to the start.
    EXPECT_EQ(Interpolate(0x100, 0x1000, kAnchorMs, kIndefiniteHueMoveMs, kAnchorMs + 16000).value, 0x100);
    // Downwards past zero wraps the same way.
    EXPECT_EQ(Interpolate(0x100, -0x200, kAnchorMs, kIndefiniteHueMoveMs, kAnchorMs + 1000).value, 0xFF00);
}

} // namespace
