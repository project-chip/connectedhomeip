/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
