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

#include <esp_diagnostics_metrics.h>
#include <esp_log.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <string.h>

namespace Insights {

/**
 * This class is used to monotonically increment the counters as per the label of the counter macro
 * 'MATTER_TRACE_COUNTER(label)' and report the metrics to esp-insights.
 * As per the label of the counter macro, it adds the counter in the linked list with the name label if not
 * present and returns the same instance and increments the value if the counter is already present
 * in the list.
 */

class ESPInsightsCounter
{
private:
    static ESPInsightsCounter * mHead; // head of the counter list
    const char * label;                // unique key ,it is used as a static string.
    int instanceCount;
    ESPInsightsCounter * mNext; // pointer to point to the next entry in the list
    bool registered = false;

    ESPInsightsCounter(const char * labelParam) : label(labelParam), instanceCount(1), mNext(nullptr) {}

public:
    static ESPInsightsCounter * GetInstance(const char * label);

    int GetInstanceCount() const;

    void ReportMetrics();
};

} // namespace Insights
