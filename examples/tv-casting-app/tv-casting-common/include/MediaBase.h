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
