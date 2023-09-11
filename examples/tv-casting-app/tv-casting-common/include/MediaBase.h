/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "TargetVideoPlayerInfo.h"

#include <controller/CHIPCluster.h>

class MediaBase
{
public:
    MediaBase(chip::ClusterId clusterId) { mClusterId = clusterId; }

    CHIP_ERROR SetTarget(TargetVideoPlayerInfo & targetVideoPlayerInfo, chip::EndpointId tvEndpoint)
    {
        auto deviceProxy = targetVideoPlayerInfo.GetOperationalDeviceProxy();
        if (deviceProxy == nullptr)
        {
            ChipLogError(AppServer,
                         "Failed in getting an instance of OperationalDeviceProxy for nodeId: 0x" ChipLogFormatX64
                         ", fabricIndex: %d",
                         ChipLogValueX64(targetVideoPlayerInfo.GetNodeId()), targetVideoPlayerInfo.GetFabricIndex());
            return CHIP_ERROR_PEER_NODE_NOT_FOUND;
        }
        mTargetVideoPlayerInfo = &targetVideoPlayerInfo;
        mTvEndpoint            = tvEndpoint;
        return CHIP_NO_ERROR;
    }

    class MediaClusterBase : public chip::Controller::ClusterBase
    {
    public:
        MediaClusterBase(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
                         chip::EndpointId endpoint) :
            ClusterBase(exchangeManager, session, endpoint)
        {}
    };

protected:
    chip::ClusterId mClusterId;
    TargetVideoPlayerInfo * mTargetVideoPlayerInfo = nullptr;
    chip::EndpointId mTvEndpoint;
};
