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
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TypeTraits.h> // chip::to_underlying
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
    EXPECT_EQ(cluster.moveToColor(30000, 40000, 0), Status::Success);
    Complete(cluster);
    ASSERT_EQ(cluster.CurrentX(), 30000u);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(cluster.SerializeSave(kTestEndpointId, ColorControl::Id, serializedBytes), CHIP_NO_ERROR);

    DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> list;
    EXPECT_EQ(cluster.DecodeAttributeValueList(serializedBytes, list), CHIP_NO_ERROR);

    bool sawX = false, sawY = false, sawMode = false;
    auto it = list.begin();
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
    EXPECT_EQ(cluster.moveToColor(30000, 40000, 0), Status::Success);
    Complete(cluster);
    ASSERT_EQ(cluster.CurrentX(), 30000u);

    // Apply the scene and drain the transition; the live color reaches the saved target.
    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, ColorControl::Id, serializedBytes, 0), CHIP_NO_ERROR);
    Complete(cluster);
    EXPECT_EQ(cluster.CurrentX(), 100u);
    EXPECT_EQ(cluster.CurrentY(), 200u);
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

} // namespace
