/*
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

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <system/SystemStats.h>
namespace chip {
namespace System {
namespace Stats {

class InsightsSystemMetrics
{
public:
    static InsightsSystemMetrics & GetInstance();

    /*
     * This api registers the system metrics to the insights and starts the
     * timer to enable system stats periodically to the insights.
     */
    CHIP_ERROR RegisterAndEnable(chip::System::Clock::Timeout aTimeout);

    /*
     * This api unregisters the system stats which are registered
     * as metrics to the esp-insights.
     */
    CHIP_ERROR Unregister(intptr_t arg);

    /*
     * This api cancels the timeout providing the user the flexibility
     * to increase or decrease the frequency of sampling the System
     * Stats. It cancels the timer if new timeout value is zero.
     * If the value of timeout differs from existing value, then
     * it cancels the previous timer and starts a new timer.
     */
    CHIP_ERROR SetSamplingInterval(chip::System::Clock::Timeout aTimeout);

    System::Clock::Timeout GetSamplingInterval() { return mTimeout; }

private:
    InsightsSystemMetrics() {}
    static constexpr int kMaxStringLength = 16;
    bool mRegistered                      = false;
    static constexpr char kPath[]         = "sys.mtr";
    static constexpr char kTag[]          = "MTR";
    System::Clock::Timeout mTimeout;
    char * mLabels[chip::System::Stats::kNumEntries];

    static void SamplingHandler(System::Layer * systemLayer, void * context);
    static void SetSamplingHandler(intptr_t arg);
};

} // namespace Stats
} // namespace System
} // namespace chip
