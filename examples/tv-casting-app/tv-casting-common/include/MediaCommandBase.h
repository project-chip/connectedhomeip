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

#include "TargetVideoPlayerInfo.h"

#include <controller/CHIPCluster.h>
#include <functional>

template <typename RequestType, typename ResponseType>
class MediaCommandBase
{
public:
    MediaCommandBase(chip::ClusterId clusterId) { mClusterId = clusterId; }

    CHIP_ERROR SetTarget(TargetVideoPlayerInfo & mTargetVideoPlayerInfo, chip::EndpointId tvEndpoint)
    {
        mOperationalDeviceProxy = mTargetVideoPlayerInfo.GetOperationalDeviceProxy();
        if (mOperationalDeviceProxy == nullptr)
        {
            ChipLogError(AppServer, "Failed in getting an instance of OperationalDeviceProxy");
            return CHIP_ERROR_PEER_NODE_NOT_FOUND;
        }

        mTvEndpoint = tvEndpoint;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Invoke(RequestType request, std::function<void(CHIP_ERROR)> responseCallback)
    {
        VerifyOrDieWithMsg(mOperationalDeviceProxy != nullptr, AppServer, "Target unknown");

        sResponseCallback = responseCallback;

        class MediaClusterBase : public chip::Controller::ClusterBase
        {
        public:
            MediaClusterBase(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
                             chip::ClusterId cluster, chip::EndpointId endpoint) :
                ClusterBase(exchangeManager, session, cluster, endpoint)
            {}
        };

        MediaClusterBase cluster(*mOperationalDeviceProxy->GetExchangeManager(),
                                 mOperationalDeviceProxy->GetSecureSession().Value(), mClusterId, mTvEndpoint);
        return cluster.InvokeCommand(request, nullptr, OnSuccess, OnFailure);
    }

    static void OnSuccess(void * context, const ResponseType & response) { sResponseCallback(CHIP_NO_ERROR); }

    static void OnFailure(void * context, CHIP_ERROR error) { sResponseCallback(error); }

protected:
    chip::ClusterId mClusterId;
    chip::OperationalDeviceProxy * mOperationalDeviceProxy = nullptr;
    chip::EndpointId mTvEndpoint;
    static std::function<void(CHIP_ERROR)> sResponseCallback;
};

template <typename RequestType, typename ResponseType>
std::function<void(CHIP_ERROR)> MediaCommandBase<RequestType, ResponseType>::sResponseCallback = {};
