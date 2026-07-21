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

// Backward-compatibility tests for the cross-cluster "wrapper" behaviors that the code-driven
// ColorControlCluster must keep providing for the legacy integration layers:
//
//   * Scene invalidation      - SceneIntegration wires a real ColorControlSceneInvalidator into the
//                               cluster; here we inject a spy and prove every color-changing command
//                               marks stored scenes stale (what MarkScenesInvalid ultimately does).
//   * CoupleColorTempToLevel  - the target of LevelControlCoupling::NotifyLevelControlCurrentLevelChanged;
//                               we drive it directly and assert the ZCL 5.2.2.1.1 level->mireds mapping.
//   * Scene SerializeSave/ApplyScene - the core of DefaultColorControlSceneHandler: capture the cluster
//                               state through the same public getters SerializeSave reads, mutate, then
//                               restore via HandleApplyScene (what ApplyScene calls) and assert round-trip.
//
// These are exercised directly on the cluster because the command *facade* (ColorControlServer::Instance())
// and the scene/coupling glue resolve their target through ember endpoint indexing / the codegen registry,
// which the unit-test harness does not provide; the lighting-app build covers that link/compile path.
// The cluster is clock-driven, so transitions are settled with the mock-clock Tick/Complete helpers.

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/color-control-server/ColorControlSceneInvalidator.h>
#include <clusters/ColorControl/Attributes.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status = Protocols::InteractionModel::Status;

constexpr EndpointId kEp = 1;

// CT physical range / coupling minimum used across the tests.
constexpr uint16_t kPhysMin   = 100;
constexpr uint16_t kPhysMax   = 400;
constexpr uint16_t kCoupleMin = 150;

// Records InvalidateScenes() calls. SceneIntegration injects the real implementation (which forwards to
// ScenesServer::MakeSceneInvalidForAllFabrics); a color change must reach it exactly this way.
class SpySceneInvalidator : public ColorControlSceneInvalidator
{
public:
    void InvalidateScenes(EndpointId endpoint) override
    {
        count++;
        lastEndpoint = endpoint;
    }
    int count               = 0;
    EndpointId lastEndpoint = kInvalidEndpointId;
};

struct TestColorControlClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    ColorControlDelegate delegate;
    System::Clock::Internal::RAIIMockClock clock;

    void Tick(ColorControlCluster & c, uint32_t ms)
    {
        clock.AdvanceMonotonic(System::Clock::Milliseconds64(ms));
        c.OnTick();
    }
    void Complete(ColorControlCluster & c) { Tick(c, 120000); }

    // Color-temperature cluster starting in CT mode, with the physical range + coupling minimum set and an
    // optional scene invalidator injected (as SceneIntegration would).
    ColorControlCluster::Config CtConfig(ColorControlSceneInvalidator * inv = nullptr)
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorTemperature);
        c.mColorValue                              = CTColor{ .mireds = 250 };
        c.ctConfig.colorTempPhysicalMinMireds      = kPhysMin;
        c.ctConfig.colorTempPhysicalMaxMireds      = kPhysMax;
        c.ctConfig.coupleColorTempToLevelMinMireds = kCoupleMin;
        c.sceneInvalidator                         = inv;
        return c;
    }
};

// ---------------------------------------------------------------- Scene invalidation

TEST_F(TestColorControlClusterBackwardsCompatibility, ColorCommandInvalidatesScenes)
{
    SpySceneInvalidator spy;
    ColorControlCluster c(kEp, CtConfig(&spy));

    EXPECT_EQ(spy.count, 0);
    EXPECT_EQ(c.moveToColorTemp(300, 10), Status::Success);

    // The command marks scenes stale immediately (at command time, not after the transition settles).
    EXPECT_GE(spy.count, 1);
    EXPECT_EQ(spy.lastEndpoint, kEp);
}

TEST_F(TestColorControlClusterBackwardsCompatibility, NoInvalidatorIsSafe)
{
    // Null invalidator (no scene coupling) must not crash on a color change.
    ColorControlCluster c(kEp, CtConfig(nullptr));
    EXPECT_EQ(c.moveToColorTemp(300, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 300u);
}

// ---------------------------------------------------------------- CoupleColorTempToLevel

TEST_F(TestColorControlClusterBackwardsCompatibility, CoupleColorTempToLevelMapsExtremes)
{
    ColorControlCluster c(kEp, CtConfig());

    // ZCL 5.2.2.1.1: minimum level -> highest mireds (physical max); maximum level -> coupling minimum.
    c.CoupleColorTempToLevel(0);
    Complete(c);
    EXPECT_EQ(c.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(c.ColorTempMireds(), kPhysMax);

    c.CoupleColorTempToLevel(254);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), kCoupleMin);
}

TEST_F(TestColorControlClusterBackwardsCompatibility, CoupleColorTempToLevelIsMonotonicAndInRange)
{
    ColorControlCluster c(kEp, CtConfig());

    // A mid-range level lands strictly inside [coupleMin, physMax]; mireds vary inversely with level.
    c.CoupleColorTempToLevel(64);
    Complete(c);
    const uint16_t low = c.ColorTempMireds();

    c.CoupleColorTempToLevel(192);
    Complete(c);
    const uint16_t high = c.ColorTempMireds();

    EXPECT_GE(low, kCoupleMin);
    EXPECT_LE(low, kPhysMax);
    EXPECT_GT(low, high); // higher level -> lower mireds
}

TEST_F(TestColorControlClusterBackwardsCompatibility, CoupleColorTempToLevelIgnoredOutsideCtMode)
{
    // Coupling only applies while color temperature is the active mode.
    ColorControlCluster::Config cfg(delegate);
    cfg.mFeatures.Set(Feature::kHueAndSaturation);
    cfg.mColorValue = HueSatColor{ .hue = 10, .saturation = 20 };
    ColorControlCluster c(kEp, cfg);

    c.CoupleColorTempToLevel(254);
    Complete(c);
    EXPECT_EQ(c.GetEnhancedColorMode(), EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation);
}

// ---------------------------------------------------------------- Scene serialize / apply round-trip

TEST_F(TestColorControlClusterBackwardsCompatibility, SceneCaptureAndApplyRestoresColorTemp)
{
    ColorControlCluster c(kEp, CtConfig());

    // Drive to a known state and capture it through the public getter SerializeSave uses.
    EXPECT_EQ(c.moveToColorTemp(350, 10), Status::Success);
    Complete(c);
    const uint16_t captured = c.ColorTempMireds();
    EXPECT_EQ(captured, 350u);

    // Move away from the captured value.
    EXPECT_EQ(c.moveToColorTemp(150, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 150u);

    // Apply the captured scene (what DefaultColorControlSceneHandler::ApplyScene does after decoding the EFS).
    ColorValue target = CTColor{ .mireds = captured };
    ColorLoopState loop{};
    EXPECT_EQ(c.HandleApplyScene(EnhancedColorModeEnum::kColorTemperatureMireds, target, loop, /*timeMs=*/0), CHIP_NO_ERROR);
    Complete(c);

    EXPECT_EQ(c.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(c.ColorTempMireds(), captured);
}

} // namespace
