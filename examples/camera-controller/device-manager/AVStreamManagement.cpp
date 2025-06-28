/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "AVStreamManagement.h"
#include "DeviceManager.h"

using namespace ::chip;

namespace {

constexpr uint16_t kMinFrameRate        = 30;
constexpr uint16_t kMaxFrameRate        = 120;
constexpr uint32_t kDefaultBitRate      = 10000; // bits per second
constexpr uint16_t kMinKeyFrameInterval = 1000;
constexpr uint16_t kMaxKeyFrameInterval = 10000;
constexpr uint16_t kMinWidth            = 640;
constexpr uint16_t kMinHeight           = 360;
constexpr uint16_t kMaxWidth            = 1920;
constexpr uint16_t kMaxHeight           = 1080;

} // namespace

namespace camera {

void AVStreamManagement::Init(Controller::DeviceCommissioner * commissioner)
{
    // Ensure that mCommissioner is not already initialized
    VerifyOrDie(mCommissioner == nullptr);

    ChipLogProgress(Camera, "Initilize CommissionerControl");
    mCommissioner = commissioner;
}

CHIP_ERROR AVStreamManagement::AllocateVideoStream(NodeId nodeId, EndpointId endpointId, uint8_t streamUsage)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Camera, "Sending VideoStreamAllocate to (node=0x" ChipLogFormatX64 ", ep=%u, usage=%u)",
                    ChipLogValueX64(nodeId), endpointId, streamUsage);

    // Clear any stale data from previous requests.
    mVideoStreamAllocate = {};

    mVideoStreamAllocate.streamUsage = static_cast<app::Clusters::Globals::StreamUsageEnum>(streamUsage);
    mVideoStreamAllocate.videoCodec =
        app::Clusters::CameraAvStreamManagement::VideoCodecEnum::kH264; // Default to H.264; adjust as needed.

    mVideoStreamAllocate.minFrameRate = kMinFrameRate;
    mVideoStreamAllocate.maxFrameRate = kMaxFrameRate;

    mVideoStreamAllocate.minResolution = { .width = kMinWidth, .height = kMinHeight };
    mVideoStreamAllocate.maxResolution = { .width = kMaxWidth, .height = kMaxHeight };

    mVideoStreamAllocate.minBitRate = kDefaultBitRate;
    mVideoStreamAllocate.maxBitRate = kDefaultBitRate;

    mVideoStreamAllocate.minKeyFrameInterval = kMinKeyFrameInterval;
    mVideoStreamAllocate.maxKeyFrameInterval = kMaxKeyFrameInterval;

    mEndpointId  = endpointId;
    mCommandType = CommandType::kVideoStreamAllocate;
    return mCommissioner->GetConnectedDevice(nodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

CHIP_ERROR AVStreamManagement::DeallocateVideoStream(NodeId nodeId, EndpointId endpointId, uint16_t videoStreamID)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Camera, "Sending VideoStreamDeallocate to (node=0x" ChipLogFormatX64 ", ep=%u, videoStreamID=%u)",
                    ChipLogValueX64(nodeId), endpointId, videoStreamID);

    mVideoStreamDeallocate.videoStreamID = videoStreamID;

    mEndpointId  = endpointId;
    mCommandType = CommandType::kVideoStreamDeallocate;
    return mCommissioner->GetConnectedDevice(nodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

void AVStreamManagement::OnResponse(app::CommandSender * client, const app::ConcreteCommandPath & path,
                                    const app::StatusIB & status, TLV::TLVReader * data)
{
    ChipLogProgress(Camera, "AVStreamManagement: OnResponse.");

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Camera, "Response Failure: %s", ErrorStr(error));
        return;
    }

    if (data != nullptr)
    {
        DeviceManager::Instance().HandleCommandResponse(path, *data);
    }
}

void AVStreamManagement::OnError(const app::CommandSender * client, CHIP_ERROR error)
{
    // Handle the error, then reset mCommandSender
    ChipLogError(Camera, "AVStreamManagement: OnError. Error: %" CHIP_ERROR_FORMAT, error.Format());
}

void AVStreamManagement::OnDone(app::CommandSender * client)
{
    ChipLogProgress(Camera, "AVStreamManagement: OnDone.");

    switch (mCommandType)
    {
    case CommandType::kVideoStreamAllocate:
        ChipLogProgress(Camera, "AVStreamManagement: Command VideoStreamAllocate has been successfully processed.");
        break;

    case CommandType::kVideoStreamDeallocate:
        ChipLogProgress(Camera, "AVStreamManagement: Command VideoStreamDeallocate has been successfully processed.");
        break;

    default:
        ChipLogError(Camera, "AVStreamManagement: Unknown or unhandled command type in OnDone.");
        break;
    }

    // Reset command type to undefined after processing is done
    mCommandType = CommandType::kUndefined;

    // Ensure that mCommandSender is cleaned up after it is done
    mCommandSender.reset();
}

CHIP_ERROR AVStreamManagement::SendCommandForType(CommandType commandType, DeviceProxy * device)
{
    ChipLogProgress(AppServer, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId,
                    static_cast<int>(commandType));

    switch (commandType)
    {
    case CommandType::kVideoStreamAllocate:
        return SendCommand(device, mEndpointId, app::Clusters::CameraAvStreamManagement::Id,
                           app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocate::Id, mVideoStreamAllocate);
    case CommandType::kVideoStreamDeallocate:
        return SendCommand(device, mEndpointId, app::Clusters::CameraAvStreamManagement::Id,
                           app::Clusters::CameraAvStreamManagement::Commands::VideoStreamDeallocate::Id, mVideoStreamDeallocate);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void AVStreamManagement::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             const SessionHandle & sessionHandle)
{
    AVStreamManagement * self = reinterpret_cast<AVStreamManagement *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectedFn: context is null"));

    OperationalDeviceProxy device(&exchangeMgr, sessionHandle);

    CHIP_ERROR err = self->SendCommandForType(self->mCommandType, &device);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send AVStreamManagement command.");
        self->OnDone(nullptr);
    }
}

void AVStreamManagement::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    AVStreamManagement * self = reinterpret_cast<AVStreamManagement *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectedFn: context is null"));
    self->OnDone(nullptr);
}

} // namespace camera
