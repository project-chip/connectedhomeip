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

#include "esp32_tracing.h"
#include <esp_heap_caps.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <memory>
#include <tracing/backend.h>

namespace chip {
namespace Tracing {
namespace Insights {

#define LOG_HEAP_INFO(label, group, entry_exit)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        ESP_DIAG_EVENT("MTR_TRC", "%s - %s - %s Min Free heap - %u - LFB - %u Start free heap - %u", entry_exit, label, group,     \
                       heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),                                                           \
                       heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),                                    \
                       heap_caps_get_free_size(MALLOC_CAP_8BIT));                                                                  \
    } while (0)

void ESP32Backend::LogMessageReceived(MessageReceivedInfo & info) {}

void ESP32Backend::LogMessageSend(MessageSendInfo & info) {}

void ESP32Backend::LogNodeLookup(NodeLookupInfo & info) {}

void ESP32Backend::LogNodeDiscovered(NodeDiscoveredInfo & info) {}

void ESP32Backend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info) {}

void ESP32Backend::TraceBegin(const char * label, const char * group)
{
    LOG_HEAP_INFO(label, group, "Entry");
}

void ESP32Backend::TraceEnd(const char * label, const char * group)
{
    LOG_HEAP_INFO(label, group, "Exit");
}

void ESP32Backend::TraceInstant(const char * label, const char * group)
{
    ESP_DIAG_EVENT("MTR_TRC", "Instant : %s -%s", label, group);
}
} // namespace Insights
} // namespace Tracing
} // namespace chip
// namespace chip
