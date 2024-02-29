/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "ContentAppClientCommandSender.h"

using namespace ::chip;

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

    ChipLogProgress(Controller, "Sending command to node 0x%" PRIx64, mDestinationId);

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
