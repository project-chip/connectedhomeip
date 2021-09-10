/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 * @file
 *  This file implements the CHIP API to collect statistics
 *  on the state of CHIP, Inet and System resources
 */

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <system/SystemTimer.h>

// Include module header
#include <system/SystemStats.h>

#include <lib/support/SafeInt.h>

#include <string.h>

namespace chip {
namespace System {
namespace Stats {

static const Label sStatsStrings[chip::System::Stats::kNumEntries] = {
#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
#define LWIP_PBUF_MEMPOOL(name, num, payload, desc) "SystemLayer_Num" desc,
#include "lwippools.h"
#undef LWIP_PBUF_MEMPOOL
#else
    "SystemLayer_NumPacketBufs",
#endif
    "SystemLayer_NumTimersInUse",
#if INET_CONFIG_NUM_RAW_ENDPOINTS
    "InetLayer_NumRawEpsInUse",
#endif
#if INET_CONFIG_NUM_TCP_ENDPOINTS
    "InetLayer_NumTCPEpsInUse",
#endif
#if INET_CONFIG_NUM_UDP_ENDPOINTS
    "InetLayer_NumUDPEpsInUse",
#endif
#if INET_CONFIG_NUM_DNS_RESOLVERS
    "InetLayer_NumDNSResolversInUse",
#endif
    "ExchangeMgr_NumContextsInUse",   "ExchangeMgr_NumUMHandlersInUse",
    "ExchangeMgr_NumBindings",        "MessageLayer_NumConnectionsInUse",
};

count_t sResourcesInUse[kNumEntries];
count_t sHighWatermarks[kNumEntries];

const Label * GetStrings()
{
    return sStatsStrings;
}

count_t * GetResourcesInUse()
{
    return sResourcesInUse;
}

count_t * GetHighWatermarks()
{
    return sHighWatermarks;
}

void UpdateSnapshot(Snapshot & aSnapshot)
{
    memcpy(&aSnapshot.mResourcesInUse, &sResourcesInUse, sizeof(aSnapshot.mResourcesInUse));
    memcpy(&aSnapshot.mHighWatermarks, &sHighWatermarks, sizeof(aSnapshot.mHighWatermarks));

#if CHIP_SYSTEM_CONFIG_USE_TIMER_POOL
    chip::System::Timer::GetStatistics(aSnapshot.mResourcesInUse[kSystemLayer_NumTimers],
                                       aSnapshot.mHighWatermarks[kSystemLayer_NumTimers]);
#endif // CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

    SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
}

bool Difference(Snapshot & result, Snapshot & after, Snapshot & before)
{
    int i;
    bool leak = false;

    for (i = 0; i < kNumEntries; i++)
    {
        // TODO: These casts can be bogus.  https://github.com/project-chip/connectedhomeip/issues/2949
        result.mResourcesInUse[i] = static_cast<count_t>(after.mResourcesInUse[i] - before.mResourcesInUse[i]);
        result.mHighWatermarks[i] = static_cast<count_t>(after.mHighWatermarks[i] - before.mHighWatermarks[i]);

        if (result.mResourcesInUse[i] > 0)
        {
            leak = true;
        }
    }

    return leak;
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS
void UpdateLwipPbufCounts(void)
{
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    size_t lwip_pool_idx = PBUF_CUSTOM_POOL_IDX_END;
    size_t system_idx    = 0;

    while (lwip_pool_idx <= PBUF_CUSTOM_POOL_IDX_START)
    {
        chip::System::Stats::GetResourcesInUse()[system_idx] = MEMP_STATS_GET(used, lwip_pool_idx);
        chip::System::Stats::GetHighWatermarks()[system_idx] = MEMP_STATS_GET(max, lwip_pool_idx);
        lwip_pool_idx++;
        system_idx++;
    }

#else // LWIP_PBUF_FROM_CUSTOM_POOLS

    chip::System::Stats::GetResourcesInUse()[kSystemLayer_NumPacketBufs] = MEMP_STATS_GET(used, MEMP_PBUF_POOL);
    chip::System::Stats::GetHighWatermarks()[kSystemLayer_NumPacketBufs] = MEMP_STATS_GET(max, MEMP_PBUF_POOL);

#endif // LWIP_PBUF_FROM_CUSTOM_POOLS
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS

} // namespace Stats
} // namespace System
} // namespace chip
