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

// Thorough success-path tests for every ColorControl command. The cluster is clock-driven: a command
// sets up a transition and the attribute value is a function of the wall clock, applied on OnTick().
// So each test installs a mock clock (RAIIMockClock == SetSystemClockForTesting), invokes the (public)
// command handler, advances the clock past the transition, drives OnTick(), and asserts the resulting
// state accessor. Each cluster is constructed already in the command's target mode so ApplyModeSwitch
// is a no-op and no delegate colour-conversion is involved (results are exact). No Startup() is needed
// (OnTick's NotifyAttributeChanged is null-context safe), which also keeps the configured start mode.

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/ColorControl/Attributes.h>
#include <clusters/ColorControl/Commands.h>
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

// Saturation/CIE limits mirrored from the cluster.
constexpr uint8_t kMaxSat    = 254;
constexpr uint16_t kMaxCieXy = 0xFEFF;

struct TestColorControlCommands : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    ColorControlDelegate delegate;
    System::Clock::Internal::RAIIMockClock clock;

    // Advance the mock clock by `ms` and run one tick.
    void Tick(ColorControlCluster & c, uint32_t ms)
    {
        clock.AdvanceMonotonic(System::Clock::Milliseconds64(ms));
        c.OnTick();
    }
    // Complete any finite transition (advance well past any duration used here) and tick.
    void Complete(ColorControlCluster & c) { Tick(c, 120000); }

    ColorControlCluster::Config HsConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kHueAndSaturation);
        c.mColorValue = HueSatColor{ .hue = 10, .saturation = 20 };
        return c;
    }
    ColorControlCluster::Config EnhancedConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kEnhancedHue).Set(Feature::kHueAndSaturation);
        c.mColorValue = EnhancedHueSatColor{ .enhancedHue = 0x1000, .saturation = 20 };
        return c;
    }
    ColorControlCluster::Config XyConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kXy);
        c.mColorValue = XYColor{ .x = 1000, .y = 2000 };
        return c;
    }
    ColorControlCluster::Config CtConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorTemperature);
        c.mColorValue                         = CTColor{ .mireds = 250 };
        c.ctConfig.colorTempPhysicalMinMireds = 100;
        c.ctConfig.colorTempPhysicalMaxMireds = 400;
        return c;
    }
    ColorControlCluster::Config LoopConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorLoop).Set(Feature::kEnhancedHue).Set(Feature::kHueAndSaturation);
        c.mColorValue = EnhancedHueSatColor{ .enhancedHue = 0x1000, .saturation = 20 };
        return c;
    }
};

// ---------------------------------------------------------------------------- Saturation

TEST_F(TestColorControlCommands, MoveToSaturation)
{
    ColorControlCluster c(kEp, HsConfig());
    EXPECT_EQ(c.moveToSaturation(200, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.Saturation(), 200);

    // Out-of-range saturation rejected (max is 254).
    EXPECT_EQ(c.moveToSaturation(255, 10), Status::ConstraintError);
}

TEST_F(TestColorControlCommands, MoveSaturation)
{
    ColorControlCluster c(kEp, HsConfig()); // start sat 20
    EXPECT_EQ(c.moveSaturation(MoveModeEnum::kUp, 50), Status::Success);
    Complete(c);
    EXPECT_EQ(c.Saturation(), kMaxSat); // moves to the max bound

    EXPECT_EQ(c.moveSaturation(MoveModeEnum::kDown, 50), Status::Success);
    Complete(c);
    EXPECT_EQ(c.Saturation(), 0); // moves to the min bound

    // rate 0 on a non-stop move is invalid; Stop is accepted.
    EXPECT_EQ(c.moveSaturation(MoveModeEnum::kUp, 0), Status::ConstraintError);
    EXPECT_EQ(c.moveSaturation(MoveModeEnum::kStop, 0), Status::Success);
}

TEST_F(TestColorControlCommands, StepSaturation)
{
    ColorControlCluster c(kEp, HsConfig()); // start sat 20
    EXPECT_EQ(c.stepSaturation(StepModeEnum::kUp, 100, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.Saturation(), 120); // 20 + 100

    // Step up beyond max clamps to 254.
    EXPECT_EQ(c.stepSaturation(StepModeEnum::kUp, 250, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.Saturation(), kMaxSat);

    // A step size of 0 is rejected.
    EXPECT_EQ(c.stepSaturation(StepModeEnum::kUp, 0, 10), Status::InvalidCommand);
}

// ---------------------------------------------------------------------------- Hue (basic + enhanced)

TEST_F(TestColorControlCommands, MoveToHue)
{
    ColorControlCluster c(kEp, HsConfig()); // start hue 10 (enhanced 0x0A00)
    EXPECT_EQ(c.moveToHue(100, DirectionEnum::kShortest, 10, /*isEnhanced=*/false), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), static_cast<uint16_t>(100 << 8)); // CurrentHue == 100

    // Unknown direction rejected.
    EXPECT_EQ(c.moveToHue(100, DirectionEnum::kUnknownEnumValue, 10, false), Status::InvalidCommand);
}

TEST_F(TestColorControlCommands, EnhancedMoveToHue)
{
    ColorControlCluster c(kEp, EnhancedConfig()); // start enhancedHue 0x1000
    EXPECT_EQ(c.moveToHue(0x8000, DirectionEnum::kShortest, 10, /*isEnhanced=*/true), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), 0x8000); // full 16-bit target applied
}

TEST_F(TestColorControlCommands, StepHue)
{
    ColorControlCluster c(kEp, HsConfig()); // start enhancedHue 0x0A00
    EXPECT_EQ(c.stepHue(StepModeEnum::kUp, 20, 10, /*isEnhanced=*/false), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), static_cast<uint16_t>(0x0A00 + (20 << 8))); // += 20 in the high byte

    EXPECT_EQ(c.stepHue(StepModeEnum::kUnknownEnumValue, 20, 10, false), Status::InvalidCommand);
    // A step size of 0 is rejected.
    EXPECT_EQ(c.stepHue(StepModeEnum::kUp, 0, 10, false), Status::InvalidCommand);
}

TEST_F(TestColorControlCommands, MoveHueRateThenStop)
{
    ColorControlCluster c(kEp, HsConfig()); // enhancedHue 0x0A00
    // Rate move: 10 (8-bit) -> 10<<8 = 2560 hue-units/sec up. Indefinite (no end).
    EXPECT_EQ(c.moveHue(MoveModeEnum::kUp, 10, /*isEnhanced=*/false), Status::Success);
    Tick(c, 1000); // one second
    EXPECT_EQ(c.EnhancedHue(), static_cast<uint16_t>(0x0A00 + 2560));

    // Stop freezes the hue axis where it is.
    const uint16_t frozen = c.EnhancedHue();
    EXPECT_EQ(c.moveHue(MoveModeEnum::kStop, 0, false), Status::Success);
    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), frozen);
}

TEST_F(TestColorControlCommands, EnhancedMoveHueRate)
{
    ColorControlCluster c(kEp, EnhancedConfig()); // enhancedHue 0x1000
    // Enhanced rate is a full 16-bit hue-units/sec (no <<8 projection).
    EXPECT_EQ(c.moveHue(MoveModeEnum::kUp, 2560, /*isEnhanced=*/true), Status::Success);
    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), static_cast<uint16_t>(0x1000 + 2560));
}

TEST_F(TestColorControlCommands, EnhancedStepHue)
{
    ColorControlCluster c(kEp, EnhancedConfig()); // enhancedHue 0x1000
    // Enhanced step is a full 16-bit delta.
    EXPECT_EQ(c.stepHue(StepModeEnum::kUp, 0x2000, 10, /*isEnhanced=*/true), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), 0x3000);
}

// ---------------------------------------------------------------------------- Hue + Saturation

TEST_F(TestColorControlCommands, MoveToHueAndSaturation)
{
    ColorControlCluster c(kEp, HsConfig());
    EXPECT_EQ(c.moveToHueAndSaturation(100, 200, 10, /*isEnhanced=*/false), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), static_cast<uint16_t>(100 << 8));
    EXPECT_EQ(c.Saturation(), 200);
}

TEST_F(TestColorControlCommands, EnhancedMoveToHueAndSaturation)
{
    ColorControlCluster c(kEp, EnhancedConfig());
    EXPECT_EQ(c.moveToHueAndSaturation(0x4000, 150, 10, /*isEnhanced=*/true), Status::Success);
    Complete(c);
    EXPECT_EQ(c.EnhancedHue(), 0x4000);
    EXPECT_EQ(c.Saturation(), 150);
}

// transitionTime == 0 is an immediate jump, NOT an indefinite rate move: the hue must land on the
// target and stay put. Regression for durationMs == 0 being misread as a MoveHue rate move, which
// reinterpreted the arc as hue-units/sec so EnhancedCurrentHue kept climbing (and wrapping) past the
// target — surfacing in TC-CC-10.1 as a stored scene value above the polled range.
TEST_F(TestColorControlCommands, EnhancedMoveToHueAndSaturationImmediateDoesNotDrift)
{
    ColorControlCluster c(kEp, EnhancedConfig()); // start enhancedHue 0x1000
    EXPECT_EQ(c.moveToHueAndSaturation(20000, 50, /*transitionTimeDs=*/0, /*isEnhanced=*/true), Status::Success);
    Tick(c, 1); // first tick materializes the immediate move
    EXPECT_EQ(c.EnhancedHue(), 20000);
    EXPECT_EQ(c.Saturation(), 50);
    Tick(c, 2000); // seconds later it must not have moved
    EXPECT_EQ(c.EnhancedHue(), 20000);
}

// ---------------------------------------------------------------------------- Color (X/Y)

TEST_F(TestColorControlCommands, MoveToColor)
{
    ColorControlCluster c(kEp, XyConfig()); // start x=1000 y=2000
    EXPECT_EQ(c.moveToColor(30000, 40000, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.CurrentX(), 30000);
    EXPECT_EQ(c.CurrentY(), 40000);
}

TEST_F(TestColorControlCommands, StepColor)
{
    ColorControlCluster c(kEp, XyConfig()); // start x=1000 y=2000
    EXPECT_EQ(c.stepColor(500, -500, 10), Status::Success);
    Complete(c);
    EXPECT_EQ(c.CurrentX(), 1500);
    EXPECT_EQ(c.CurrentY(), 1500);

    // Both-zero step is invalid.
    EXPECT_EQ(c.stepColor(0, 0, 10), Status::InvalidCommand);
}

TEST_F(TestColorControlCommands, MoveColorRate)
{
    ColorControlCluster c(kEp, XyConfig());
    // Positive X rate -> moves toward the CIE max; negative Y -> toward 0. Rate is units/sec; pick a rate
    // large enough that both axes complete within Complete()'s advance window.
    EXPECT_EQ(c.moveColor(2000, -2000, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()), Status::Success);
    Complete(c);
    EXPECT_EQ(c.CurrentX(), kMaxCieXy);
    EXPECT_EQ(c.CurrentY(), 0);

    // Both-zero rate stops movement (no error).
    EXPECT_EQ(c.moveColor(0, 0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()), Status::Success);
}

// ---------------------------------------------------------------------------- Color temperature

TEST_F(TestColorControlCommands, MoveColorTemperature)
{
    ColorControlCluster c(kEp, CtConfig()); // start 250, physical [100,400]
    // Up increases mireds toward the (clamped) upper field limit; 0 field -> physical max.
    EXPECT_EQ(c.moveColorTemp(MoveModeEnum::kUp, 50, 0, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 400);

    EXPECT_EQ(c.moveColorTemp(MoveModeEnum::kDown, 50, 0, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 100);
}

TEST_F(TestColorControlCommands, StepColorTemperature)
{
    ColorControlCluster c(kEp, CtConfig()); // start 250
    EXPECT_EQ(c.stepColorTemp(StepModeEnum::kUp, 100, 10, 0, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 350); // 250 + 100

    // Step up beyond the physical max clamps to 400.
    EXPECT_EQ(c.stepColorTemp(StepModeEnum::kUp, 500, 10, 0, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 400);
}

// A valid moveToColorTemp target is CLAMPED to the configured physical min/max [100, 400].
TEST_F(TestColorControlCommands, MoveToColorTemperatureClampsToPhysicalRange)
{
    ColorControlCluster c(kEp, CtConfig()); // physical range [100, 400]

    // Above the physical max -> clamped down to it.
    ASSERT_EQ(c.moveToColorTemp(500, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 400u);

    // Below the physical min -> clamped up to it.
    ASSERT_EQ(c.moveToColorTemp(50, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 100u);

    // In-range value -> applied unchanged.
    ASSERT_EQ(c.moveToColorTemp(300, 0), Status::Success);
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 300u);
}

// ---------------------------------------------------------------------------- Mid-transition interpolation

TEST_F(TestColorControlCommands, TransitionInterpolatesOverTime)
{
    ColorControlCluster c(kEp, CtConfig()); // start 250
    // 200 ds = 20 s to reach 400.
    EXPECT_EQ(c.moveToColorTemp(400, 200), Status::Success);
    Tick(c, 10000); // halfway (10 s)
    const uint16_t mid = c.ColorTempMireds();
    EXPECT_GT(mid, 250);
    EXPECT_LT(mid, 400); // partway, not yet at target
    Complete(c);
    EXPECT_EQ(c.ColorTempMireds(), 400); // finishes exactly on target
}

// ---------------------------------------------------------------------------- ColorLoopSet

TEST_F(TestColorControlCommands, ColorLoopSetUpdatesAndActivates)
{
    ColorControlCluster c(kEp, LoopConfig());

    // Update time and direction (no action): the color-loop attributes change, loop stays inactive.
    EXPECT_EQ(c.colorLoopSet(BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateTime).Set(UpdateFlagsBitmap::kUpdateDirection),
                             ColorLoopActionEnum::kDeactivate, ColorLoopDirectionEnum::kIncrement, /*time=*/30,
                             /*startHue=*/0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);
    EXPECT_EQ(c.ColorLoopTime(), 30);
    EXPECT_EQ(c.ColorLoopDirection(), 1); // kIncrement
    EXPECT_EQ(c.ColorLoopActive(), 0);

    // Activate.
    EXPECT_EQ(c.colorLoopSet(BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateAction),
                             ColorLoopActionEnum::kActivateFromColorLoopStartEnhancedHue, ColorLoopDirectionEnum::kIncrement, 0, 0,
                             BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);
    EXPECT_EQ(c.ColorLoopActive(), 1);

    // Deactivate.
    EXPECT_EQ(c.colorLoopSet(BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateAction), ColorLoopActionEnum::kDeactivate,
                             ColorLoopDirectionEnum::kIncrement, 0, 0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);
    EXPECT_EQ(c.ColorLoopActive(), 0);

    // Unknown action is rejected.
    EXPECT_EQ(c.colorLoopSet(BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateAction), ColorLoopActionEnum::kUnknownEnumValue,
                             ColorLoopDirectionEnum::kIncrement, 0, 0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::InvalidCommand);
}

// ---------------------------------------------------------------------------- StopMoveStep

TEST_F(TestColorControlCommands, StopMoveStepFreezesTransition)
{
    ColorControlCluster c(kEp, CtConfig()); // start 250
    Testing::ClusterTester tester(c);

    EXPECT_EQ(c.moveToColorTemp(400, 200), Status::Success); // 20 s transition
    Tick(c, 10000);                                          // halfway
    const uint16_t frozen = c.ColorTempMireds();
    EXPECT_GT(frozen, 250);
    EXPECT_LT(frozen, 400);

    Commands::StopMoveStep::Type stop;
    stop.optionsMask.ClearAll();
    stop.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::StopMoveStep::Id, stop).IsSuccess());

    // After Stop the transition is cleared: further ticks do not advance the value.
    Tick(c, 60000);
    EXPECT_EQ(c.ColorTempMireds(), frozen);
}

// ---------------------------------------------------------------------------- Validation / RemainingTime consistency

// B4: moveToHueAndSaturation must apply the same constraint checks as its twin moveToSaturation.
TEST_F(TestColorControlCommands, MoveToHueAndSaturationRejectsOutOfRange)
{
    ColorControlCluster c(kEp, HsConfig());
    EXPECT_EQ(c.moveToHueAndSaturation(100, 255, 10, /*isEnhanced=*/false), Status::ConstraintError);    // sat > 254
    EXPECT_EQ(c.moveToHueAndSaturation(100, 200, 65535, /*isEnhanced=*/false), Status::ConstraintError); // time > 65534
    EXPECT_EQ(c.moveToHueAndSaturation(100, 200, 10, /*isEnhanced=*/false), Status::Success);            // valid
}

// B5: a fresh command sets RemainingTime immediately (the §3.2.7.4 edge report), before any tick.
TEST_F(TestColorControlCommands, FreshCommandSetsRemainingTime)
{
    {
        ColorControlCluster c(kEp, XyConfig());
        Testing::ClusterTester tester(c);
        uint16_t rt = 0;

        EXPECT_EQ(c.stepColor(500, -500, 30), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 30);

        // moveColor is rate-based with a computed time-to-boundary → finite and nonzero.
        EXPECT_EQ(c.moveColor(2000, -2000), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_GT(rt, 0);
    }
    {
        ColorControlCluster c(kEp, CtConfig()); // start 250, phys [100,400]
        Testing::ClusterTester tester(c);
        uint16_t rt = 0;

        EXPECT_EQ(c.moveColorTemp(MoveModeEnum::kUp, 50, 0, 0), Status::Success); // 150 mireds @ 50/s = 3 s = 30 ds
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 30);

        EXPECT_EQ(c.stepColorTemp(StepModeEnum::kUp, 20, 15, 0, 0), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 15);
    }
}

// B6: every Stop path drives RemainingTime back to 0.
TEST_F(TestColorControlCommands, StopResetsRemainingTime)
{
    // Color-temperature Stop.
    {
        ColorControlCluster c(kEp, CtConfig());
        Testing::ClusterTester tester(c);
        uint16_t rt = 0;
        EXPECT_EQ(c.moveColorTemp(MoveModeEnum::kUp, 50, 0, 0), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_GT(rt, 0);
        EXPECT_EQ(c.moveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 0);
    }
    // MoveColor both-zero Stop.
    {
        ColorControlCluster c(kEp, XyConfig());
        Testing::ClusterTester tester(c);
        uint16_t rt = 0;
        EXPECT_EQ(c.moveColor(2000, -2000), Status::Success);
        EXPECT_EQ(c.moveColor(0, 0), Status::Success); // stop
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 0);
    }
    // MoveSaturation Stop.
    {
        ColorControlCluster c(kEp, HsConfig());
        Testing::ClusterTester tester(c);
        uint16_t rt = 0;
        EXPECT_EQ(c.moveSaturation(MoveModeEnum::kUp, 50), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_GT(rt, 0);
        EXPECT_EQ(c.moveSaturation(MoveModeEnum::kStop, 0), Status::Success);
        ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, rt).IsSuccess());
        EXPECT_EQ(rt, 0);
    }
}

// ---------------------------------------------------------------------------- Color-loop tick / interpolation

// The loop is an autonomous enhanced-hue driver: EnhancedCurrentHue = (startHue + rate*elapsed) mod 65536,
// where one full 65536-unit revolution takes ColorLoopTime seconds. Position is derived from the wall-clock
// anchor stamped at activation (drift-free), so every tick recomputes from absolute elapsed time.
TEST_F(TestColorControlCommands, ColorLoopTickAdvancesEnhancedHueUp)
{
    ColorControlCluster c(kEp, LoopConfig()); // enhancedHue 0x1000 (4096), enhanced-HS mode
    const auto flags = BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateTime)
                           .Set(UpdateFlagsBitmap::kUpdateDirection)
                           .Set(UpdateFlagsBitmap::kUpdateAction);
    // 10 s / revolution, increment (up), starting from the current enhanced hue.
    EXPECT_EQ(c.colorLoopSet(flags, ColorLoopActionEnum::kActivateFromEnhancedCurrentHue, ColorLoopDirectionEnum::kIncrement,
                             /*time=*/10, /*startHue=*/0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);
    ASSERT_EQ(c.ColorLoopActive(), 1);
    EXPECT_EQ(c.EnhancedHue(), 0x1000); // no tick yet → still the anchor

    // After 1 s: traveled = 65536 * 1000 / (10 * 1000) = 6553 → 4096 + 6553 = 10649.
    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), 10649u);

    // After a further 1 s (2 s total from the anchor): 65536 * 2000 / 10000 = 13107 → 4096 + 13107 = 17203.
    // Proves the position comes from absolute elapsed time, not per-tick accumulation.
    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), 17203u);
}

TEST_F(TestColorControlCommands, ColorLoopTickDecrementWrapsAroundZero)
{
    ColorControlCluster c(kEp, LoopConfig()); // enhancedHue 0x1000 (4096)
    const auto flags = BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateTime)
                           .Set(UpdateFlagsBitmap::kUpdateDirection)
                           .Set(UpdateFlagsBitmap::kUpdateAction);
    EXPECT_EQ(c.colorLoopSet(flags, ColorLoopActionEnum::kActivateFromEnhancedCurrentHue, ColorLoopDirectionEnum::kDecrement,
                             /*time=*/10, /*startHue=*/0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);

    // After 1 s decrementing: 4096 - 6553 = -2457 → wraps to 65536 - 2457 = 63079.
    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), 63079u);
}

// A color loop owns the hue axis and nothing but Deactivate stops it: an ordinary hue command issued while
// it runs is a no-op and the loop keeps advancing.
TEST_F(TestColorControlCommands, ColorLoopKeepsRunningThroughHueCommand)
{
    ColorControlCluster c(kEp, LoopConfig());
    const auto flags = BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateTime)
                           .Set(UpdateFlagsBitmap::kUpdateDirection)
                           .Set(UpdateFlagsBitmap::kUpdateAction);
    EXPECT_EQ(c.colorLoopSet(flags, ColorLoopActionEnum::kActivateFromEnhancedCurrentHue, ColorLoopDirectionEnum::kIncrement, 10, 0,
                             BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);

    // moveToHue must not disturb the loop (§3.2.8.1): returns Success but changes nothing.
    EXPECT_EQ(c.moveToHue(0x8000, DirectionEnum::kShortest, 10, /*isEnhanced=*/true), Status::Success);
    EXPECT_EQ(c.ColorLoopActive(), 1);

    Tick(c, 1000);
    EXPECT_EQ(c.EnhancedHue(), 10649u); // still following the loop, not the (ignored) moveToHue target
}

} // namespace
