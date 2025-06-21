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

#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <app/util/config.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

/**
 * The application interface to define the options & implement commands.
 */
class CameraAVStreamController : public CameraAVStreamMgmtDelegate
{
public:
    virtual ~CameraAVStreamController() = default;

    /**
     * delegate handlers
     */
    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override = 0;

    Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID, const chip::Optional<bool> waterMarkEnabled,
                                                          const chip::Optional<bool> osdEnabled) override = 0;

    Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) override = 0;

    Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override = 0;

    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override = 0;

    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                               uint16_t & outStreamID) override = 0;

    Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const chip::Optional<bool> waterMarkEnabled,
                                                             const chip::Optional<bool> osdEnabled) override = 0;

    Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) override = 0;

    void OnStreamUsagePrioritiesChanged() override = 0;

    void OnAttributeChanged(AttributeId attributeId) override = 0;

    Protocols::InteractionModel::Status CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID,
                                                        const VideoResolutionStruct & resolution,
                                                        ImageSnapshot & outImageSnapshot) override = 0;

    CHIP_ERROR
    LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams) override = 0;

    CHIP_ERROR
    LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams) override = 0;

    CHIP_ERROR
    LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams) override = 0;

    CHIP_ERROR PersistentAttributesLoadedCallback() override = 0;

    CHIP_ERROR OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override = 0;

    CHIP_ERROR OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override = 0;

    /**
     * controller methods
     */
    virtual CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage,
                                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    virtual CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) = 0;

    virtual CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) = 0;

    virtual CHIP_ERROR IsPrivacyModeActive(bool & isActive) = 0;

    virtual bool HasAllocatedVideoStreams() = 0;

    virtual bool HasAllocatedAudioStreams() = 0;
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
