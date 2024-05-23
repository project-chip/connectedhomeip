/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file Contains functions relating to Content App of the Video Player.
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::AppPlatform;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace AppPlatform {

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_APPLICATION_BASIC_CLUSTER_REVISION (1u)

inline constexpr EndpointId kCastingVideoPlayerEndpointId = 1;

Status ContentApp::HandleReadAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer,
                    "Read Attribute for endpoint " ChipLogFormatMEI " cluster " ChipLogFormatMEI " attribute " ChipLogFormatMEI,
                    ChipLogValueMEI(mEndpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId));

    return Status::Failure;
}

Status ContentApp::HandleWriteAttribute(ClusterId clusterId, AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer,
                    "Read Attribute for endpoint " ChipLogFormatMEI " cluster " ChipLogFormatMEI " attribute " ChipLogFormatMEI,
                    ChipLogValueMEI(mEndpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId));

    return Status::Failure;
}

void ContentApp::AddClientNode(NodeId subjectNodeId)
{
    mClientNodes[mNextClientNodeIndex++] = subjectNodeId;
    if (mClientNodeCount < kMaxClientNodes)
    {
        mClientNodeCount++;
    }
    if (mNextClientNodeIndex >= kMaxClientNodes)
    {
        // if we exceed the max number, then overwrite the oldest entry
        mNextClientNodeIndex = 0;
    }
}

void ContentApp::SendAppObserverCommand(chip::Controller::DeviceCommissioner * commissioner, NodeId clientNodeId, char * data,
                                        char * encodingHint)
{
    ChipLogProgress(Controller, "Attempting to send AppObserver command");
    if (mContentAppClientCommandSender.IsBusy())
    {
        ChipLogProgress(Controller, "SendAppObserverCommand busy");
        return;
    }

    mContentAppClientCommandSender.SendContentAppMessage(commissioner, clientNodeId, kCastingVideoPlayerEndpointId, data,
                                                         encodingHint);

    ChipLogProgress(Controller, "Completed send of AppObserver command");
}

CHIP_ERROR ContentAppClientCommandSender::SendContentAppMessage(chip::Controller::DeviceCommissioner * commissioner,
                                                                chip::NodeId destinationId, chip::EndpointId endPointId,
                                                                char * data, char * encodingHint)
{
    if (mIsBusy)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mIsBusy        = true;
    mDestinationId = destinationId;
    mEndPointId    = endPointId;
    mData          = std::string(data);
    mEncodingHint  = std::string(encodingHint);

    ChipLogProgress(Controller, "Sending command to node 0x" ChipLogFormatX64, ChipLogValueX64(mDestinationId));

    return commissioner->GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void ContentAppClientCommandSender::OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                                        const chip::SessionHandle & sessionHandle)
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::OnDeviceConnectedFn");
    ContentAppClientCommandSender * sender = reinterpret_cast<ContentAppClientCommandSender *>(context);
    VerifyOrReturn(sender != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    sender->SendMessage(exchangeMgr, sessionHandle);
}

CHIP_ERROR ContentAppClientCommandSender::SendMessage(chip::Messaging::ExchangeManager & exchangeMgr,
                                                      const chip::SessionHandle & sessionHandle)
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::SendMessage");

    chip::Controller::ClusterBase cluster(exchangeMgr, sessionHandle, mEndPointId);

    chip::app::Clusters::ContentAppObserver::Commands::ContentAppMessage::Type request;
    request.data         = Optional<CharSpan>(CharSpan::fromCharString(mData.c_str()));
    request.encodingHint = CharSpan::fromCharString(mEncodingHint.c_str());
    CHIP_ERROR err       = cluster.InvokeCommand(request, nullptr, OnCommandResponse, OnCommandFailure);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Controller, "ContentAppClientCommandSender SendMessage error err %s", ErrorStr(err));
    }

    mIsBusy = false;
    ChipLogProgress(Controller, "ContentAppClientCommandSender: Completed send of AppObserver command");

    return CHIP_NO_ERROR;
}

void ContentAppClientCommandSender::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::OnDeviceConnectedFn error err %s", ErrorStr(err));

    ContentAppClientCommandSender * sender = reinterpret_cast<ContentAppClientCommandSender *>(context);
    VerifyOrReturn(sender != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));

    sender->Cleanup();
}

void ContentAppClientCommandSender::Cleanup()
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::Cleanup");
    mIsBusy = false;
}

void ContentAppClientCommandSender::OnCommandResponse(void * context, const ContentAppMessageResponseDecodableType & response)
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::OnCommandResponse");
}

void ContentAppClientCommandSender::OnCommandFailure(void * context, CHIP_ERROR error)
{
    ChipLogProgress(Controller, "ContentAppClientCommandSender::OnCommandFailure error err %s", ErrorStr(error));
}

} // namespace AppPlatform
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
