/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
        VerifyOrDieWithMsg(mTargetVideoPlayerInfo != nullptr, AppServer, "Target unknown");

        auto deviceProxy = mTargetVideoPlayerInfo->GetOperationalDeviceProxy();
        ReturnErrorCodeIf(deviceProxy == nullptr || !deviceProxy->ConnectionReady(), CHIP_ERROR_PEER_NODE_NOT_FOUND);

        MediaClusterBase cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), mTvEndpoint);

        return cluster.template SubscribeAttribute<TypeInfo>(context, successFn, failureFn, minInterval, maxInterval,
                                                             onSubscriptionEstablished, nullptr /* resubscribeCb */,
                                                             true /* fabricFiltered */, true /* keepPreviousSubscriptions */);
    }
};
