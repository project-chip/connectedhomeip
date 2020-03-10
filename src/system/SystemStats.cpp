/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 * @file
 *  This file implements the Weave API to collect statistics
 *  on the state of Weave, Inet and System resources
 */

// Include module header
#include <SystemLayer/SystemStats.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <SystemLayer/SystemTimer.h>

#include <string.h>

namespace nl {
namespace Weave {
namespace System {
namespace Stats {

static const Label sStatsStrings[nl::Weave::System::Stats::kNumEntries] =
{
#if WEAVE_SYSTEM_CONFIG_USE_LWIP && LWIP_PBUF_FROM_CUSTOM_POOLS
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
#if INET_CONFIG_NUM_TUN_ENDPOINTS
    "InetLayer_NumTunEpsInUse",
#endif
#if INET_CONFIG_NUM_DNS_RESOLVERS
    "InetLayer_NumDNSResolversInUse",
#endif
    "ExchangeMgr_NumContextsInUse",
    "ExchangeMgr_NumUMHandlersInUse",
    "ExchangeMgr_NumBindings",
    "MessageLayer_NumConnectionsInUse",
#if WEAVE_CONFIG_ENABLE_SERVICE_DIRECTORY
    "ServiceMgr_NumRequestsInUse",
#endif

#if WDM_ENABLE_SUBSCRIPTION_PUBLISHER
    "WDM_NumTraits",
#endif
#if WDM_ENABLE_SUBSCRIPTION_CLIENT
    "WDM_NumSubscriptionClients",
#endif
#if WDM_ENABLE_SUBSCRIPTION_PUBLISHER
    "WDM_NumSubscriptionHandlers",
#endif
#if WDM_PUBLISHER_ENABLE_CUSTOM_COMMANDS
    "WDM_NumCommands",
#endif

#if WEAVE_CONFIG_LEGACY_WDM
    "WDMLegacy_NumViewInUse",
#if WEAVE_CONFIG_WDM_ALLOW_CLIENT_SUBSCRIPTION
    "WDMLegacy_NumSubscribeInUse",
    "WDMLegacy_NumCancelInUse",
#endif // WEAVE_CONFIG_WDM_ALLOW_CLIENT_SUBSCRIPTION
    "WDMLegacy_NumUpdateInUse",
    "WDMLegacy_NumBindingsInUse",
    "WDMLegacy_NumTransactions",
#endif // WEAVE_CONFIG_LEGACY_WDM

};

count_t sResourcesInUse[kNumEntries];
count_t sHighWatermarks[kNumEntries];

const Label *GetStrings(void)
{
    return sStatsStrings;
}

count_t *GetResourcesInUse(void)
{
    return sResourcesInUse;
}

count_t *GetHighWatermarks(void)
{
    return sHighWatermarks;
}

void UpdateSnapshot(Snapshot &aSnapshot)
{
    memcpy(&aSnapshot.mResourcesInUse, &sResourcesInUse, sizeof(aSnapshot.mResourcesInUse));
    memcpy(&aSnapshot.mHighWatermarks, &sHighWatermarks, sizeof(aSnapshot.mHighWatermarks));

    nl::Weave::System::Timer::GetStatistics(aSnapshot.mResourcesInUse[kSystemLayer_NumTimers],
                                            aSnapshot.mHighWatermarks[kSystemLayer_NumTimers]);

    SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS();
}

bool Difference(Snapshot &result, Snapshot &after, Snapshot &before)
{
    int i;
    bool leak = false;

    for (i = 0; i < kNumEntries; i++)
    {
        result.mResourcesInUse[i] = after.mResourcesInUse[i] - before.mResourcesInUse[i];
        result.mHighWatermarks[i] = after.mHighWatermarks[i] - before.mHighWatermarks[i];

        if (result.mResourcesInUse[i] > 0)
        {
            leak = true;
        }
    }

    return leak;
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS
void UpdateLwipPbufCounts(void)
{
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    size_t lwip_pool_idx = PBUF_CUSTOM_POOL_IDX_END;
    size_t system_idx = 0;

    while (lwip_pool_idx <= PBUF_CUSTOM_POOL_IDX_START)
    {
        nl::Weave::System::Stats::GetResourcesInUse()[system_idx] = MEMP_STATS_GET(used, lwip_pool_idx);
        nl::Weave::System::Stats::GetHighWatermarks()[system_idx] = MEMP_STATS_GET(max, lwip_pool_idx);
        lwip_pool_idx++;
        system_idx++;
    }

#else // LWIP_PBUF_FROM_CUSTOM_POOLS

    nl::Weave::System::Stats::GetResourcesInUse()[kSystemLayer_NumPacketBufs] = MEMP_STATS_GET(used, MEMP_PBUF_POOL);
    nl::Weave::System::Stats::GetHighWatermarks()[kSystemLayer_NumPacketBufs] = MEMP_STATS_GET(max, MEMP_PBUF_POOL);

#endif // LWIP_PBUF_FROM_CUSTOM_POOLS
}
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS


} // namespace Stats
} // namespace System
} // namespace Weave
} // namespace nl
