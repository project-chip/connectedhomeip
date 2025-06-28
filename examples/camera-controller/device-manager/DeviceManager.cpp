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

#include <app/data-model/Nullable.h>
#include <commands/interactive/InteractiveCommands.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>
#include <webrtc-manager/WebRTCManager.h>

#include <cstring>
#include <errno.h>
#include <map>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace chip;
using StreamUsageEnum = chip::app::Clusters::Globals::StreamUsageEnum;

namespace camera {

namespace {

constexpr EndpointId kCameraEndpointId = 1;

} // namespace

void DeviceManager::VideoStreamSignalHandler(int sig)
{
    if (sig == SIGCHLD)
    {
        // Reap any terminated child processes
        pid_t pid;
        int status;

        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            ChipLogProgress(Camera, "Video stream process (PID: %d) terminated", pid);
        }
    }
}

CHIP_ERROR DeviceManager::Init(Controller::DeviceCommissioner * commissioner)
{
    VerifyOrReturnError(commissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner = commissioner;
    mAVStreamManagment.Init(commissioner);

    // Register callback for WebRTC session establishment
    WebRTCManager::Instance().SetSessionEstablishedCallback(
        [this](uint16_t streamId) { this->OnWebRTCSessionEstablished(streamId); });

    return CHIP_NO_ERROR;
}

void DeviceManager::Shutdown()
{
    // Stop all video stream processes
    for (auto & pair : mVideoStreamProcesses)
    {
        StopVideoStreamProcess(pair.first);
    }
    mVideoStreamProcesses.clear();

    // Disconnect WebRTC session
    WebRTCManager::Instance().Disconnect();
}

CHIP_ERROR DeviceManager::AllocateVideoStream(NodeId nodeId, uint8_t streamUsage)
{
    ChipLogProgress(Camera, "Allocate a video stream on the camera device.");

    CHIP_ERROR error = mAVStreamManagment.AllocateVideoStream(nodeId, kCameraEndpointId, streamUsage);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Camera,
                     "Failed to send VideoStreamAllocate command to the camera device (NodeId: " ChipLogFormatX64
                     "). Error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), error.Format());
    }
    else
    {
        mNodeId      = nodeId;
        mStreamUsage = streamUsage;
    }

    return error;
}

CHIP_ERROR DeviceManager::DeallocateVideoStream(NodeId nodeId, uint16_t videoStreamId)
{
    ChipLogProgress(Camera, "Deallocate a video stream on the camera device.");

    CHIP_ERROR error = mAVStreamManagment.DeallocateVideoStream(nodeId, kCameraEndpointId, videoStreamId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Camera,
                     "Failed to send VideoStreamDeallocate command to the camera device (NodeId: " ChipLogFormatX64
                     "). Error: %" CHIP_ERROR_FORMAT,
                     ChipLogValueX64(nodeId), error.Format());
    }
    else
    {
        // Stop the video stream process and disconnect WebRTC
        StopVideoStreamProcess(videoStreamId);
        WebRTCManager::Instance().Disconnect();
    }

    return error;
}

void DeviceManager::HandleAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader & data) {}

void DeviceManager::HandleEventData(const app::EventHeader & header, TLV::TLVReader & data) {}

void DeviceManager::HandleCommandResponse(const app::ConcreteCommandPath & path, TLV::TLVReader & data)
{
    ChipLogProgress(Camera, "Command Response received.");

    if (path.mClusterId == app::Clusters::CameraAvStreamManagement::Id &&
        path.mCommandId == app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::Id)
    {
        HandleVideoStreamAllocateResponse(data);
    }
}

void DeviceManager::StopVideoStream(uint16_t streamId)
{
    // Disconnect WebRTC session
    WebRTCManager::Instance().Disconnect();

    StopVideoStreamProcess(streamId);
}

void DeviceManager::HandleVideoStreamAllocateResponse(TLV::TLVReader & data)
{
    ChipLogProgress(Camera, "Handle VideoStreamAllocateResponse command.");

    app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Log all fields
    ChipLogProgress(Camera, "DecodableType fields:");
    ChipLogProgress(Camera, "  videoStreamId: %u", value.videoStreamID);

    // Store the stream ID we're setting up
    mPendingVideoStreamId = value.videoStreamID;

    InitiateWebRTCSession(value.videoStreamID);
}

void DeviceManager::InitiateWebRTCSession(uint16_t videoStreamId)
{
    ChipLogProgress(Camera, "DeviceManager: Initiating WebRTC session for node=0x" ChipLogFormatX64, ChipLogValueX64(mNodeId));

    // Connect to the WebRTC transport provider on the device
    CHIP_ERROR err = WebRTCManager::Instance().Connnect(*mCommissioner, mNodeId, kCameraEndpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to connect WebRTC manager. Error: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    auto videoStreamIdNullable = app::DataModel::MakeNullable(videoStreamId);
    auto videoStreamIdOptional = MakeOptional(videoStreamIdNullable);
    auto streamUsage           = static_cast<StreamUsageEnum>(mStreamUsage);

    // Provide the offer to establish the WebRTC session
    err = WebRTCManager::Instance().ProvideOffer(app::DataModel::NullNullable, // session ID (null)
                                                 streamUsage,                  // stream‑usage field
                                                 videoStreamIdOptional,        // videoStreamId you just built
                                                 NullOptional);                // audioStreamID (empty)

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to provide an offer. Error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void DeviceManager::OnWebRTCSessionEstablished(uint16_t streamId)
{
    ChipLogProgress(Camera, "WebRTC session established for stream ID: %u", streamId);

    // Verify this matches our pending stream
    if (streamId == mPendingVideoStreamId)
    {
        StartVideoStreamProcess(streamId);
        mPendingVideoStreamId = 0;
    }
}

void DeviceManager::StartVideoStreamProcess(uint16_t streamId)
{
    ChipLogProgress(Camera, "Starting video stream process for stream ID: %u", streamId);

    // Terminate any previous pipeline that was bound to this stream ID
    StopVideoStreamProcess(streamId);

    const uint16_t udpPort    = 5000;
    const std::string portStr = "port=" + std::to_string(udpPort);
    const char * const argv[] = { "gst-launch-1.0",
                                  "udpsrc",
                                  portStr.c_str(),
                                  "!",
                                  "application/x-rtp,media=video,clock-rate=90000,encoding-name=H264,payload=96",
                                  "!",
                                  "rtph264depay",
                                  "!",
                                  "queue",
                                  "!",
                                  "h264parse",
                                  "!",
                                  "avdec_h264",
                                  "!",
                                  "videoconvert",
                                  "!",
                                  "autovideosink",
                                  nullptr };

    // Fork process to run GStreamer pipeline
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process - execute GStreamer command
        // Put the pipeline in its own process group so we can kill it safely if needed.
        setpgid(0, 0);

        // Redirect stdout and stderr to /dev/null to suppress output
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Replace the child with gst‑launch‑1.0 (only returns on error)
        execvp(argv[0], const_cast<char * const *>(argv));

        // If we got here execvp failed
        ChipLogError(Camera, "execvp(gst-launch-1.0) failed: %s", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        // Parent process - store PID for later cleanup
        mVideoStreamProcesses[streamId] = pid; // Track the real gst‑launch PID
        ChipLogProgress(Camera, "Video stream process started with PID: %d", pid);
    }
    else
    {
        // Fork failed
        ChipLogError(Camera, "Failed to fork process for video stream. Error: %s", strerror(errno));
    }
}

void DeviceManager::StopVideoStreamProcess(uint16_t streamId)
{
    auto it = mVideoStreamProcesses.find(streamId);
    if (it != mVideoStreamProcesses.end())
    {
        pid_t pid = it->second;
        ChipLogProgress(Camera, "Stopping video stream process (PID: %d) for stream ID: %u", pid, streamId);

        // Send SIGTERM first for graceful shutdown
        if (kill(pid, SIGTERM) == 0)
        {
            // Wait for graceful shutdown (with timeout)
            int status;
            int waitResult = waitpid(pid, &status, WNOHANG);

            if (waitResult == 0)
            {
                // Process still running, wait a bit then force kill
                sleep(1);
                waitResult = waitpid(pid, &status, WNOHANG);

                if (waitResult == 0)
                {
                    ChipLogProgress(Camera, "Force killing video stream process (PID: %d)", pid);
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0); // Wait for process to be reaped
                }
            }
        }
        else
        {
            ChipLogError(Camera, "Failed to send SIGTERM to process %d: %s", pid, strerror(errno));
        }

        mVideoStreamProcesses.erase(it);
    }
}

} // namespace camera
