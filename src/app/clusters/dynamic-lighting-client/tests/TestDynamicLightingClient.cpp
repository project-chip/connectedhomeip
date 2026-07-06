/*
 *
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <app/clusters/dynamic-lighting-client/DynamicLightingClient.h>
#include <pw_unit_test/framework.h>

#include <app/data-model/Encode.h>
#include <clusters/DynamicLighting/Attributes.h>
#include <lib/core/TLVWriter.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DynamicLighting;

namespace {

// Helper: encode a value via DataModel::Encode, return a TLVReader
// positioned at the encoded element.
template <typename T>
CHIP_ERROR EncodeAndPrepareReader(uint8_t * buf, size_t bufSize, const T & value, TLV::TLVReader & outReader)
{
    TLV::TLVWriter writer;
    writer.Init(buf, bufSize);
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), value));
    ReturnErrorOnFailure(writer.Finalize());

    outReader.Init(buf, writer.GetLengthWritten());
    return outReader.Next();
}

// ---------------------------------------------------------------------------
// Mock delegate that records the most recently reported values.
// ---------------------------------------------------------------------------
struct MockDelegate : public DynamicLightingClientDelegate
{
    // Track call counts for each callback.
    int availableEffectsCallCount = 0;
    int currentEffectIdCallCount  = 0;
    int currentSpeedCallCount     = 0;

    // Last-received values.
    DataModel::Nullable<uint16_t> lastCurrentEffectId;
    DataModel::Nullable<uint16_t> lastCurrentSpeed;
    // Note: DecodableList is not copyable; we track whether the call was made
    // and whether the list was empty.
    bool lastAvailableEffectsWasEmpty = false;

    void OnAvailableEffectsChanged(
        const DataModel::DecodableList<Structs::EffectStruct::DecodableType> & availableEffects) override
    {
        ++availableEffectsCallCount;
        auto it                      = availableEffects.begin();
        lastAvailableEffectsWasEmpty = !it.Next();
    }

    void OnCurrentEffectIDChanged(const DataModel::Nullable<uint16_t> & currentEffectId) override
    {
        ++currentEffectIdCallCount;
        lastCurrentEffectId = currentEffectId;
    }

    void OnCurrentSpeedChanged(const DataModel::Nullable<uint16_t> & currentSpeed) override
    {
        ++currentSpeedCallCount;
        lastCurrentSpeed = currentSpeed;
    }
};

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
struct TestDynamicLightingClient : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    DynamicLightingClient client;
    MockDelegate delegate;
};

} // namespace

// ---------------------------------------------------------------------------
// SetDelegate / GetDelegate
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, SetAndGetDelegate)
{
    EXPECT_EQ(client.GetDelegate(), nullptr);

    client.SetDelegate(&delegate);
    EXPECT_EQ(client.GetDelegate(), &delegate);

    client.SetDelegate(nullptr);
    EXPECT_EQ(client.GetDelegate(), nullptr);
}

// ---------------------------------------------------------------------------
// No delegate: HandleAttributeData must not crash and must return CHIP_NO_ERROR
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, NoDelegateIsHarmless)
{
    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> nullValue;
    nullValue.SetNull();
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), nullValue, reader), CHIP_NO_ERROR);

    EXPECT_EQ(client.HandleAttributeData(Attributes::CurrentEffectID::Id, reader), CHIP_NO_ERROR);
}

// ---------------------------------------------------------------------------
// CurrentEffectID: null value
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, HandleCurrentEffectIDNull)
{
    client.SetDelegate(&delegate);

    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> nullValue;
    nullValue.SetNull();
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), nullValue, reader), CHIP_NO_ERROR);

    ASSERT_EQ(client.HandleAttributeData(Attributes::CurrentEffectID::Id, reader), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.currentEffectIdCallCount, 1);
    EXPECT_TRUE(delegate.lastCurrentEffectId.IsNull());
}

// ---------------------------------------------------------------------------
// CurrentEffectID: non-null value
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, HandleCurrentEffectIDNonNull)
{
    client.SetDelegate(&delegate);

    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull(static_cast<uint16_t>(42));
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), value, reader), CHIP_NO_ERROR);

    ASSERT_EQ(client.HandleAttributeData(Attributes::CurrentEffectID::Id, reader), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.currentEffectIdCallCount, 1);
    ASSERT_FALSE(delegate.lastCurrentEffectId.IsNull());
    EXPECT_EQ(delegate.lastCurrentEffectId.Value(), 42u);
}

// ---------------------------------------------------------------------------
// CurrentSpeed: null value
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, HandleCurrentSpeedNull)
{
    client.SetDelegate(&delegate);

    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> nullValue;
    nullValue.SetNull();
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), nullValue, reader), CHIP_NO_ERROR);

    ASSERT_EQ(client.HandleAttributeData(Attributes::CurrentSpeed::Id, reader), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.currentSpeedCallCount, 1);
    EXPECT_TRUE(delegate.lastCurrentSpeed.IsNull());
}

// ---------------------------------------------------------------------------
// CurrentSpeed: non-null value
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, HandleCurrentSpeedNonNull)
{
    client.SetDelegate(&delegate);

    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull(static_cast<uint16_t>(100));
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), value, reader), CHIP_NO_ERROR);

    ASSERT_EQ(client.HandleAttributeData(Attributes::CurrentSpeed::Id, reader), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.currentSpeedCallCount, 1);
    ASSERT_FALSE(delegate.lastCurrentSpeed.IsNull());
    EXPECT_EQ(delegate.lastCurrentSpeed.Value(), 100u);
}

// ---------------------------------------------------------------------------
// AvailableEffects: empty list
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, HandleAvailableEffectsEmptyList)
{
    client.SetDelegate(&delegate);

    // Encode an empty TLV array.
    uint8_t buf[32];
    TLV::TLVWriter writer;
    writer.Init(buf, sizeof(buf));
    TLV::TLVType outerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(outerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buf, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);

    ASSERT_EQ(client.HandleAttributeData(Attributes::AvailableEffects::Id, reader), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.availableEffectsCallCount, 1);
    EXPECT_TRUE(delegate.lastAvailableEffectsWasEmpty);
}

// ---------------------------------------------------------------------------
// Unknown attribute ID: silently ignored, no delegate callback
// ---------------------------------------------------------------------------
TEST_F(TestDynamicLightingClient, UnknownAttributeIsIgnored)
{
    client.SetDelegate(&delegate);

    uint8_t buf[32];
    TLV::TLVReader reader;
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull(static_cast<uint16_t>(7));
    ASSERT_EQ(EncodeAndPrepareReader(buf, sizeof(buf), value, reader), CHIP_NO_ERROR);

    // Use an attribute ID that does not exist on this cluster.
    constexpr AttributeId kBogusAttributeId = 0xFFFF;
    EXPECT_EQ(client.HandleAttributeData(kBogusAttributeId, reader), CHIP_NO_ERROR);

    EXPECT_EQ(delegate.availableEffectsCallCount, 0);
    EXPECT_EQ(delegate.currentEffectIdCallCount, 0);
    EXPECT_EQ(delegate.currentSpeedCallCount, 0);
}
