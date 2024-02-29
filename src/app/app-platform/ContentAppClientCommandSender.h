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

#include <controller/CHIPDeviceController.h>

class ContentAppClientCommandSender
{
public:
    ContentAppClientCommandSender() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {}

    bool IsBusy() const { return mIsBusy; }
    CHIP_ERROR SendContentAppMessage(chip::Controller::DeviceCommissioner * commissioner, chip::NodeId destinationId,
                                     chip::EndpointId endPointId, char * data, char * encodingHint);

protected:
    CHIP_ERROR SendMessage(chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    void Cleanup();

private:
    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    using ContentAppMessageResponseDecodableType =
        chip::app::Clusters::ContentAppObserver::Commands::ContentAppMessageResponse::DecodableType;

    static void OnCommandResponse(void * context, const ContentAppMessageResponseDecodableType & response);
    static void OnCommandFailure(void * context, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    bool mIsBusy                 = false;
    chip::NodeId mDestinationId  = 0;
    chip::EndpointId mEndPointId = 0;

    std::string mData;
    std::string mEncodingHint;
};
