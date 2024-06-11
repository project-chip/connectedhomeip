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
 *  This file declares the CHIP API to collect statistics
 *  on the state of CHIP, Inet and System resources
 */

#pragma once

// Include configuration headers
#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>
#include <system/SystemConfig.h>

// Include dependent headers
#include <lib/support/DLLUtil.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/mem.h>
#include <lwip/opt.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <stdint.h>

namespace chip {
namespace System {
namespace Stats {

enum
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP && CHIP_SYSTEM_CONFIG_LWIP_PBUF_FROM_CUSTOM_POOL
#define LWIP_PBUF_MEMPOOL(name, num, payload, desc) kSystemLayer_Num##name,
#include "lwippools.h"
#undef LWIP_PBUF_MEMPOOL
#else
    kSystemLayer_NumPacketBufs,
#endif
    kSystemLayer_NumTimers,
#if INET_CONFIG_NUM_TCP_ENDPOINTS
    kInetLayer_NumTCPEps,
#endif
#if INET_CONFIG_NUM_UDP_ENDPOINTS
    kInetLayer_NumUDPEps,
#endif
    kExchangeMgr_NumContexts,
    kExchangeMgr_NumUMHandlers,
    kPlatformMgr_NumEvents,
    kNumEntries
};

typedef int8_t count_t;
#define CHIP_SYS_STATS_COUNT_MAX INT8_MAX

extern count_t ResourcesInUse[kNumEntries];
extern count_t HighWatermarks[kNumEntries];

class Snapshot
{
public:
    count_t mResourcesInUse[kNumEntries];
    count_t mHighWatermarks[kNumEntries];
};

bool Difference(Snapshot & result, Snapshot & after, Snapshot & before);
void UpdateSnapshot(Snapshot & aSnapshot);
count_t * GetResourcesInUse();
count_t * GetHighWatermarks();

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS
void UpdateLwipPbufCounts(void);
#endif

typedef const char * Label;
const Label * GetStrings();

} // namespace Stats
} // namespace System
} // namespace chip

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS

#define SYSTEM_STATS_INCREMENT(entry)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::count_t new_value = ++(chip::System::Stats::GetResourcesInUse()[entry]);                              \
        if (chip::System::Stats::GetHighWatermarks()[entry] < new_value)                                                           \
        {                                                                                                                          \
            chip::System::Stats::GetHighWatermarks()[entry] = new_value;                                                           \
        }                                                                                                                          \
    } while (0)

#define SYSTEM_STATS_DECREMENT(entry)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::GetResourcesInUse()[entry]--;                                                                         \
    } while (0)

#define SYSTEM_STATS_DECREMENT_BY_N(entry, count)                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::GetResourcesInUse()[entry] -= (count);                                                                \
    } while (0)

#define SYSTEM_STATS_SET(entry, count)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::count_t new_value = chip::System::Stats::GetResourcesInUse()[entry] = (count);                        \
        if (chip::System::Stats::GetHighWatermarks()[entry] < new_value)                                                           \
        {                                                                                                                          \
            chip::System::Stats::GetHighWatermarks()[entry] = new_value;                                                           \
        }                                                                                                                          \
    } while (0)

#define SYSTEM_STATS_RESET(entry)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::GetResourcesInUse()[entry] = 0;                                                                       \
    } while (0)

#if CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS
#define SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS()                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::UpdateLwipPbufCounts();                                                                               \
    } while (0)
#else // CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS
#define SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS()
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_STATS && MEMP_STATS

// Additional macros for testing.
#define SYSTEM_STATS_TEST_IN_USE(entry, expected) (chip::System::Stats::GetResourcesInUse()[entry] == (expected))
#define SYSTEM_STATS_TEST_HIGH_WATER_MARK(entry, expected) (chip::System::Stats::GetHighWatermarks()[entry] == (expected))
#define SYSTEM_STATS_RESET_HIGH_WATER_MARK_FOR_TESTING(entry)                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        chip::System::Stats::GetHighWatermarks()[entry] = 0;                                                                       \
    } while (0)

#else // CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS

#define SYSTEM_STATS_INCREMENT(entry)

#define SYSTEM_STATS_DECREMENT(entry)

#define SYSTEM_STATS_DECREMENT_BY_N(entry, count)

#define SYSTEM_STATS_RESET(entry)

#define SYSTEM_STATS_UPDATE_LWIP_PBUF_COUNTS()

#define SYSTEM_STATS_TEST_IN_USE(entry, expected) (true)
#define SYSTEM_STATS_TEST_HIGH_WATER_MARK(entry, expected) (true)
#define SYSTEM_STATS_RESET_HIGH_WATER_MARK_FOR_TESTING(entry)

#endif // CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
