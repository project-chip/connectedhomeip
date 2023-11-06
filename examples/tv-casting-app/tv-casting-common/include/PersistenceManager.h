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

#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

inline constexpr size_t kMaxCachedVideoPlayers = 32;

class PersistenceManager : public chip::FabricTable::Delegate
{
public:
    CHIP_ERROR AddVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo);

    CHIP_ERROR ReadAllVideoPlayers(TargetVideoPlayerInfo outVideoPlayers[]);

    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex);

    CHIP_ERROR PurgeVideoPlayerCache();

    CHIP_ERROR DeleteVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo);

private:
    CHIP_ERROR WriteAllVideoPlayers(TargetVideoPlayerInfo videoPlayers[]);

    enum CastingTLVTag
    {
        kNodeIdTag = 1,
        kFabricIndexTag,
        kVideoPlayersContainerTag,
        kContentAppEndpointsContainerTag,
        kEndpointIdTag,
        kClusterIdsContainerTag,
        kClusterIdTag,
        kCurrentCastingDataVersionTag,
        kVideoPlayerVendorIdTag,
        kVideoPlayerProductIdTag,
        kVideoPlayerDeviceTypeIdTag,
        kVideoPlayerDeviceNameTag,
        kVideoPlayerHostNameTag,
        kVideoPlayerNumIPsTag,
        kVideoPlayerIPAddressTag,
        kIpAddressesContainerTag,
        kVideoPlayerLastDiscoveredTag,
        kVideoPlayerMACAddressTag,
        kVideoPlayerInstanceNameTag,
        kVideoPlayerPortTag,

        kContextTagMaxNum = UINT8_MAX
    };

    constexpr static size_t kCastingDataMaxBytes               = 1024 * 100; // 100 KBs
    constexpr static char * kCastingDataKey                    = (char *) "com.matter.casting";
    constexpr static uint32_t kCurrentCastingDataVersion       = 1;
    constexpr static uint32_t kSupportedCastingDataVersions[1] = { 1 };
};
