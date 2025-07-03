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

TEST_F(TestPushAVStreamTransportStorage, TestTransportConfigurationStorage)
{
    uint8_t tlvBuffer[512];
    TransportMotionTriggerTimeControlDecodableStruct motionTimeControl;
    TransportTriggerOptionsDecodableStruct triggerOptions;
    DataModel::DecodableList<Structs::TransportZoneOptionsStruct::DecodableType> decodedList;
    TransportOptionsDecodableStruct transportOptions;

    /*Test TransportOptionsStorage*/

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
    triggerOptions.motionSensitivity.SetValue(DataModel::MakeNullable((uint8_t) 5));
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    TransportTriggerOptionsStorage transportTriggerOptionsStorage(triggerOptions);

    EXPECT_EQ(transportTriggerOptionsStorage.triggerType, TransportTriggerTypeEnum::kMotion);

    DataModel::List<const Structs::TransportZoneOptionsStruct::Type> & motionZonesList =
        transportTriggerOptionsStorage.motionZones.Value().Value();

    EXPECT_EQ(motionZonesList.size(), (size_t) 2);

    Structs::TransportZoneOptionsStruct::Type motionZone1 = motionZonesList[0];
    Structs::TransportZoneOptionsStruct::Type motionZone2 = motionZonesList[1];

    EXPECT_FALSE(motionZone1.zone.IsNull());
    EXPECT_EQ(motionZone1.zone.Value(), 1);
    EXPECT_TRUE(motionZone1.sensitivity.HasValue());
    EXPECT_EQ(motionZone1.sensitivity.Value(), 5);

    EXPECT_FALSE(motionZone2.zone.IsNull());
    EXPECT_EQ(motionZone2.zone.Value(), 2);
    EXPECT_TRUE(motionZone2.sensitivity.HasValue());
    EXPECT_EQ(motionZone2.sensitivity.Value(), 10);

    EXPECT_EQ(transportTriggerOptionsStorage.motionSensitivity.Value().Value(), (uint8_t) 5);

    EXPECT_EQ(transportTriggerOptionsStorage.motionTimeControl.Value().initialDuration, 5000);
    EXPECT_EQ(transportTriggerOptionsStorage.motionTimeControl.Value().augmentationDuration, 2000);
    EXPECT_EQ(transportTriggerOptionsStorage.motionTimeControl.Value().maxDuration, (uint32_t) 30000);
    EXPECT_EQ(transportTriggerOptionsStorage.motionTimeControl.Value().blindDuration, 1000);
    EXPECT_EQ(transportTriggerOptionsStorage.maxPreRollLen.Value(), 1000);

    TransportTriggerOptionsStorage transportTriggerOptionsStorageCopy(transportTriggerOptionsStorage);

    EXPECT_EQ(transportTriggerOptionsStorageCopy.triggerType, TransportTriggerTypeEnum::kMotion);

    DataModel::List<const Structs::TransportZoneOptionsStruct::Type> & motionZonesListCopy =
        transportTriggerOptionsStorageCopy.motionZones.Value().Value();

    EXPECT_EQ(motionZonesListCopy.size(), (size_t) 2);

    Structs::TransportZoneOptionsStruct::Type motionZone1Copy = motionZonesListCopy[0];
    Structs::TransportZoneOptionsStruct::Type motionZone2Copy = motionZonesListCopy[1];

    EXPECT_FALSE(motionZone1Copy.zone.IsNull());
    EXPECT_EQ(motionZone1Copy.zone.Value(), 1);
    EXPECT_TRUE(motionZone1Copy.sensitivity.HasValue());
    EXPECT_EQ(motionZone1Copy.sensitivity.Value(), 5);

    EXPECT_FALSE(motionZone2Copy.zone.IsNull());
    EXPECT_EQ(motionZone2Copy.zone.Value(), 2);
    EXPECT_TRUE(motionZone2Copy.sensitivity.HasValue());
    EXPECT_EQ(motionZone2Copy.sensitivity.Value(), 10);

    EXPECT_EQ(transportTriggerOptionsStorageCopy.motionSensitivity.Value().Value(), (uint8_t) 5);

    EXPECT_EQ(transportTriggerOptionsStorageCopy.motionTimeControl.Value().initialDuration, 5000);
    EXPECT_EQ(transportTriggerOptionsStorageCopy.motionTimeControl.Value().augmentationDuration, 2000);
    EXPECT_EQ(transportTriggerOptionsStorageCopy.motionTimeControl.Value().maxDuration, (uint32_t) 30000);
    EXPECT_EQ(transportTriggerOptionsStorageCopy.motionTimeControl.Value().blindDuration, 1000);

    // Accessing buffer using base struct
    TransportTriggerOptionsStruct BaseTransportTriggerOptions =
        static_cast<TransportTriggerOptionsStruct>(transportTriggerOptionsStorageCopy);

    DataModel::List<const Structs::TransportZoneOptionsStruct::Type> & motionZonesListBase =
        BaseTransportTriggerOptions.motionZones.Value().Value();

    EXPECT_EQ(motionZonesListBase.size(), (size_t) 2);

    Structs::TransportZoneOptionsStruct::Type motionZone1Base = motionZonesListBase[0];
    Structs::TransportZoneOptionsStruct::Type motionZone2Base = motionZonesListBase[1];

    EXPECT_FALSE(motionZone1Base.zone.IsNull());
    EXPECT_EQ(motionZone1Base.zone.Value(), 1);
    EXPECT_TRUE(motionZone1Base.sensitivity.HasValue());
    EXPECT_EQ(motionZone1Base.sensitivity.Value(), 5);

    EXPECT_FALSE(motionZone2Base.zone.IsNull());
    EXPECT_EQ(motionZone2Base.zone.Value(), 2);
    EXPECT_TRUE(motionZone2Base.sensitivity.HasValue());
    EXPECT_EQ(motionZone2Base.sensitivity.Value(), 10);

    /*Test CMAFContainerOptionsStorage*/

    CMAFContainerOptionsStruct cmafContainerOptions;

    cmafContainerOptions.chunkDuration = 1000;
    cmafContainerOptions.metadataEnabled.SetValue(true);

    std::string cencKey   = "1234567890ABCDEF";
    std::string cencKeyID = "1234567890ABCDEF";

    cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
    cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

    CMAFContainerOptionsStorage cmafContainerOptionsStorage(cmafContainerOptions);

    EXPECT_EQ(cmafContainerOptionsStorage.chunkDuration, 1000);
    EXPECT_TRUE(cmafContainerOptionsStorage.metadataEnabled.HasValue());
    EXPECT_TRUE(cmafContainerOptionsStorage.metadataEnabled.Value());

    std::string cencKeyStr(cmafContainerOptionsStorage.CENCKey.Value().data(),
                           cmafContainerOptionsStorage.CENCKey.Value().data() + cmafContainerOptionsStorage.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStr, cencKey);

    std::string cencKeyIDStr(cmafContainerOptionsStorage.CENCKeyID.Value().data(),
                             cmafContainerOptionsStorage.CENCKeyID.Value().data() +
                                 cmafContainerOptionsStorage.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStr, cencKeyID);

    CMAFContainerOptionsStorage cmafContainerOptionsStorageCopy(cmafContainerOptionsStorage);

    EXPECT_EQ(cmafContainerOptionsStorageCopy.chunkDuration, 1000);
    EXPECT_TRUE(cmafContainerOptionsStorageCopy.metadataEnabled.HasValue());
    EXPECT_TRUE(cmafContainerOptionsStorageCopy.metadataEnabled.Value());

    std::string cencKeyStrCopy(cmafContainerOptionsStorageCopy.CENCKey.Value().data(),
                               cmafContainerOptionsStorageCopy.CENCKey.Value().data() +
                                   cmafContainerOptionsStorageCopy.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStrCopy, cencKey);

    std::string cencKeyIDStrCopy(cmafContainerOptionsStorageCopy.CENCKeyID.Value().data(),
                                 cmafContainerOptionsStorageCopy.CENCKeyID.Value().data() +
                                     cmafContainerOptionsStorageCopy.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStrCopy, cencKeyID);

    // Accessing buffer using base struct

    CMAFContainerOptionsStruct BaseCMAFContainerOptions = static_cast<CMAFContainerOptionsStruct>(cmafContainerOptionsStorageCopy);

    EXPECT_EQ(BaseCMAFContainerOptions.chunkDuration, 1000);
    EXPECT_TRUE(BaseCMAFContainerOptions.metadataEnabled.HasValue());
    EXPECT_TRUE(BaseCMAFContainerOptions.metadataEnabled.Value());

    std::string cencKeyStrBase(BaseCMAFContainerOptions.CENCKey.Value().data(),
                               BaseCMAFContainerOptions.CENCKey.Value().data() + BaseCMAFContainerOptions.CENCKey.Value().size());

    EXPECT_EQ(cencKeyStrBase, cencKey);

    std::string cencKeyIDStrBase(BaseCMAFContainerOptions.CENCKeyID.Value().data(),
                                 BaseCMAFContainerOptions.CENCKeyID.Value().data() +
                                     BaseCMAFContainerOptions.CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStrBase, cencKeyID);

    /*Test ContainerOptionsStorage*/

    // Using Type as ContainerOptionsStruct::DecodableType == ContainerOptionsStruct::Type

    ContainerOptionsStruct containerOptions;

    containerOptions.containerType = ContainerFormatEnum::kCmaf;
    containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

    ContainerOptionsStorage containerOptionsStorage(containerOptions);

    EXPECT_EQ(containerOptionsStorage.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(containerOptionsStorage.CMAFContainerOptions.HasValue());
    EXPECT_EQ(containerOptionsStorage.CMAFContainerOptions.Value().chunkDuration, 1000);
    EXPECT_TRUE(containerOptionsStorage.CMAFContainerOptions.Value().metadataEnabled.HasValue());
    EXPECT_TRUE(containerOptionsStorage.CMAFContainerOptions.Value().metadataEnabled.Value());

    std::string cencKeyStrContainer(containerOptionsStorage.CMAFContainerOptions.Value().CENCKey.Value().data(),
                                    containerOptionsStorage.CMAFContainerOptions.Value().CENCKey.Value().data() +
                                        containerOptionsStorage.CMAFContainerOptions.Value().CENCKey.Value().size());

    EXPECT_EQ(cencKeyStrContainer, cencKey);

    std::string cencKeyIDStrContainer(containerOptionsStorage.CMAFContainerOptions.Value().CENCKeyID.Value().data(),
                                      containerOptionsStorage.CMAFContainerOptions.Value().CENCKeyID.Value().data() +
                                          containerOptionsStorage.CMAFContainerOptions.Value().CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStrContainer, cencKeyID);

    ContainerOptionsStorage containerOptionsStorageCopy(containerOptionsStorage);

    EXPECT_EQ(containerOptionsStorageCopy.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(containerOptionsStorageCopy.CMAFContainerOptions.HasValue());
    EXPECT_EQ(containerOptionsStorageCopy.CMAFContainerOptions.Value().chunkDuration, 1000);
    EXPECT_TRUE(containerOptionsStorageCopy.CMAFContainerOptions.Value().metadataEnabled.HasValue());
    EXPECT_TRUE(containerOptionsStorageCopy.CMAFContainerOptions.Value().metadataEnabled.Value());

    std::string cencKeyStrContainerCopy(containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKey.Value().data(),
                                        containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKey.Value().data() +
                                            containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKey.Value().size());

    EXPECT_EQ(cencKeyStrContainerCopy, cencKey);

    std::string cencKeyIDStrContainerCopy(containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKeyID.Value().data(),
                                          containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKeyID.Value().data() +
                                              containerOptionsStorageCopy.CMAFContainerOptions.Value().CENCKeyID.Value().size());

    EXPECT_EQ(cencKeyIDStrContainerCopy, cencKeyID);

    // Accessing buffer using base struct

    ContainerOptionsStruct BaseContainerOptions = static_cast<ContainerOptionsStruct>(containerOptionsStorage);

    EXPECT_EQ(BaseContainerOptions.containerType, ContainerFormatEnum::kCmaf);
    EXPECT_TRUE(BaseContainerOptions.CMAFContainerOptions.HasValue());
    EXPECT_EQ(BaseContainerOptions.CMAFContainerOptions.Value().chunkDuration, 1000);
    EXPECT_TRUE(BaseContainerOptions.CMAFContainerOptions.Value().metadataEnabled.HasValue());
    EXPECT_TRUE(BaseContainerOptions.CMAFContainerOptions.Value().metadataEnabled.Value());

    std::string containerCENCKeyStrBase(BaseContainerOptions.CMAFContainerOptions.Value().CENCKey.Value().data(),
                                        BaseContainerOptions.CMAFContainerOptions.Value().CENCKey.Value().data() +
                                            BaseContainerOptions.CMAFContainerOptions.Value().CENCKey.Value().size());

    EXPECT_EQ(containerCENCKeyStrBase, cencKey);

    std::string containerCENCKeyIDStrBase(BaseContainerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().data(),
                                          BaseContainerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().data() +
                                              BaseContainerOptions.CMAFContainerOptions.Value().CENCKeyID.Value().size());

    EXPECT_EQ(containerCENCKeyIDStrBase, cencKeyID);

    /*Test TransportOptionsStorage*/
    triggerOptions.triggerType = TransportTriggerTypeEnum::kMotion;
    triggerOptions.motionZones.SetValue(DataModel::MakeNullable(decodedList));

    triggerOptions.motionSensitivity.SetValue(DataModel::MakeNullable((uint8_t) 5));
    triggerOptions.motionTimeControl.SetValue(motionTimeControl);
    triggerOptions.maxPreRollLen.SetValue(1000);

    transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
    transportOptions.videoStreamID.SetValue(1);
    transportOptions.audioStreamID.SetValue(2);
    transportOptions.endpointID       = 1;
    std::string url                   = "rtsp://192.168.1.100:554/stream";
    transportOptions.url              = Span(url.data(), url.size());
    transportOptions.triggerOptions   = triggerOptions;
    transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
    transportOptions.containerOptions = containerOptions;
    transportOptions.expiryTime.SetValue(1000);

    TransportOptionsStorage transportOptionsStorage(transportOptions);

    EXPECT_EQ(transportOptionsStorage.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(transportOptionsStorage.videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(transportOptionsStorage.audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(transportOptionsStorage.endpointID, 1);
    EXPECT_EQ(transportOptionsStorage.url.size(), url.size());
    std::string transportOptionsUrlStr(transportOptionsStorage.url.data(), transportOptionsStorage.url.size());
    EXPECT_EQ(transportOptionsUrlStr, url);

    TransportOptionsStorage transportOptionsStorageCopy(transportOptionsStorage);

    EXPECT_EQ(transportOptionsStorageCopy.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(transportOptionsStorageCopy.videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(transportOptionsStorageCopy.audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(transportOptionsStorageCopy.endpointID, 1);
    EXPECT_EQ(transportOptionsStorageCopy.url.size(), url.size());
    std::string transportOptionsUrlStrCopy(transportOptionsStorageCopy.url.data(), transportOptionsStorageCopy.url.size());
    EXPECT_EQ(transportOptionsUrlStrCopy, url);

    // Accessing buffer using base struct

    TransportOptionsStruct BaseTransportOptions = static_cast<TransportOptionsStruct>(transportOptionsStorage);

    EXPECT_EQ(BaseTransportOptions.streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(BaseTransportOptions.videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(BaseTransportOptions.audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(BaseTransportOptions.endpointID, 1);
    EXPECT_EQ(BaseTransportOptions.url.size(), url.size());
    std::string transportOptionsUrlStrBase(BaseTransportOptions.url.data(), BaseTransportOptions.url.size());
    EXPECT_EQ(transportOptionsUrlStrBase, url);

    /*Test TransportConfigurationStorage*/

    std::shared_ptr<TransportOptionsStorage> transportOptionsPtr{ new (std::nothrow) TransportOptionsStorage(transportOptions) };

    TransportConfigurationStorage transportConfigurationStorage(1, transportOptionsPtr);

    EXPECT_EQ(transportConfigurationStorage.connectionID, 1);
    EXPECT_EQ(transportConfigurationStorage.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_EQ(transportConfigurationStorage.transportOptions.Value().streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(transportConfigurationStorage.transportOptions.Value().videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(transportConfigurationStorage.transportOptions.Value().audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(transportConfigurationStorage.transportOptions.Value().endpointID, 1);
    EXPECT_EQ(transportConfigurationStorage.transportOptions.Value().url.size(), url.size());
    std::string transportOptionsUrlStrConfiguration(transportConfigurationStorage.transportOptions.Value().url.data(),
                                                    transportConfigurationStorage.transportOptions.Value().url.size());
    EXPECT_EQ(transportOptionsUrlStrConfiguration, url);

    TransportConfigurationStorage transportConfigurationStorageCopy(transportConfigurationStorage);

    EXPECT_EQ(transportConfigurationStorageCopy.connectionID, 1);
    EXPECT_EQ(transportConfigurationStorageCopy.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_EQ(transportConfigurationStorageCopy.transportOptions.Value().streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(transportConfigurationStorageCopy.transportOptions.Value().videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(transportConfigurationStorageCopy.transportOptions.Value().audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(transportConfigurationStorageCopy.transportOptions.Value().endpointID, 1);
    EXPECT_EQ(transportConfigurationStorageCopy.transportOptions.Value().url.size(), url.size());
    std::string transportOptionsUrlStrConfigurationCopy(transportConfigurationStorageCopy.transportOptions.Value().url.data(),
                                                        transportConfigurationStorageCopy.transportOptions.Value().url.size());
    EXPECT_EQ(transportOptionsUrlStrConfigurationCopy, url);

    // Accessing buffer using base struct

    TransportConfigurationStruct BaseTransportConfiguration =
        static_cast<TransportConfigurationStruct>(transportConfigurationStorage);

    EXPECT_EQ(BaseTransportConfiguration.connectionID, 1);
    EXPECT_EQ(BaseTransportConfiguration.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_EQ(BaseTransportConfiguration.transportOptions.Value().streamUsage, StreamUsageEnum::kAnalysis);
    EXPECT_EQ(BaseTransportConfiguration.transportOptions.Value().videoStreamID.Value(), (uint16_t) 1);
    EXPECT_EQ(BaseTransportConfiguration.transportOptions.Value().audioStreamID.Value(), (uint16_t) 2);
    EXPECT_EQ(BaseTransportConfiguration.transportOptions.Value().endpointID, 1);
    EXPECT_EQ(BaseTransportConfiguration.transportOptions.Value().url.size(), url.size());
    std::string transportOptionsUrlStrConfigurationBase(BaseTransportConfiguration.transportOptions.Value().url.data(),
                                                        BaseTransportConfiguration.transportOptions.Value().url.size());
    EXPECT_EQ(transportOptionsUrlStrConfigurationBase, url);

    /*Test TransportConfigurationStorage with null transport options*/

    std::shared_ptr<TransportOptionsStorage> nullTransportOptionsPtr;

    TransportConfigurationStorage transportConfigurationStorageNull(1, nullTransportOptionsPtr);

    EXPECT_EQ(transportConfigurationStorageNull.connectionID, 1);
    EXPECT_EQ(transportConfigurationStorageNull.transportStatus, TransportStatusEnum::kInactive);
    EXPECT_FALSE(transportConfigurationStorageNull.transportOptions.HasValue());
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
