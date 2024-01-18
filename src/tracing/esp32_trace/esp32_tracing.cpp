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
#include <algorithm>
#include <esp_heap_caps.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <memory>
#include <tracing/backend.h>

namespace chip {
namespace Tracing {
namespace Insights {
namespace {

constexpr size_t kPermitListMaxSize = 10;
using HashValue                     = uint32_t;

// Implements a murmurhash with 0 seed.
uint32_t MurmurHash(const void * key)
{
    const uint32_t kMultiplier = 0x5bd1e995;
    const uint32_t kShift      = 24;
    const unsigned char * data = (const unsigned char *) key;
    uint32_t hash              = 0;

    while (*data)
    {
        uint32_t value = *data++;
        value *= kMultiplier;
        value ^= value >> kShift;
        value *= kMultiplier;
        hash *= kMultiplier;
        hash ^= value;
    }

    hash ^= hash >> 13;
    hash *= kMultiplier;
    hash ^= hash >> 15;

    if (hash == 0)
    {
        ESP_LOGW("Tracing", "MurmurHash resulted in a hash value of 0");
    }

    return hash;
}

/* PASESession,CASESession,NetworkCommissioning,GeneralCommissioning,OperationalCredentials
 * are well known permitted entries.
 */

HashValue gPermitList[kPermitListMaxSize] = {
    MurmurHash("PASESession"),
    MurmurHash("CASESession"),
    MurmurHash("NetworkCommissioning"),
    MurmurHash("GeneralCommissioning"),
    MurmurHash("OperationalCredentials"),
};

bool IsPermitted(HashValue hashValue)
{
    for (HashValue permitted : gPermitList)
    {
        if (permitted == 0)
        {
            break;
        }
        if (hashValue == permitted)
        {
            return true;
        }
    }
    return false;
}

} // namespace

namespace ESP32Filter {

CHIP_ERROR AddHashToPermitlist(const char * str)
{
    HashValue hashValue = MurmurHash(str);
    if (hashValue == 0)
    {
        ESP_LOGW("TRC", "Hash value for '%s' is 0", str);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    for (HashValue & permitted : gPermitList)
    {
        if (permitted == 0)
        {
            permitted = hashValue;
            return CHIP_NO_ERROR;
        }
        if (hashValue == permitted)
        {
            ESP_LOGW("TRC", "Hash value for '%s' is colliding with an existing entry", str);
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
    }
    return CHIP_ERROR_NO_MEMORY;
}

void RemoveHashFromPermitlist(const char * str)
{
    HashValue hashValue = MurmurHash(str);

    auto * end = gPermitList + kPermitListMaxSize;
    std::fill(std::remove(gPermitList, end, hashValue), end, 0);
}

} // namespace ESP32Filter

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
    HashValue hashValue = MurmurHash(group);
    if (IsPermitted(hashValue))
    {
        LOG_HEAP_INFO(label, group, "Entry");
    }
}

void ESP32Backend::TraceEnd(const char * label, const char * group)
{
    HashValue hashValue = MurmurHash(group);
    if (IsPermitted(hashValue))
    {
        LOG_HEAP_INFO(label, group, "Exit");
    }
}

void ESP32Backend::TraceInstant(const char * label, const char * group)
{
    ESP_DIAG_EVENT("MTR_TRC", "Instant : %s -%s", label, group);
}
} // namespace Insights
} // namespace Tracing
} // namespace chip
