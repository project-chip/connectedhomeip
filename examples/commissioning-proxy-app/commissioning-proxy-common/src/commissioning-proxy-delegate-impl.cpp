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
#include <unordered_map>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

namespace {

constexpr uint8_t kDefaultScanMaxTimeSeconds = 5;

// Store per-instance state without requiring member fields in the header.
struct DelegateState
{
    uint8_t scanMaxTime         = kDefaultScanMaxTimeSeconds;
};

static std::unordered_map<const void *, DelegateState> gState;

static DelegateState & GetState(const void * self)
{
    // Creates default state if missing.
    return gState[self];
}

} // namespace

uint8_t MyCPDelegate::GetScanMaxTime()
{
    ChipLogProgress(AppServer, "===SHM %s()", __func__);
    return GetState(this).scanMaxTime;
}

void MyCPDelegate::SetScanMaxTime(uint8_t seconds)
{
    auto & st             = GetState(this);
    st.scanMaxTime = seconds;
    ChipLogProgress(AppServer, "===SHM %s() delegate scanMaxTimeSeconds=%u", 
        __func__, static_cast<unsigned>(seconds));
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
