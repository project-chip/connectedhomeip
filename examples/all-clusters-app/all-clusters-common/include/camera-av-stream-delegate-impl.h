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

constexpr uint8_t kMaxVideoStreams  = 10; // Maximum number of pre-allocated streams
constexpr uint16_t kInvalidStreamID = 65500;

struct VideoStream
{
    uint16_t id;          // Stream ID
    bool isAllocated;     // Flag to indicate if the stream is allocated
    VideoCodecEnum codec; // Codec information (e.g., "H.264", "HEVC")
    uint16_t frameRate;   // frame rate
};

struct AudioStream
{
    uint16_t id;          // Stream ID
    bool isAllocated;     // Flag to indicate if the stream is allocated
    AudioCodecEnum codec; // Codec information (e.g., "OPUS", "AACLC")
    uint8_t channelCount; // channel count
};

struct SnapshotStream
{
    uint16_t id;          // Stream ID
    bool isAllocated;     // Flag to indicate if the stream is allocated
    ImageCodecEnum codec; // Codec information (e.g., "JPEG")
    uint8_t quality;      // Quality
};

/**
 * The application delegate to define the options & implement commands.
 */
class CameraAVStreamManager : public CameraAVStreamMgmtDelegate
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

    Protocols::InteractionModel::Status CaptureSnapshot(const DataModel::Nullable<uint16_t> streamID,
                                                        const VideoResolutionStruct & resolution,
                                                        ImageSnapshot & outImageSnapshot) override;

    CHIP_ERROR
    LoadAllocatedVideoStreams(std::vector<VideoStreamStruct> & allocatedVideoStreams) override;

    CHIP_ERROR
    LoadAllocatedAudioStreams(std::vector<AudioStreamStruct> & allocatedAudioStreams) override;

    CHIP_ERROR
    LoadAllocatedSnapshotStreams(std::vector<SnapshotStreamStruct> & allocatedSnapshotStreams) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    CHIP_ERROR OnTransportAcquireAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override;

    CHIP_ERROR OnTransportReleaseAudioVideoStreams(uint16_t audioStreamID, uint16_t videoStreamID) override;

    void Init();

    CameraAVStreamManager()  = default;
    ~CameraAVStreamManager() = default;

    // static inline CameraAVStreamManager & GetInstance() { return sCameraAVStreamMgrInstance; }

private:
    std::vector<VideoStream> videoStreams;       // Vector to hold available video streams
    std::vector<AudioStream> audioStreams;       // Vector to hold available audio streams
    std::vector<SnapshotStream> snapshotStreams; // Vector to hold available snapshot streams

    void InitializeAvailableVideoStreams();
    void InitializeAvailableAudioStreams();
    void InitializeAvailableSnapshotStreams();
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
