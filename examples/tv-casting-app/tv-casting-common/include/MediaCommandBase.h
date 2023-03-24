/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaBase.h"

#include <functional>

template <typename RequestType, typename ResponseType>
class MediaCommandBase : public MediaBase
{
public:
    MediaCommandBase(chip::ClusterId clusterId) : MediaBase(clusterId) {}

    CHIP_ERROR Invoke(RequestType request, std::function<void(CHIP_ERROR)> responseCallback)
    {
        VerifyOrDieWithMsg(mTargetVideoPlayerInfo != nullptr, AppServer, "Target unknown");

        auto deviceProxy = mTargetVideoPlayerInfo->GetOperationalDeviceProxy();
        ReturnErrorCodeIf(deviceProxy == nullptr || !deviceProxy->ConnectionReady(), CHIP_ERROR_PEER_NODE_NOT_FOUND);

        sResponseCallback = responseCallback;

        MediaClusterBase cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), mTvEndpoint);
        return cluster.InvokeCommand(request, nullptr, OnSuccess, OnFailure);
    }

    static void OnSuccess(void * context, const ResponseType & response) { sResponseCallback(CHIP_NO_ERROR); }

    static void OnFailure(void * context, CHIP_ERROR error) { sResponseCallback(error); }

protected:
    static std::function<void(CHIP_ERROR)> sResponseCallback;
};

template <typename RequestType, typename ResponseType>
std::function<void(CHIP_ERROR)> MediaCommandBase<RequestType, ResponseType>::sResponseCallback = {};
