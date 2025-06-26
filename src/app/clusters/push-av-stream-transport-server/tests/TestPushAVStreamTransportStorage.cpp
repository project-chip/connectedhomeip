/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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
#include <vector>

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-storage.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

using TransportZoneOptionsDecodableStruct              = Structs::TransportZoneOptionsStruct::DecodableType;
using TransportTriggerOptionsDecodableStruct           = Structs::TransportTriggerOptionsStruct::DecodableType;
using TransportMotionTriggerTimeControlDecodableStruct = Structs::TransportMotionTriggerTimeControlStruct::DecodableType;
using TransportOptionsDecodableStruct                  = Structs::TransportOptionsStruct::DecodableType;

class TestPushAVStreamTransportStorage : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

uint8_t tlvBuffer[512];
TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
TransportTriggerOptionsDecodableStruct triggerOptions;
DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;

TEST_F(TestPushAVStreamTransportStorage, TestSetUpTriggerOptions)
{
    // Create a TransportMotionTriggerTimeControlStruct object
    motionTimeControl.initialDuration      = 5000;
    motionTimeControl.augmentationDuration = 2000;
    motionTimeControl.maxDuration          = 30000;
    motionTimeControl.blindDuration        = 1000;

    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;

    // Create transport zone options structs
    Structs::TransportZoneOptionsStruct::Type zone1;
    zone1.zone.SetNonNull(1);
    zone1.sensitivity.SetValue(5);

    Structs::TransportZoneOptionsStruct::Type zone2;
    zone2.zone.SetNonNull(2);
    zone2.sensitivity.SetValue(10);

    // Encode them into a TLV buffer
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer, sizeof(tlvBuffer));

    TLV::TLVWriter containerWriter;
    CHIP_ERROR err;

    err = writer.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(containerWriter, TLV::AnonymousTag(), zone2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(containerWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    size_t encodedLen = writer.GetLengthWritten();

    // Decode the TLV into a DecodableList
    TLV::TLVReader reader;
    reader.Init(tlvBuffer, static_cast<uint32_t>(encodedLen));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodedList.Decode(reader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));
    triggerOptions.motionSensitivity.ClearValue();
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);
}

TEST_F(TestPushAVStreamTransportStorage, TestTransportTriggerOptionsStorage)
{
    TransportTriggerOptionsStorage transportTriggerOptionsStorage(triggerOptions);
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
