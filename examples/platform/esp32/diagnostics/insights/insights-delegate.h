/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <esp_heap_caps.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <map>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostics/DiagnosticEntry.h>
#include <tracing/esp32_diagnostics/DiagnosticStorage.h>

namespace chip {
namespace Insights {

struct InsightsInitParams
{
    uint8_t * diagnosticBuffer;
    size_t diagnosticBufferSize;
    const char * authKey;
};

/**
 * @brief Delegate interface for handling diagnostic data operations
 */
class InsightsDelegate
{
public:
    static inline InsightsDelegate & GetInstance()
    {
        static InsightsDelegate sInstance;
        return sInstance;
    }

    CHIP_ERROR Init(InsightsInitParams & initParams);

    /**
     * @brief Start periodic insights for a given timeout
     * Get diagnostic data from storage and send to insights
     *
     * @param aTimeout
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise
     */
    CHIP_ERROR StartPeriodicInsights(chip::System::Clock::Timeout aTimeout);

    /**
     * @brief Stop periodic insights
     * Stop sending diagnostic data to insights
     * Cancel the timer for sending diagnostic data to insights
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise
     */
    CHIP_ERROR StopPeriodicInsights();

    /**
     * @brief Set sampling interval for sending diagnostic data to insights
     *
     * @param aTimeout Sampling interval
     * @return CHIP_ERROR CHIP_NO_ERROR on success, error code otherwise
     */
    CHIP_ERROR SetSamplingInterval(chip::System::Clock::Timeout aTimeout);

private:
    Tracing::Diagnostics::CircularDiagnosticBuffer * mStorageInstance = nullptr;
    System::Clock::Timeout mTimeout                                   = System::Clock::kZero;
    std::map<std::string, chip::Tracing::Diagnostics::ValueType> mRegisteredMetrics;

    CHIP_ERROR SendInsightsData();
    void LogTraceData(const chip::Tracing::Diagnostics::DiagnosticEntry & entry);
    void LogMetricData(const chip::Tracing::Diagnostics::DiagnosticEntry & entry);
    void RegisterMetric(const std::string & key, chip::Tracing::Diagnostics::ValueType type);
    static void InsightsHandler(chip::System::Layer * systemLayer, void * context);
};

} // namespace Insights
} // namespace chip
