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

template <typename TypeInfo>
class MediaSubscriptionBase : public MediaBase
{
public:
    MediaSubscriptionBase(chip::ClusterId clusterId) : MediaBase(clusterId) {}

    CHIP_ERROR SubscribeAttribute(void * context,
                                  chip::Controller::ReadResponseSuccessCallback<typename TypeInfo::DecodableArgType> successFn,
                                  chip::Controller::ReadResponseFailureCallback failureFn, uint16_t minInterval,
                                  uint16_t maxInterval, chip::Controller::SubscriptionEstablishedCallback onSubscriptionEstablished)
    {
        ReturnErrorCodeIf(mTargetVideoPlayerInfo == nullptr, CHIP_ERROR_PEER_NODE_NOT_FOUND);

        auto deviceProxy = mTargetVideoPlayerInfo->GetOperationalDeviceProxy();
        ReturnErrorCodeIf(deviceProxy == nullptr || !deviceProxy->ConnectionReady(), CHIP_ERROR_PEER_NODE_NOT_FOUND);
        ReturnErrorCodeIf(!deviceProxy->GetSecureSession().HasValue(), CHIP_ERROR_MISSING_SECURE_SESSION);
        const chip::SessionHandle & sessionHandle = deviceProxy->GetSecureSession().Value();
        ReturnErrorCodeIf(!sessionHandle->IsSecureSession(), CHIP_ERROR_MISSING_SECURE_SESSION);
        ReturnErrorCodeIf(sessionHandle->AsSecureSession()->IsDefunct(), CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY);

        MediaClusterBase cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), mClusterId,
                                 mTvEndpoint);

        return cluster.template SubscribeAttribute<TypeInfo>(context, successFn, failureFn, minInterval, maxInterval,
                                                             onSubscriptionEstablished, nullptr /* resubscribeCb */,
                                                             true /* fabricFiltered */, true /* keepPreviousSubscriptions */);
    }
};
