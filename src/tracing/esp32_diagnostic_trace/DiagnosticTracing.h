#pragma once

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

#include <lib/core/CHIPError.h>
#include <tracing/backend.h>
#include <tracing/metric_event.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticStorageManager.h>
#include <esp_log.h>


#include <memory>
namespace chip {
namespace Tracing {
namespace Diagnostics {
/// A Backend that outputs data to chip logging.
///
/// Structured data is formatted as json strings.
class ESP32Diagnostics : public ::chip::Tracing::Backend
{
public:
    ESP32Diagnostics(uint8_t *buffer, size_t buffer_size)
    {
        DiagnosticStorageImpl::GetInstance(buffer, buffer_size);
    }

    // Deleted copy constructor and assignment operator to prevent copying
    ESP32Diagnostics(const ESP32Diagnostics&) = delete;
    ESP32Diagnostics& operator=(const ESP32Diagnostics&) = delete;

    void TraceBegin(const char * label, const char * group) override;

    void TraceEnd(const char * label, const char * group) override;

    /// Trace a zero-sized event
    void TraceInstant(const char * label, const char * group) override;

    void TraceCounter(const char * label) override;

    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;

    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
    void LogMetricEvent(const MetricEvent &) override;
    void StoreDiagnostics(const char* label, const char* group);

private:
    using ValueType = MetricEvent::Value::Type;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip