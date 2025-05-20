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

#include "DeviceManager.h"

#include <commands/interactive/InteractiveCommands.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <cstdio>
#include <string>
#include <thread>

using namespace chip;

namespace camera {

namespace {

constexpr EndpointId kCameraEndpointId = 1;

} // namespace

CHIP_ERROR DeviceManager::Init(Controller::DeviceCommissioner * commissioner)
{
    VerifyOrReturnError(commissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner = commissioner;
    mAVStreamManagment.Init(commissioner);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceManager::AllocateVideoStream(NodeId nodeId, uint8_t streamUsage)
{
    ChipLogProgress(NotSpecified, "Allocate a video stream on the camera device.");

    CHIP_ERROR error = mAVStreamManagment.AllocateVideoStream(nodeId, kCameraEndpointId, streamUsage);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to send VideoStreamAllocate command to the camera device (NodeId: " ChipLogFormatX64
                     "). Error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), error.Format());
    }

    return error;
}

CHIP_ERROR DeviceManager::DeallocateVideoStream(NodeId nodeId, uint16_t videoStreamID)
{
    ChipLogProgress(NotSpecified, "Deallocate a video stream on the camera device.");

    CHIP_ERROR error = mAVStreamManagment.DeallocateVideoStream(nodeId, kCameraEndpointId, videoStreamID);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified,
                     "Failed to send VideoStreamDeallocate command to the camera device (NodeId: " ChipLogFormatX64
                     "). Error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), error.Format());
    }

    return error;
}

void DeviceManager::HandleAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader & data) {}

void DeviceManager::HandleEventData(const app::EventHeader & header, TLV::TLVReader & data) {}

void DeviceManager::HandleCommandResponse(const app::ConcreteCommandPath & path, TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Command Response received.");

    if (path.mClusterId == app::Clusters::CameraAvStreamManagement::Id &&
        path.mCommandId == app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id)
    {
        HandleVideoStreamAllocateResponse(data);
    }
}

void DeviceManager::HandleVideoStreamAllocateResponse(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "Handle VideoStreamAllocateResponse command.");

    app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Log all fields
    ChipLogProgress(NotSpecified, "DecodableType fields:");
    ChipLogProgress(NotSpecified, "  videoStreamID: %u", value.videoStreamID);

    // Persist the stream ID
    mActiveVideoStreamId = value.videoStreamID;

    // ----- Build & launch the GStreamer pipeline ------------------------------
    constexpr char kGstCmd[] = "gst-launch-1.0 -v "
                               "udpsrc port=5000 "
                               "! \"application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96\" "
                               "! rtph264depay "
                               "! queue "
                               "! h264parse "
                               "! avdec_h264 "
                               "! videoconvert "
                               "! autovideosink";

    std::thread(
        [](std::string cmd) {
            ChipLogProgress(NotSpecified, "Starting GStreamer pipeline …");
            int rc = std::system(cmd.c_str());
            if (rc != 0)
            {
                ChipLogError(NotSpecified, "GStreamer pipeline exited with status %d", rc);
            }
        },
        std::string(kGstCmd) /* run a copy in detached thread */)
        .detach();
}

} // namespace camera
