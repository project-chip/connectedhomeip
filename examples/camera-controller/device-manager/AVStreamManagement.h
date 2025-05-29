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

#pragma once

#include <app/CommandSender.h>
#include <controller/CHIPDeviceController.h>

namespace camera {

/**
 * @class AVStreamManagement
 * @brief This class handles sending CHIP commands to manage, control, and configure various audio, video,
 * and snapshot streams on a camera.
 *
 * The class acts as a command sender and implements the `chip::app::CommandSender::Callback` interface
 * to handle responses, errors, and completion events for the commands it sends.
 */
class AVStreamManagement : public chip::app::CommandSender::Callback
{
public:
    AVStreamManagement() :
        mOnConnectedCallback(OnDeviceConnectedFn, this), mOnConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    /**
     * @brief Initializes the AVStreamManagement with a DeviceCommissioner.
     *
     * @param commissioner The DeviceCommissioner to use for the commissioning process.
     */
    void Init(chip::Controller::DeviceCommissioner * commissioner);

    /**
     * @brief Sends a VideoStreamAllocate command to the device.
     *
     * @param nodeId      The node ID of the remote camera device.
     * @param endpointId  The endpoint on which to send VideoStreamAllocate commands.
     * @param streamUsage The usage of the stream(Recording, LiveView, etc) that this allocation is for.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR AllocateVideoStream(chip::NodeId nodeId, chip::EndpointId endpointId, uint8_t streamUsage);

    /**
     * @brief Sends a VideoStreamDeallocate command to the device.
     *
     * @param nodeId        The node ID of the remote camera device.
     * @param endpointId    The endpoint on which to send VideoStreamDeallocate commands.
     * @param videoStreamID The VideoStreamID for the stream to be deallocated.
     * @return CHIP_ERROR   CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR DeallocateVideoStream(chip::NodeId nodeId, chip::EndpointId endpointId, uint16_t videoStreamID);

    /////////// CommandSender Callback Interface /////////
    virtual void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                            const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override;

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override;

    virtual void OnDone(chip::app::CommandSender * client) override;

private:
    enum class CommandType : uint8_t
    {
        kUndefined             = 0,
        kVideoStreamAllocate   = 1,
        kVideoStreamDeallocate = 2,
    };

    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value)
    {
        chip::app::CommandPathParams commandPath = { endpointId, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kEndpointIdValid) };
        mCommandSender = std::make_unique<chip::app::CommandSender>(this, device->GetExchangeManager(), false, false,
                                                                    device->GetSecureSession().Value()->AllowsLargePayload());

        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::app::CommandSender::AddRequestDataParameters addRequestDataParams(chip::NullOptional);
        ReturnErrorOnFailure(mCommandSender->AddRequestData(commandPath, value, addRequestDataParams));
        ReturnErrorOnFailure(mCommandSender->SendCommandRequest(device->GetSecureSession().Value()));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendCommandForType(CommandType commandType, chip::DeviceProxy * device);

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    // Private data members
    chip::Controller::DeviceCommissioner * mCommissioner = nullptr;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;
    chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
    CommandType mCommandType     = CommandType::kUndefined;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;

    chip::app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocate::Type mVideoStreamAllocate;
    chip::app::Clusters::CameraAvStreamManagement::Commands::VideoStreamDeallocate::Type mVideoStreamDeallocate;
};

} // namespace camera
