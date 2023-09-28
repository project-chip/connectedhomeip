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

#include <esp_log.h>
#include <tracing/backend.h>
#include "esp32_tracing.h"
#include <memory>
#include <esp_insights.h>
namespace chip {
namespace Tracing {
namespace Insights {

void ESP32Backend::LogMessageReceived(MessageReceivedInfo & info)
{
    ESP_LOGI("Matter", "Message Received");
}

void ESP32Backend::LogMessageSend(MessageSendInfo & info)
{
    ESP_LOGI("Data_Log", "Message Sent");
}

void ESP32Backend::LogNodeLookup(NodeLookupInfo & info)
{
   ESP_LOGI("Data_Log", "Node Lookup");
}

void ESP32Backend::LogNodeDiscovered(NodeDiscoveredInfo & info)
{
   ESP_LOGI("Data_Log", "Log Node Discovered");
}

void ESP32Backend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info)
{
   ESP_LOGI("Data_Log", "Log Node Discovery Failed");
}

void ESP32Backend::TraceBegin(const char * label, const char * group)
{
    ESP_DIAG_EVENT("Trc", "Entry : %s -%s", label, group);
}

void ESP32Backend::TraceEnd(const char * label, const char * group)
{
    ESP_DIAG_EVENT("Trc", "Exit : %s -%s", label, group);
}

void ESP32Backend::TraceInstant(const char * label, const char * group)
{
    ESP_DIAG_EVENT("Trc", "Instant : %s -%s", label, group);
}
}
} // namespace Insights
} // namespace Tracing
// namespace chip
