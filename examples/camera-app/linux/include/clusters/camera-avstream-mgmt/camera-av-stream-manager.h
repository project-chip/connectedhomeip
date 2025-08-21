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

#include "camera-avstream-controller.h"
#include "camera-device-interface.h"
#include <app/clusters/camera-av-stream-management-server/camera-av-stream-management-server.h>
#include <app/util/config.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

/**
 * The application delegate to define the options & implement commands.
 */
class CameraAVStreamManager : public CameraAVStreamMgmtDelegate, public CameraAVStreamController
{
public:
    Protocols::InteractionModel::Status VideoStreamAllocate(const VideoStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override;

    Protocols::InteractionModel::Status VideoStreamModify(const uint16_t streamID, const chip::Optional<bool> waterMarkEnabled,
                                                          const chip::Optional<bool> osdEnabled) override;

    Protocols::InteractionModel::Status VideoStreamDeallocate(const uint16_t streamID) override;

    Protocols::InteractionModel::Status AudioStreamAllocate(const AudioStreamStruct & allocateArgs,
                                                            uint16_t & outStreamID) override;

    Protocols::InteractionModel::Status AudioStreamDeallocate(const uint16_t streamID) override;

    Protocols::InteractionModel::Status SnapshotStreamAllocate(const SnapshotStreamStruct & allocateArgs,
                                                               uint16_t & outStreamID) override;

    Protocols::InteractionModel::Status SnapshotStreamModify(const uint16_t streamID, const chip::Optional<bool> waterMarkEnabled,
                                                             const chip::Optional<bool> osdEnabled) override;

    Protocols::InteractionModel::Status SnapshotStreamDeallocate(const uint16_t streamID) override;

    void OnVideoStreamAllocated(const VideoStreamStruct & allocatedStream, StreamAllocationAction action) override;

    void OnStreamUsagePrioritiesChanged() override;

    void OnAttributeChanged(AttributeId attributeId) override;

    Protocols::InteractionModel::Status CaptureSnapshot(const chip::app::DataModel::Nullable<uint16_t> streamID,
                                                        const VideoResolutionStruct & resolution,
                                                        ImageSnapshot & outImageSnapshot) override;

    CHIP_ERROR
    LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams) override;

    CHIP_ERROR
    LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams) override;

    CHIP_ERROR
    LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams) override;

    CHIP_ERROR
    ValidateStreamUsage(StreamUsageEnum streamUsage, const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                        const Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override;

    CHIP_ERROR
    ValidateVideoStreamID(uint16_t videoStreamId) override;

    CHIP_ERROR
    ValidateAudioStreamID(uint16_t audioStreamId) override;

    CHIP_ERROR
    IsPrivacyModeActive(bool & isActive) override;

    bool HasAllocatedVideoStreams() override;

    bool HasAllocatedAudioStreams() override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    CHIP_ERROR OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override;

    CHIP_ERROR OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override;

    CameraAVStreamManager()  = default;
    ~CameraAVStreamManager() = default;

    void SetCameraDeviceHAL(CameraDeviceInterface * aCameraDevice);

private:
    CameraDeviceInterface * mCameraDeviceHAL = nullptr;
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
