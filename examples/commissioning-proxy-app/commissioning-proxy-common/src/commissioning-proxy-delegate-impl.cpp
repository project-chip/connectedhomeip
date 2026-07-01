/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "commissioning-proxy-delegate-impl.h"
#include <cstdint>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {

constexpr uint8_t kMaxCachedResults = 10;

} // namespace

uint8_t MyCPDelegate::GetScanMaxTime()
{
    return mScanMaxTime;
}

void MyCPDelegate::SetScanMaxTime(uint8_t seconds)
{
    mScanMaxTime = seconds;
}

uint8_t MyCPDelegate::GetMaxCachedResults()
{
    return kMaxCachedResults;
}

uint16_t MyCPDelegate::GetCacheTimeout()
{
    return mCacheTimeout;
}

void MyCPDelegate::SetCacheTimeout(uint16_t seconds)
{
    mCacheTimeout = seconds;
}

chip::BitMask<WiFiBandBitmap> MyCPDelegate::GetSupportedWiFiBands()
{
    return mSupportedWiFiBands;
}

void MyCPDelegate::SetSupportedWiFiBands(chip::BitMask<WiFiBandBitmap> bands)
{
    mSupportedWiFiBands = bands;
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
