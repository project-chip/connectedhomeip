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
#include <string.h>

#define PATH1 "sys.cnt"
namespace Insights {
class ESPInsightsCounter
{
private:
    static ESPInsightsCounter * mHead;
    char label[50];
    char group[50];
    int instanceCount;
    ESPInsightsCounter * mNext;
    bool registered = false;

    ESPInsightsCounter(const char * labelParam, const char * groupParam) : instanceCount(1), mNext(nullptr)
    {
        strncpy(label, labelParam, sizeof(label));
        strncpy(group, groupParam, sizeof(group));
    }

public:
    static ESPInsightsCounter * GetInstance(const char * label, const char * group);

    int GetInstanceCount() const;

    void ReportMetrics()
    {
        if (!registered)
        {
            esp_diag_metrics_register("SYS_CNT", label, label, PATH1, ESP_DIAG_DATA_TYPE_UINT);
            registered = true;
        }
        ESP_LOGI("Mtr", "Label = %s Count = %d", label, instanceCount);
        esp_diag_metrics_add_uint(label, instanceCount);
    }
};

} // namespace Insights
