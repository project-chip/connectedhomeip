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

// Persistence round-trips + color-loop resume.
//
// The NVM-flagged attributes (EnhancedColorMode, CurrentX/Y, CurrentHue, CurrentSaturation,
// EnhancedCurrentHue, ColorTemperatureMireds, ColorLoop{Active,Direction,Time}, StartUpColorTemperatureMireds)
// must survive a reboot. Each test drives a first cluster ("a") that Startup()s against a storage-backed
// context and settles some state, then constructs a SECOND cluster ("b") on the SAME storage and asserts
// b.Startup() restores it. A single ClusterTester owns the shared context; b is started with that same
// context, exactly as a reboot would re-open the same KVS. Transitions are clock-driven (RAIIMockClock),
// so persistence fires from OnTick's settle path just as it would on a device.

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
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

struct TestColorControlPersistence : public ::testing::Test
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

    ColorControlCluster::Config CtConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorTemperature);
        c.mColorValue                         = CTColor{ .mireds = 250 };
        c.ctConfig.colorTempPhysicalMinMireds = 100;
        c.ctConfig.colorTempPhysicalMaxMireds = 400;
        return c;
    }
    ColorControlCluster::Config XyConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kXy);
        c.mColorValue = XYColor{ .x = 1000, .y = 2000 };
        return c;
    }
    ColorControlCluster::Config HsConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kHueAndSaturation);
        c.mColorValue = HueSatColor{ .hue = 10, .saturation = 20 };
        return c;
    }
    ColorControlCluster::Config LoopConfig()
    {
        ColorControlCluster::Config c(delegate);
        c.mFeatures.Set(Feature::kColorLoop).Set(Feature::kEnhancedHue).Set(Feature::kHueAndSaturation);
        c.mColorValue = EnhancedHueSatColor{ .enhancedHue = 0x1000, .saturation = 20 };
        return c;
    }

    // Drive a first cluster to a settled color (`drive` issues the command, which OnTick then persists),
    // reboot a second cluster on the SAME storage, and hand it to `verify` to assert the restored state.
    // Both clusters share one config and one storage-backed context, exactly as a reboot re-opens the KVS.
    template <typename DriveFn, typename VerifyFn>
    void ExpectStateSurvivesReboot(const ColorControlCluster::Config & config, DriveFn drive,
                                   EnhancedColorModeEnum expectedMode, VerifyFn verify)
    {
        ColorControlCluster a(kEp, config);
        Testing::ClusterTester tester(a);
        ASSERT_EQ(a.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        ASSERT_EQ(drive(a), Status::Success);
        Complete(a); // settles the target -> OnTick persists it

        ColorControlCluster b(kEp, config);
        ASSERT_EQ(b.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
        EXPECT_EQ(b.GetEnhancedColorMode(), expectedMode);
        verify(b);
    }
};

TEST_F(TestColorControlPersistence, ColorTemperatureSurvivesReboot)
{
    // Default config value is 250, but the persisted 350 must win after reboot.
    ExpectStateSurvivesReboot(
        CtConfig(), [](ColorControlCluster & c) { return c.moveToColorTemp(350, 10); },
        EnhancedColorModeEnum::kColorTemperatureMireds,
        [](ColorControlCluster & b) { EXPECT_EQ(b.ColorTempMireds(), 350u); });
}

TEST_F(TestColorControlPersistence, XyColorSurvivesReboot)
{
    ExpectStateSurvivesReboot(
        XyConfig(), [](ColorControlCluster & c) { return c.moveToColor(30000, 40000, 10); },
        EnhancedColorModeEnum::kCurrentXAndCurrentY, [](ColorControlCluster & b) {
            EXPECT_EQ(b.CurrentX(), 30000u);
            EXPECT_EQ(b.CurrentY(), 40000u);
        });
}

TEST_F(TestColorControlPersistence, HueSaturationSurvivesReboot)
{
    ExpectStateSurvivesReboot(
        HsConfig(),
        [](ColorControlCluster & c) { return c.moveToHueAndSaturation(100, 200, 10, /*isEnhanced=*/false); },
        EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation, [](ColorControlCluster & b) {
            EXPECT_EQ(b.EnhancedHue(), static_cast<uint16_t>(100 << 8)); // CurrentHue == 100
            EXPECT_EQ(b.Saturation(), 200u);
        });
}

// A Stop mid-transition freezes the color at its current value; that frozen value must persist even
// though it is not a natural transition end (RemainingTime / the transition itself are RAM-only).
TEST_F(TestColorControlPersistence, StopFreezesAndPersistsCurrentValue)
{
    ColorControlCluster a(kEp, CtConfig());
    Testing::ClusterTester tester(a);
    ASSERT_EQ(a.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(a.moveToColorTemp(400, 200), Status::Success); // 20 s transition
    Tick(a, 10000);                                          // halfway
    const uint16_t frozen = a.ColorTempMireds();
    EXPECT_GT(frozen, 250u);
    EXPECT_LT(frozen, 400u);

    EXPECT_EQ(a.moveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success); // freeze + persist

    ColorControlCluster b(kEp, CtConfig());
    ASSERT_EQ(b.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(b.ColorTempMireds(), frozen);
}

TEST_F(TestColorControlPersistence, StartUpColorTemperatureWriteSurvivesReboot)
{
    ColorControlCluster a(kEp, CtConfig());
    Testing::ClusterTester tester(a);
    ASSERT_EQ(a.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(tester.WriteAttribute(Attributes::StartUpColorTemperatureMireds::Id, DataModel::MakeNullable<uint16_t>(320))
                    .IsSuccess());

    // Reboot: b's config leaves StartUpColorTemperatureMireds null, so the only way it lands on 320 is
    // by loading the persisted write and applying it (§3.2.11.10 forces CT mode to the startup value).
    ColorControlCluster b(kEp, CtConfig());
    ASSERT_EQ(b.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(b.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(b.ColorTempMireds(), 320u);
}

// Issue 3: a color loop running at shutdown resumes on the next boot instead of lying dormant.
TEST_F(TestColorControlPersistence, ColorLoopResumesAfterReboot)
{
    ColorControlCluster a(kEp, LoopConfig());
    Testing::ClusterTester tester(a);
    ASSERT_EQ(a.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Activate the loop from the current enhanced hue, incrementing, one revolution / 30 s.
    const BitMask<UpdateFlagsBitmap> flags = BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateTime)
                                                    .Set(UpdateFlagsBitmap::kUpdateDirection)
                                                    .Set(UpdateFlagsBitmap::kUpdateAction);
    EXPECT_EQ(a.ColorLoopSet(flags, ColorLoopActionEnum::kActivateFromEnhancedCurrentHue, ColorLoopDirectionEnum::kIncrement,
                             /*time=*/30, /*startHue=*/0, BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);
    ASSERT_EQ(a.ColorLoopActive(), 1);

    // Reboot on the same storage.
    ColorControlCluster b(kEp, LoopConfig());
    ASSERT_EQ(b.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Loop attributes restored...
    EXPECT_EQ(b.ColorLoopActive(), 1);
    EXPECT_EQ(b.ColorLoopTime(), 30u);
    EXPECT_EQ(b.ColorLoopDirection(), 1u); // kIncrement
    ASSERT_EQ(b.GetEnhancedColorMode(), EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);

    // ...and it is DRIVING again (not dormant): advancing the clock moves EnhancedCurrentHue.
    const uint16_t before = b.EnhancedHue();
    Tick(b, 1000);
    EXPECT_NE(b.EnhancedHue(), before);
}

// A loop that is active but dormant (CT owns the output via StartUpColorTemperatureMireds) must NOT be
// re-armed as driving on boot — LoopIsDriving() is false, so EnhancedCurrentHue stays put.
TEST_F(TestColorControlPersistence, DormantColorLoopDoesNotResumeUnderStartupCt)
{
    ColorControlCluster a(kEp, LoopConfig());
    Testing::ClusterTester tester(a);
    ASSERT_EQ(a.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(a.ColorLoopSet(BitMask<UpdateFlagsBitmap>(UpdateFlagsBitmap::kUpdateAction),
                             ColorLoopActionEnum::kActivateFromEnhancedCurrentHue, ColorLoopDirectionEnum::kIncrement, 0, 0,
                             BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()),
              Status::Success);

    // Reboot with a StartUpColorTemperatureMireds set: §3.2.11.10 forces CT mode, so the loop is dormant.
    ColorControlCluster::Config bcfg = LoopConfig();
    bcfg.mFeatures.Set(Feature::kColorTemperature);
    bcfg.ctConfig.colorTempPhysicalMinMireds = 100;
    bcfg.ctConfig.colorTempPhysicalMaxMireds = 400;
    bcfg.ctConfig.startUpColorTemperatureMireds.SetNonNull(300);
    ColorControlCluster b(kEp, bcfg);
    ASSERT_EQ(b.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(b.ColorLoopActive(), 1);                                              // still active...
    EXPECT_EQ(b.GetEnhancedColorMode(), EnhancedColorModeEnum::kColorTemperatureMireds); // ...but CT owns the output
    EXPECT_EQ(b.ColorTempMireds(), 300u);
}

} // namespace
