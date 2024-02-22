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

#include <string.h>
#include <tracing/esp32_trace/counter.h>

using namespace chip;

namespace Insights {

// This is a one time allocation for counters. It is not supposed to be freed.
ESPInsightsCounter * ESPInsightsCounter::mHead = nullptr;

ESPInsightsCounter * ESPInsightsCounter::GetInstance(const char * label)
{
    ESPInsightsCounter * current = mHead;

    while (current != nullptr)
    {
        if (strcmp(current->label, label) == 0)
        {
            current->instanceCount++;
            return current;
        }
        current = current->mNext;
    }

    // Allocate a new instance if counter is not present in the list.
    void * ptr = Platform::MemoryAlloc(sizeof(ESPInsightsCounter));
    VerifyOrDie(ptr != nullptr);

    ESPInsightsCounter * newInstance = new (ptr) ESPInsightsCounter(label);
    newInstance->mNext               = mHead;
    mHead                            = newInstance;

    return newInstance;
}

int ESPInsightsCounter::GetInstanceCount() const
{
    return instanceCount;
}

void ESPInsightsCounter::ReportMetrics()
{
    if (!registered)
    {
        esp_diag_metrics_register("SYS_CNT" /* Tag of metrics */, label /* Unique key 8 */,
                                  label /* label displayed on dashboard */, "insights.cnt" /* hierarchical path */,
                                  ESP_DIAG_DATA_TYPE_UINT /* data_type */);
        registered = true;
    }
    ESP_LOGI("mtr", "Label = %s Count = %d", label, instanceCount);
    esp_diag_metrics_add_uint(label, instanceCount);
}

} // namespace Insights
