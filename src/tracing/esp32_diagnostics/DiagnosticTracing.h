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

#include <tracing/backend.h>
#include <tracing/esp32_diagnostics/DiagnosticStorage.h>
#include <tracing/metric_event.h>
#include <unordered_set>

namespace chip {
namespace Tracing {
namespace Diagnostics {
/// A Backend that stores data to storage instance
///
/// Structured data is formatted as TLV.
class ESP32Diagnostics : public ::chip::Tracing::Backend
{
public:
    ESP32Diagnostics(CircularDiagnosticBuffer * storageInstance);

    // Deleted copy constructor and assignment operator to prevent copying
    ESP32Diagnostics(const ESP32Diagnostics &)             = delete;
    ESP32Diagnostics & operator=(const ESP32Diagnostics &) = delete;

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTIC_TRACES
    void TraceBegin(const char * label, const char * group) override;

    void TraceEnd(const char * label, const char * group) override;

    /// Trace a zero-sized event
    void TraceInstant(const char * label, const char * group) override;
#endif // CONFIG_ENABLE_ESP_DIAGNOSTIC_TRACES

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTIC_METRICS
    void TraceCounter(const char * label) override;

    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;

    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
    void LogMetricEvent(const MetricEvent &) override;
#endif // CONFIG_ENABLE_ESP_DIAGNOSTIC_METRICS

    /*
     * @brief Add a filter to the diagnostic backend. Only traces and metrics with the given scope will be stored while other
     * scopes will be ignored.
     * @param scope The scope to filter
     * @return CHIP_ERROR_INVALID_ARGUMENT if the scope is invalid
     * @return CHIP_NO_ERROR if the filter already exists or added successfully
     */
    CHIP_ERROR AddFilter(const char * scope);

    /*
     * @brief Remove a filter from the diagnostic backend
     * @param scope The scope to remove
     * @return CHIP_ERROR_INVALID_ARGUMENT if the scope is invalid, CHIP_ERROR_INCORRECT_STATE if the filter does not exist
     * @return CHIP_NO_ERROR if the filter was removed successfully
     */
    CHIP_ERROR RemoveFilter(const char * scope);

    /*
     * @brief Clear all filters from the diagnostic backend
     */
    void ClearFilters();

    /*
     * @brief Check if the scope is enabled
     * @param scope The scope to check
     * @return true if the scope is enabled or if no filters are set, false otherwise
     */
    bool IsEnabled(const char * scope);

private:
    using ValueType = MetricEvent::Value::Type;
    CircularDiagnosticBuffer * mStorageInstance;
    std::unordered_set<uint32_t> mEnabledFilters;

    /*
     * @brief Store the diagnostics in the storage instance
     * @param label The trace label
     * @param group The trace group/scope
     * @return CHIP_ERROR_INVALID_ARGUMENT if the label or group is invalid
     * @return CHIP_NO_ERROR if the diagnostics were stored successfully
     */
    CHIP_ERROR StoreDiagnostics(const char * label, const char * group);

    /*
     * @brief Initialize the default filters
     */
    void InitializeDefaultFilters();
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
