/*
 *
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

#include <esp_diagnostics_metrics.h>
#include <esp_log.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>

#define PATH "insights.cnt"

namespace Insights {
class ESPInsightsCounter
{
private:
    static ESPInsightsCounter * mHead; // head of the counter list
    char label[50];                    // unique key
    char group[50];
    int instanceCount;
    ESPInsightsCounter * mNext; // pointer to point to the next entry in the list
    bool registered = false;

    ESPInsightsCounter(const char * labelParam, const char * groupParam) : instanceCount(1), mNext(nullptr)
    {
        chip::Platform::CopyString(label, sizeof(label), labelParam);
        chip::Platform::CopyString(group, sizeof(group), groupParam);
    }

public:
    static ESPInsightsCounter * GetInstance(const char * label, const char * group);

    int GetInstanceCount() const;

    void ReportMetrics()
    {
        if (!registered)
        {
            esp_diag_metrics_register("SYS_CNT" /* Tag of metrics */, label /* Unique key 8 */,
                                      label /* label displayed on dashboard */, PATH /* hierarchical path */,
                                      ESP_DIAG_DATA_TYPE_UINT /* data_type */);
            registered = true;
        }
        ESP_LOGI("mtr", "Label = %s Count = %d", label, instanceCount);
        esp_diag_metrics_add_uint(label, instanceCount);
    }
};

} // namespace Insights
