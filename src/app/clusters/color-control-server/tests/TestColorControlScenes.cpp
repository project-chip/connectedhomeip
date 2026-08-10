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

// ColorControl scene-handler tests. ColorControlCluster IS its own scene handler (it inherits
// scenes::DefaultSceneHandlerImpl); the application registers it in the endpoint's scene table
// (table->RegisterHandler(&cluster)). These exercise the handler directly:
//   * SupportsCluster's own-endpoint scoping (no registry, no cross-endpoint resolution),
//   * SerializeSave by decoding the EFS bytes (verifies exactly what is captured, no transition), and
//   * ApplyScene by building an EFS and draining the resulting transition.
// The live color is always driven with a command first (never relied upon through Startup, which rebuilds
// it from persistence), and asserted only after the transition is completed (CurrentX/Y read the stored
// value, which the tick materializes).

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/scenes-server/ScenesIntegrationDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TypeTraits.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status             = Protocols::InteractionModel::Status;
using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

// Counts the scene-invalidation calls the cluster makes. The other three methods are only reached
// through the Scenes cluster's own command paths, which ColorControl never drives.
class CountingScenesIntegrationDelegate : public chip::scenes::ScenesIntegrationDelegate
{
public:
    CHIP_ERROR MakeSceneInvalidForAllFabrics() override
    {
        mInvalidateCount++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR StoreCurrentGlobalScene(FabricIndex) override { return CHIP_NO_ERROR; }
    CHIP_ERROR RecallGlobalScene(FabricIndex) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GroupWillBeRemoved(FabricIndex, GroupId) override { return CHIP_NO_ERROR; }

    uint32_t mInvalidateCount = 0;
};

constexpr EndpointId kTestEndpointId = 1;

struct TestColorControlScenes : public ::testing::Test
{
    // ApplyScene arms the tick via DeviceLayer::SystemLayer().StartTimer(); InitChipStack() brings the layer
    // up so HandleApplyScene's ArmTick() does not fail. The work queue starts suspended, so the armed timer
    // never fires on a background thread and cannot race the manual OnTick() calls below.
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        DeviceLayer::PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    ColorControlDelegate delegate;
    System::Clock::Internal::RAIIMockClock clock;

    // Advance the mock clock well past any transition used here and run one tick, so the transition
    // materializes into the stored color (which CurrentX/Y read).
    void Complete(ColorControlCluster & c)
    {
        clock.AdvanceMonotonic(System::Clock::Milliseconds64(120000));
        c.OnTick();
    }
};

// SupportsCluster is scoped to this cluster's own endpoint and cluster id — no registry, no
// cross-endpoint resolution.
TEST_F(TestColorControlScenes, SupportsClusterMatchesOwnEndpointOnly)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy);
    ColorControlCluster cluster(kTestEndpointId, config);

    EXPECT_TRUE(cluster.SupportsCluster(kTestEndpointId, ColorControl::Id));
    EXPECT_FALSE(cluster.SupportsCluster(static_cast<EndpointId>(kTestEndpointId + 1), ColorControl::Id));
    EXPECT_FALSE(cluster.SupportsCluster(kTestEndpointId, static_cast<ClusterId>(ColorControl::Id + 1)));
}

// SerializeSave captures the cluster's live color. Verified by decoding the EFS bytes directly (no
// transition involved): the saved pairs must be exactly the live XY value plus the mandatory
// EnhancedColorMode.
TEST_F(TestColorControlScenes, SerializeSaveCapturesLiveColor)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy);
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Drive the live color to a known value (immediate move), independent of what Startup restored.
    EXPECT_EQ(cluster.MoveToColor(30000, 40000, 0), Status::Success);
    Complete(cluster);
    ASSERT_EQ(cluster.CurrentX(), 30000u);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(cluster.SerializeSave(kTestEndpointId, ColorControl::Id, serializedBytes), CHIP_NO_ERROR);

    DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> list;
    EXPECT_EQ(cluster.DecodeAttributeValueList(serializedBytes, list), CHIP_NO_ERROR);

    bool sawX    = false;
    bool sawY    = false;
    bool sawMode = false;
    auto it      = list.begin();
    while (it.Next())
    {
        const auto & p = it.GetValue();
        if (p.attributeID == Attributes::CurrentX::Id)
        {
            EXPECT_EQ(p.valueUnsigned16.Value(), 30000u);
            sawX = true;
        }
        else if (p.attributeID == Attributes::CurrentY::Id)
        {
            EXPECT_EQ(p.valueUnsigned16.Value(), 40000u);
            sawY = true;
        }
        else if (p.attributeID == Attributes::EnhancedColorMode::Id)
        {
            EXPECT_EQ(p.valueUnsigned8.Value(), to_underlying(EnhancedColorModeEnum::kCurrentXAndCurrentY));
            sawMode = true;
        }
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_TRUE(sawX);
    EXPECT_TRUE(sawY);
    EXPECT_TRUE(sawMode);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// ApplyScene decodes an EFS and drives the live color to the scene's target. The EFS is built explicitly,
// and the resulting transition is drained before asserting the final color.
TEST_F(TestColorControlScenes, ApplySceneDrivesColorToSavedTarget)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy);
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Build a scene EFS targeting (100, 200) in XY mode.
    AttributeValuePair pairs[3];
    pairs[0].attributeID = Attributes::CurrentX::Id;
    pairs[0].valueUnsigned16.SetValue(100);
    pairs[1].attributeID = Attributes::CurrentY::Id;
    pairs[1].valueUnsigned16.SetValue(200);
    pairs[2].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[2].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kCurrentXAndCurrentY));
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    // Move the live color away from the scene target first, so ApplyScene has something to change.
    EXPECT_EQ(cluster.MoveToColor(30000, 40000, 0), Status::Success);
    Complete(cluster);
    ASSERT_EQ(cluster.CurrentX(), 30000u);

    // Apply the scene and drain the transition; the live color reaches the saved target.
    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_NO_ERROR);
    Complete(cluster);
    EXPECT_EQ(cluster.CurrentX(), 100u);
    EXPECT_EQ(cluster.CurrentY(), 200u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A scene's transition time is a uint32 of milliseconds (AddScene constrains it to 60000000), so the
// RemainingTime it implies does not fit in uint16 deciseconds: it must saturate at the attribute's
// constraint max (0xFFFE) rather than wrap — 600000 tenths would come back as 10176.
TEST_F(TestColorControlScenes, ApplySceneSaturatesRemainingTime)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy);
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    AttributeValuePair pairs[3];
    pairs[0].attributeID = Attributes::CurrentX::Id;
    pairs[0].valueUnsigned16.SetValue(100);
    pairs[1].attributeID = Attributes::CurrentY::Id;
    pairs[1].valueUnsigned16.SetValue(200);
    pairs[2].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[2].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kCurrentXAndCurrentY));
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    ASSERT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 60000000), CHIP_NO_ERROR);

    uint16_t remainingTime = 0;
    ASSERT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 0xFFFE);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A scene may carry ColorLoopActive == 1 on an endpoint that has no ColorLoop feature — nothing upstream
// rejects the pair (the per-pair validator only type-checks it, and SerializeAdd only checks that the
// declared mode's companion attributes are present). Restoring it must not start a loop, which would
// switch the endpoint into enhanced-hue mode and report loop attributes it does not have; the ordinary
// color restore runs instead.
TEST_F(TestColorControlScenes, ApplySceneIgnoresColorLoopWithoutFeature)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy); // no ColorLoop, no EnhancedHue
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    AttributeValuePair pairs[4];
    pairs[0].attributeID = Attributes::CurrentX::Id;
    pairs[0].valueUnsigned16.SetValue(100);
    pairs[1].attributeID = Attributes::CurrentY::Id;
    pairs[1].valueUnsigned16.SetValue(200);
    pairs[2].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[2].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kCurrentXAndCurrentY));
    pairs[3].attributeID = Attributes::ColorLoopActive::Id;
    pairs[3].valueUnsigned8.SetValue(1);
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    ASSERT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    // Move the live color away from the scene target so the restore has something to change.
    EXPECT_EQ(cluster.MoveToColor(30000, 40000, 0), Status::Success);
    Complete(cluster);
    ASSERT_EQ(cluster.CurrentX(), 30000u);

    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_NO_ERROR);
    Complete(cluster);

    EXPECT_EQ(cluster.ColorLoopActive(), 0);
    EXPECT_EQ(cluster.GetEnhancedColorMode(), EnhancedColorModeEnum::kCurrentXAndCurrentY);
    EXPECT_EQ(cluster.CurrentX(), 100u); // saved color restored rather than discarded by a loop start
    EXPECT_EQ(cluster.CurrentY(), 200u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// The counterpart: on an endpoint that does support ColorLoop, a scene saved with ColorLoopActive == 1
// starts the loop, which takes over the hue axis (so the scene's saved color axes are deliberately not
// restored — the loop drives hue itself).
TEST_F(TestColorControlScenes, ApplySceneStartsColorLoopWhenSupported)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorLoop).Set(Feature::kEnhancedHue).Set(Feature::kHueAndSaturation);
    config.mColorValue = EnhancedHueSatColor{ .enhancedHue = 0x1000, .saturation = 20 };
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    AttributeValuePair pairs[4];
    pairs[0].attributeID = Attributes::EnhancedCurrentHue::Id;
    pairs[0].valueUnsigned16.SetValue(0x4000);
    pairs[1].attributeID = Attributes::CurrentSaturation::Id;
    pairs[1].valueUnsigned8.SetValue(200);
    pairs[2].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[2].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation));
    pairs[3].attributeID = Attributes::ColorLoopActive::Id;
    pairs[3].valueUnsigned8.SetValue(1);
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    ASSERT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.ColorLoopActive(), 1);
    EXPECT_EQ(cluster.GetEnhancedColorMode(), EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// SerializeAdd only accepts the decodable form of an EFS, so a Type is TLV-encoded into `backing` and
// decoded back into `out`. `backing` must outlive every use of `out` — the decoded list iterates over
// those bytes. Builds an EFS scoped to the ColorControl cluster from the given attribute/value pairs.
CHIP_ERROR MakeDecodableEfs(chip::Span<const AttributeValuePair> pairs, MutableByteSpan & backing,
                            ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType & out)
{
    ScenesManagement::Structs::ExtensionFieldSetStruct::Type in;
    in.clusterID          = ColorControl::Id;
    in.attributeValueList = DataModel::List<const AttributeValuePair>(pairs.data(), pairs.size());

    TLV::TLVWriter writer;
    writer.Init(backing);
    ReturnErrorOnFailure(in.Encode(writer, TLV::AnonymousTag()));
    ReturnErrorOnFailure(writer.Finalize());
    backing.reduce_size(writer.GetLengthWritten());

    TLV::TLVReader reader;
    reader.Init(backing);
    ReturnErrorOnFailure(reader.Next());
    return out.Decode(reader);
}

// §3.2.7.1.1: at AddScene, an EFS whose declared EnhancedColorMode is missing a companion attribute the mode
// requires is rejected (a presence check across the whole EFS, done before delegating to the base handler).
TEST_F(TestColorControlScenes, SerializeAddRejectsModeMissingRequiredAttributes)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorTemperature).Set(Feature::kXy).Set(Feature::kHueAndSaturation).Set(Feature::kEnhancedHue);
    ColorControlCluster cluster(kTestEndpointId, config);

    // Each case declares a mode but omits one attribute that mode requires → CHIP_ERROR_INVALID_ARGUMENT.
    struct Case
    {
        const char * name;
        EnhancedColorModeEnum mode;
        AttributeId presentButInsufficient; // the lone companion supplied (the other required one is missing)
        bool present16;                     // the supplied companion is uint16 (else uint8)
    };
    const Case cases[] = {
        { "CT without ColorTemperatureMireds", EnhancedColorModeEnum::kColorTemperatureMireds, Attributes::CurrentX::Id, true },
        { "XY with X but no Y", EnhancedColorModeEnum::kCurrentXAndCurrentY, Attributes::CurrentX::Id, true },
        { "HS with hue but no saturation", EnhancedColorModeEnum::kCurrentHueAndCurrentSaturation, Attributes::CurrentHue::Id,
          false },
        { "EnhancedHS with saturation but no EnhancedCurrentHue", EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation,
          Attributes::CurrentSaturation::Id, false },
    };

    for (const auto & c : cases)
    {
        AttributeValuePair pairs[2];
        pairs[0].attributeID = Attributes::EnhancedColorMode::Id;
        pairs[0].valueUnsigned8.SetValue(to_underlying(c.mode));
        pairs[1].attributeID = c.presentButInsufficient;
        if (c.present16)
        {
            pairs[1].valueUnsigned16.SetValue(1);
        }
        else
        {
            pairs[1].valueUnsigned8.SetValue(1);
        }

        uint8_t backing[128];
        MutableByteSpan backingSpan(backing);
        ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType efs;
        ASSERT_EQ(MakeDecodableEfs(chip::Span<const AttributeValuePair>(pairs), backingSpan, efs), CHIP_NO_ERROR) << c.name;

        uint8_t out[128];
        MutableByteSpan outSpan(out);
        EXPECT_EQ(cluster.SerializeAdd(kTestEndpointId, efs, outSpan), CHIP_ERROR_INVALID_ARGUMENT) << c.name;
    }
}

// ColorLoopDirection is constrained to 0..1 (kDecrement/kIncrement), so AddScene must reject a scene
// carrying an out-of-range direction. Without this bound the value would be stored verbatim and later
// written straight into the attribute by ApplyScene, making a read report a value the type forbids.
TEST_F(TestColorControlScenes, SerializeAddRejectsOutOfRangeColorLoopDirection)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorLoop).Set(Feature::kHueAndSaturation).Set(Feature::kEnhancedHue);
    ColorControlCluster cluster(kTestEndpointId, config);

    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::ColorLoopDirection::Id;

    // kIncrement (1) is the largest legal value; anything above it is out of range.
    for (uint8_t direction : { to_underlying(ColorLoopDirectionEnum::kDecrement), to_underlying(ColorLoopDirectionEnum::kIncrement),
                               static_cast<uint8_t>(to_underlying(ColorLoopDirectionEnum::kIncrement) + 1), uint8_t{ 0xFF } })
    {
        pairs[0].valueUnsigned8.SetValue(direction);

        uint8_t backing[128];
        MutableByteSpan backingSpan(backing);
        ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType efs;
        ASSERT_EQ(MakeDecodableEfs(chip::Span<const AttributeValuePair>(pairs), backingSpan, efs), CHIP_NO_ERROR)
            << "direction " << static_cast<int>(direction);

        uint8_t out[128];
        MutableByteSpan outSpan(out);
        const CHIP_ERROR expected =
            (direction <= to_underlying(ColorLoopDirectionEnum::kIncrement)) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ARGUMENT;
        EXPECT_EQ(cluster.SerializeAdd(kTestEndpointId, efs, outSpan), expected) << "direction " << static_cast<int>(direction);
    }
}

// ApplyScene rejects an EnhancedColorMode value outside the defined enum range (0..3): the decode loop
// caps the mode before building any target.
TEST_F(TestColorControlScenes, ApplySceneRejectsOutOfRangeMode)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy);
    ColorControlCluster cluster(kTestEndpointId, config);

    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[0].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kEnhancedCurrentHueAndCurrentSaturation) + 1); // = 4
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    ASSERT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_ERROR_INVALID_ARGUMENT);
}

// ApplyScene rejects a valid mode the device does not support: an XY-only cluster cannot restore a
// color-temperature scene (SupportsMode is false).
TEST_F(TestColorControlScenes, ApplySceneRejectsUnsupportedMode)
{
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kXy); // no color-temperature feature
    ColorControlCluster cluster(kTestEndpointId, config);

    AttributeValuePair pairs[2];
    pairs[0].attributeID = Attributes::ColorTemperatureMireds::Id;
    pairs[0].valueUnsigned16.SetValue(250);
    pairs[1].attributeID = Attributes::EnhancedColorMode::Id;
    pairs[1].valueUnsigned8.SetValue(to_underlying(EnhancedColorModeEnum::kColorTemperatureMireds));
    DataModel::List<AttributeValuePair> list(pairs);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    ASSERT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_ERROR_INVALID_ARGUMENT);
}

// ── Stop paths invalidate scenes ────────────────────────────────────────────────────────────────
// A Stop freezes the output part-way through a transition, so the live color no longer matches any
// stored scene and SceneValid must not keep claiming it does. RecallScene marks the scene valid while
// the transition it started is still in flight, so a Stop landing mid-flight is the case that would
// otherwise leave SceneValid set on a color the device never reached.
//
// The counter is read as a delta around the Stop: the command that starts each transition invalidates
// too, and that call is not what these assert.

struct TestColorControlStopInvalidatesScenes : public TestColorControlScenes
{
    CountingScenesIntegrationDelegate scenes;

    // Every stop path here is exercised on a cluster wired to the counting delegate.
    ColorControlCluster::Config MakeConfig()
    {
        ColorControlCluster::Config config(delegate);
        config.scenesIntegrationDelegate = &scenes;
        config.mFeatures.Set(Feature::kXy).Set(Feature::kHueAndSaturation).Set(Feature::kColorTemperature);
        config.ctConfig.colorTempPhysicalMinMireds = 100;
        config.ctConfig.colorTempPhysicalMaxMireds = 400;
        return config;
    }

    // Runs one tick partway into the movement so the Stop lands mid-transition rather than on an
    // already-settled one.
    void AdvancePartway(ColorControlCluster & c)
    {
        clock.AdvanceMonotonic(System::Clock::Milliseconds64(500));
        c.OnTick();
    }
};

TEST_F(TestColorControlStopInvalidatesScenes, MoveHueStopInvalidatesWhenItFreezesATransition)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveHue(MoveModeEnum::kUp, 10, /*isEnhanced=*/false), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveHue(MoveModeEnum::kStop, 0, /*isEnhanced=*/false), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

TEST_F(TestColorControlStopInvalidatesScenes, MoveSaturationStopInvalidatesWhenItFreezesATransition)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveSaturation(MoveModeEnum::kUp, 10), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveSaturation(MoveModeEnum::kStop, 0), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

TEST_F(TestColorControlStopInvalidatesScenes, MoveColorWithBothRatesZeroInvalidatesWhenItFreezesATransition)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveColor(100, 100), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveColor(0, 0), Status::Success); // both rates zero == stop
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

TEST_F(TestColorControlStopInvalidatesScenes, MoveColorTempStopInvalidatesWhenItFreezesATransition)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveColorTemp(MoveModeEnum::kUp, 10, 0, 0), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

TEST_F(TestColorControlStopInvalidatesScenes, StopMoveStepInvalidatesWhenItFreezesATransition)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveToColor(30000, 40000, 100), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.StopMoveStep(), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

// The other half of the contract: a Stop with nothing running moved nothing, so it must not invalidate.
// Otherwise a repeated Stop would emit a FabricSceneInfo report per fabric for a no-op.
TEST_F(TestColorControlStopInvalidatesScenes, IdleStopDoesNotInvalidate)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveHue(MoveModeEnum::kStop, 0, /*isEnhanced=*/false), Status::Success);
    EXPECT_EQ(cluster.MoveSaturation(MoveModeEnum::kStop, 0), Status::Success);
    EXPECT_EQ(cluster.MoveColor(0, 0), Status::Success);
    EXPECT_EQ(cluster.MoveColorTemp(MoveModeEnum::kStop, 0, 0, 0), Status::Success);
    EXPECT_EQ(cluster.StopMoveStep(), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before);
}

// A second Stop has nothing left to freeze, so only the first one invalidates.
TEST_F(TestColorControlStopInvalidatesScenes, RepeatedStopInvalidatesOnlyOnce)
{
    ColorControlCluster::Config config = MakeConfig();
    ColorControlCluster cluster(kTestEndpointId, config);
    Testing::ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.MoveHue(MoveModeEnum::kUp, 10, /*isEnhanced=*/false), Status::Success);
    AdvancePartway(cluster);

    const uint32_t before = scenes.mInvalidateCount;
    EXPECT_EQ(cluster.MoveHue(MoveModeEnum::kStop, 0, /*isEnhanced=*/false), Status::Success);
    EXPECT_EQ(cluster.MoveHue(MoveModeEnum::kStop, 0, /*isEnhanced=*/false), Status::Success);
    EXPECT_EQ(scenes.mInvalidateCount, before + 1);
}

} // namespace
