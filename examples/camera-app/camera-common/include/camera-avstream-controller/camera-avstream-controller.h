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

#include <app/clusters/camera-av-stream-management-server/CameraAVStreamManagementCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvStreamManagement {

/**
 * The application interface to define the options & implement commands.
 */
class CameraAVStreamController
{
public:
    virtual ~CameraAVStreamController() = default;

    virtual CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                           Optional<DataModel::Nullable<uint16_t>> & audioStreamId) = 0;

    virtual CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) = 0;

    virtual CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) = 0;

    virtual CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) = 0;

    virtual CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) = 0;

    virtual CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) = 0;

    virtual CHIP_ERROR SetHardPrivacyModeOn(bool hardPrivacyMode) = 0;

    virtual bool HasAllocatedVideoStreams() = 0;

    virtual bool HasAllocatedAudioStreams() = 0;

    /**
     * @brief Calculates the total bandwidth in bps for the given video and audio stream IDs.
     * It iterates through the allocated video and audio streams, and if a matching stream ID is found,
     * its bit rate (maxBitRate for video, bitRate for audio) is added to the total bandwidth.
     *
     * @param videoStreamId Optional nullable ID of the video stream.
     * @param audioStreamId Optional nullable ID of the audio stream.
     * @param outBandwidthbps Output parameter for the calculated total bandwidth in bps.
     */
    virtual void GetBandwidthForStreams(const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                        const Optional<DataModel::Nullable<uint16_t>> & audioStreamId,
                                        uint32_t & outBandwidthbps) = 0;
};

} // namespace CameraAvStreamManagement
} // namespace Clusters
} // namespace app
} // namespace chip
