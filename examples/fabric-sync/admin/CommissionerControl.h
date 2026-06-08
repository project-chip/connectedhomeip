/*
 *   Copyright (c) 2024 Project CHIP Authors
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

namespace admin {

/**
 * @class CommissionerControl
 * @brief This class handles sending CHIP commands related to commissioning, including sending
 * commissioning approval requests and commissioning nodes.
 *
 * The class acts as a command sender and implements the `chip::app::CommandSender::Callback` interface
 * to handle responses, errors, and completion events for the commands it sends. It relies on external
 * CCTRL delegate and server mechanisms to manage the overall protocol and state transitions, including
 * processing the CommissioningRequestResult and invoking CommissionNode.
 */
class CommissionerControl : public chip::app::CommandSender::Callback
{
public:
    CommissionerControl() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    /**
     * @brief Initializes the CommissionerControl with a DeviceCommissioner, NodeId, and EndpointId.
     *
     * @param commissioner The DeviceCommissioner to use for the commissioning process.
     * @param nodeId The node ID of the remote fabric bridge.
     * @param endpointId The endpoint on which to send CommissionerControl commands.
     */
    void Init(chip::Controller::DeviceCommissioner & commissioner, chip::NodeId nodeId, chip::EndpointId endpointId);

    /**
     * @brief Sends a RequestCommissioningApproval command to the device.
     *
     * @param requestId The unique request ID.
     * @param vendorId The vendor ID of the device.
     * @param productId The product ID of the device.
     * @param label Optional label for the device.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR RequestCommissioningApproval(uint64_t requestId, uint16_t vendorId, uint16_t productId,
                                            chip::Optional<chip::CharSpan> label);
    /**
     * @brief Sends a CommissionNode command to the device.
     *
     * @param requestId The unique request ID.
     * @param responseTimeoutSeconds Timeout for the response in seconds.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR CommissionNode(uint64_t requestId, uint16_t responseTimeoutSeconds);

    /////////// CommandSender Callback Interface /////////
    virtual void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                            const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override;

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override;

    virtual void OnDone(chip::app::CommandSender * client) override;

private:
    static constexpr uint16_t kMaxDeviceLabelLength = 64;

    enum class CommandType : uint8_t
    {
        kUndefined                    = 0,
        kRequestCommissioningApproval = 1,
        kCommissionNode               = 2,
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
    chip::NodeId mDestinationId  = chip::kUndefinedNodeId;
    chip::EndpointId mEndpointId = chip::kRootEndpointId;
    CommandType mCommandType     = CommandType::kUndefined;
    char mLabelBuffer[kMaxDeviceLabelLength];

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    chip::app::Clusters::CommissionerControl::Commands::RequestCommissioningApproval::Type mRequestCommissioningApproval;
    chip::app::Clusters::CommissionerControl::Commands::CommissionNode::Type mCommissionNode;
};

} // namespace admin
