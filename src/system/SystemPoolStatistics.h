/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <algorithm>
#include <atomic>

#include <system/SystemStats.h>

namespace chip {
namespace System {

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS

class ObjectPoolStatistics
{
public:
    void ResetStatistics() { mHighWatermark.store(mUsage.load()); }

    void GetStatistics(Stats::count_t & aNumInUse, Stats::count_t & aHighWatermark)
    {
        aNumInUse = static_cast<Stats::count_t>(std::min<unsigned int>(Stats::CHIP_SYS_STATS_COUNT_MAX, mUsage.load()));
        aHighWatermark =
            static_cast<Stats::count_t>(std::min<unsigned int>(Stats::CHIP_SYS_STATS_COUNT_MAX, mHighWatermark.load()));
    }

protected:
    void IncreaseUsage()
    {
        unsigned int usage = ++mUsage;
        unsigned int prev  = mHighWatermark;
        while (prev < usage && !mHighWatermark.compare_exchange_weak(prev, usage))
        {
        }
    }

    void DecreaseUsage() { --mUsage; }

private:
    std::atomic<unsigned int> mUsage;
    std::atomic<unsigned int> mHighWatermark;
};

#else

class ObjectPoolStatistics
{
public:
    void ResetStatistics() {}
    void GetStatistics(Stats::count_t & aNumInUse, Stats::count_t & aHighWatermark) {}

protected:
    void IncreaseUsage() {}
    void DecreaseUsage() {}
};

#endif

} // namespace System
} // namespace chip
