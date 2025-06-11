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

#include <app-common/zap-generated/cluster-objects.h>
#include <device-manager/AVStreamManagement.h>
#include <platform/CHIPDeviceLayer.h>

namespace camera {

class DeviceManager
{
public:
    DeviceManager() = default;

    static DeviceManager & Instance()
    {
        static DeviceManager instance;
        return instance;
    }

    static void VideoStreamSignalHandler(int sig);

    CHIP_ERROR Init(chip::Controller::DeviceCommissioner * commissioner);

    void Shutdown();

    /**
     * @brief Sends a VideoStreamAllocate command to the device.
     *
     * @param nodeId      The node ID of the remote camera device.
     * @param streamUsage The usage of the stream(Recording, LiveView, etc) that this allocation is for.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR AllocateVideoStream(chip::NodeId nodeId, uint8_t streamUsage);

    /**
     * @brief Sends a VideoStreamDeallocate command to the device.
     *
     * @param nodeId        The node ID of the remote camera device.
     * @param videoStreamId The VideoStreamID for the stream to be deallocated.
     * @return CHIP_ERROR   CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR DeallocateVideoStream(chip::NodeId nodeId, uint16_t videoStreamId);

    void HandleAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader & data);

    void HandleEventData(const chip::app::EventHeader & header, chip::TLV::TLVReader & data);

    void HandleCommandResponse(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader & data);

    void StopVideoStream(uint16_t streamId);

    /**
     * @brief Callback invoked when WebRTC session is established
     *
     * @param streamId The video stream ID for which the session was established
     */
    void OnWebRTCSessionEstablished(uint16_t streamId);

private:
    chip::Controller::DeviceCommissioner * mCommissioner = nullptr;
    chip::NodeId mNodeId                                 = chip::kUndefinedNodeId;
    uint8_t mStreamUsage                                 = 0;
    std::map<uint16_t, pid_t> mVideoStreamProcesses; // Stream ID -> Process ID mapping
    uint16_t mPendingVideoStreamId = 0;              // Track the stream ID we're setting up

    AVStreamManagement mAVStreamManagment;

    void HandleVideoStreamAllocateResponse(chip::TLV::TLVReader & data);
    void InitiateWebRTCSession(uint16_t videoStreamId);
    void StartVideoStreamProcess(uint16_t streamId);
    void StopVideoStreamProcess(uint16_t streamId);
};

} // namespace camera
