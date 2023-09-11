/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "MediaBase.h"

template <typename TypeInfo>
class MediaReadBase : public MediaBase
{
public:
    MediaReadBase(chip::ClusterId clusterId) : MediaBase(clusterId) {}

    CHIP_ERROR ReadAttribute(void * context,
                             chip::Controller::ReadResponseSuccessCallback<typename TypeInfo::DecodableArgType> successFn,
                             chip::Controller::ReadResponseFailureCallback failureFn)
    {
        VerifyOrDieWithMsg(mTargetVideoPlayerInfo != nullptr, AppServer, "Target unknown");

        auto deviceProxy = mTargetVideoPlayerInfo->GetOperationalDeviceProxy();
        ReturnErrorCodeIf(deviceProxy == nullptr || !deviceProxy->ConnectionReady(), CHIP_ERROR_PEER_NODE_NOT_FOUND);

        MediaClusterBase cluster(*deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), mTvEndpoint);

        return cluster.template ReadAttribute<TypeInfo>(context, successFn, failureFn);
    }
};
