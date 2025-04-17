#pragma once

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

#include <tracing/backend.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticStorage.h>
#include <tracing/metric_event.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {
/// A Backend that stores data to storage instance
///
/// Structured data is formatted as TLV.
class ESP32Diagnostics : public ::chip::Tracing::Backend
{
public:
    ESP32Diagnostics(CircularDiagnosticBuffer * storageInstance) : mStorageInstance(storageInstance) {}

    // Deleted copy constructor and assignment operator to prevent copying
    ESP32Diagnostics(const ESP32Diagnostics &)             = delete;
    ESP32Diagnostics & operator=(const ESP32Diagnostics &) = delete;

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

private:
    using ValueType = MetricEvent::Value::Type;
    CircularDiagnosticBuffer * mStorageInstance;
    CHIP_ERROR StoreDiagnostics(const char * label, const char * group);
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
