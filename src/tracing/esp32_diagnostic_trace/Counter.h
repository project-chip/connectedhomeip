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

#pragma once

#include "tracing/esp32_diagnostic_trace/DiagnosticEntry.h"
#include "tracing/esp32_diagnostic_trace/DiagnosticStorage.h"
#include <map>

namespace chip {
namespace Tracing {
namespace Diagnostics {

/**
 * This class is used to monotonically increment the counters as per the label of the counter macro
 * 'MATTER_TRACE_COUNTER(label)'
 * As per the label of the counter macro, it adds the counter in the linked list with the name label if not
 * present and returns the same instance and increments the value if the counter is already present
 * in the list.
 */

class ESPDiagnosticCounter
{
public:
    static ESPDiagnosticCounter & GetInstance()
    {
        static ESPDiagnosticCounter instance;
        return instance;
    }
    static void IncreaseCount(const char * label);

    uint32_t GetInstanceCount(const char * label) const;

    CHIP_ERROR ReportMetrics(const char * label, CircularDiagnosticBuffer * storageInstance);

private:
    ESPDiagnosticCounter() {}
    // Custom comparator for the map to store the label count
    struct StringCompare
    {
        bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; }
    };
    static std::map<const char *, uint32_t, StringCompare> mCounterList;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
