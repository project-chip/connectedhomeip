/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "TargetEndpointInfo.h"
#include "TargetVideoPlayerInfo.h"

constexpr size_t kMaxCachedVideoPlayers = 32;

class PersistenceManager : public chip::FabricTable::Delegate
{
public:
    CHIP_ERROR AddVideoPlayer(TargetVideoPlayerInfo * targetVideoPlayerInfo);

    CHIP_ERROR ReadAllVideoPlayers(TargetVideoPlayerInfo outVideoPlayers[]);

    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex);

    CHIP_ERROR PurgeVideoPlayerCache();

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

        kContextTagMaxNum = UINT8_MAX
    };

    constexpr static size_t kCastingDataMaxBytes               = 1024 * 100; // 100 KBs
    constexpr static char * kCastingDataKey                    = (char *) "com.matter.casting";
    constexpr static uint32_t kCurrentCastingDataVersion       = 1;
    constexpr static uint32_t kSupportedCastingDataVersions[1] = { 1 };
};
