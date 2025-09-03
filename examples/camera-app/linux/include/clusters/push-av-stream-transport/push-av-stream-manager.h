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

#pragma once
#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-cluster.h>

#include <camera-device-interface.h>
#include <media-controller.h>
#include <pushav-transport.h>

#include <unordered_map>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

struct PushAvStream
{
    uint16_t id;
    TransportOptionsStruct transportOptions;
    TransportStatusEnum transportStatus;
    PushAvStreamTransportStatusEnum connectionStatus;
};

/**
 * The application delegate to define the options & implement commands.
 */
class PushAvStreamTransportManager : public PushAvStreamTransportDelegate
{
public:
    PushAvStreamTransportManager() = default;
    ~PushAvStreamTransportManager();

    void Init();
    void SetMediaController(MediaController * mediaController);
    void SetCameraDevice(CameraDeviceInterface * cameraDevice);

    // Add missing override keywords and fix signatures
    Protocols::InteractionModel::Status AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                              const uint16_t connectionID) override;

    Protocols::InteractionModel::Status DeallocatePushTransport(const uint16_t connectionID) override;

    Protocols::InteractionModel::Status ModifyPushTransport(const uint16_t connectionID,
                                                            const TransportOptionsStorage transportOptions) override;

    Protocols::InteractionModel::Status SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                           TransportStatusEnum transportStatus) override;

    Protocols::InteractionModel::Status ManuallyTriggerTransport(
        const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
        const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl) override;

    bool ValidateUrl(const std::string & url) override;

    bool ValidateStreamUsage(StreamUsageEnum streamUsage) override;

    bool ValidateSegmentDuration(uint16_t segmentDuration) override;

    Protocols::InteractionModel::Status
    ValidateBandwidthLimit(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override;

    Protocols::InteractionModel::Status SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId) override;

    Protocols::InteractionModel::Status SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId) override;

    Protocols::InteractionModel::Status ValidateVideoStream(uint16_t videoStreamId) override;

    Protocols::InteractionModel::Status ValidateAudioStream(uint16_t audioStreamId) override;

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t connectionID) override;

    void OnAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

private:
    std::vector<PushAvStream> pushavStreams;
    MediaController * mMediaController    = nullptr;
    CameraDeviceInterface * mCameraDevice = nullptr;

    AudioStreamStruct mAudioStreamParams;
    VideoStreamStruct mVideoStreamParams;
    std::unordered_map<uint16_t, std::unique_ptr<PushAVTransport>> mTransportMap; // map for the transport objects
    std::unordered_map<uint16_t, TransportOptionsStruct> mTransportOptionsMap;    // map for the transport options
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
