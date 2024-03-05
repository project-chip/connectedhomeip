/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "core/CastingPlayer.h"

namespace matter {
namespace casting {
namespace support {

/**
 * @brief CastingStore uses TLV-encoding to cache casting related data on disk, like the list of CastingPlayers that the
 * tv-casting-app previously connected to.
 */
class CastingStore : public chip::FabricTable::Delegate
{
public:
    static CastingStore * GetInstance();

    /**
     * @brief If castingPlayer already exists in the cache on disk (compares by castingPlayer.id), this will update the record for
     * it in the cache. If castingPlayer is new i.e. not found, this will add it to the cache
     */
    CHIP_ERROR AddOrUpdate(core::CastingPlayer castingPlayer);

    /**
     * @brief Reads and returns a vector of all CastingPlayers found in the cache
     */
    std::vector<core::CastingPlayer> ReadAll();

    /**
     * @brief If castingPlayer is found in the cache, this will delete it. If it is not found, this method is a no-op
     */
    CHIP_ERROR Delete(core::CastingPlayer castingPlayer);

    /**
     * @brief Purges the cache of all CastingStore data
     */
    CHIP_ERROR DeleteAll();

    /**
     * @brief Implements chip::FabricTable::Delegate.OnFabricRemoved. Searches for and deletes the CastingPlayer(s) that match(es)
     * the fabrixIndex argument, if any. If no such CastingPlayer exists, this method is a no-op.
     */
    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override;

private:
    CastingStore();
    static CastingStore * _CastingStore;

    /**
     * @brief Writes the vector of CastingPlayers to the cache. This method will overwrite any pre-existing cached data.
     */
    CHIP_ERROR WriteAll(std::vector<core::CastingPlayer> castingPlayers);

    enum CastingStoreTLVTag
    {
        kCastingStoreDataVersionTag = 1,

        kCastingPlayersContainerTag,
        kCastingPlayerIdTag,
        kCastingPlayerNodeIdTag,
        kCastingPlayerFabricIndexTag,
        kCastingPlayerVendorIdTag,
        kCastingPlayerProductIdTag,
        kCastingPlayerDeviceTypeIdTag,
        kCastingPlayerPortTag,
        kCastingPlayerInstanceNameTag,
        kCastingPlayerDeviceNameTag,
        kCastingPlayerHostNameTag,

        kCastingPlayerEndpointsContainerTag,
        kCastingPlayerEndpointIdTag,
        kCastingPlayerEndpointVendorIdTag,
        kCastingPlayerEndpointProductIdTag,

        kCastingPlayerEndpointDeviceTypeListContainerTag,
        kCastingPlayerEndpointDeviceTypeTag,
        kCastingPlayerEndpointDeviceTypeRevisionTag,

        kCastingPlayerEndpointServerListContainerTag,
        kCastingPlayerEndpointServerClusterIdTag,

        kContextTagMaxNum = UINT8_MAX
    };

    constexpr static size_t kCastingStoreDataMaxBytes               = 1024 * 100; // 100 KBs
    constexpr static char * kCastingStoreDataKey                    = (char *) "com.matter.casting.CastingStore";
    constexpr static uint32_t kCurrentCastingStoreDataVersion       = 1;
    constexpr static uint32_t kSupportedCastingStoreDataVersions[1] = { 1 };
};

}; // namespace support
}; // namespace casting
}; // namespace matter
