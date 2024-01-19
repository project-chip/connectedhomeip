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

#include <tracing/esp32_trace/counter.h>

namespace Insights {
ESPInsightsCounter * ESPInsightsCounter::mHead = nullptr;

ESPInsightsCounter * ESPInsightsCounter::GetInstance(const char * label, const char * group)
{

    ESPInsightsCounter * current  = mHead;
    ESPInsightsCounter * previous = nullptr;

    while (current != nullptr)
    {
        if (strcmp(current->label, label) == 0 && strcmp(current->group, group) == 0)
        {
            current->instanceCount++;
            return current;
        }
        previous = current;
        current  = current->mNext;
    }

    ESPInsightsCounter * newInstance = new ESPInsightsCounter(label, group);
    newInstance->mNext               = mHead;
    mHead                            = newInstance;
    return newInstance;
}

int ESPInsightsCounter::GetInstanceCount() const
{
    return instanceCount;
}

} // namespace Insights
