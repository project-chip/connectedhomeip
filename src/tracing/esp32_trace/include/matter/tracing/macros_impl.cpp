/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "macros_impl.h"
#include <esp_heap_caps.h>
#include <esp_insights.h>
namespace Insights {

#define LOG_HEAP_INFO(label, group, entry_exit)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        ESP_DIAG_EVENT("MTR_TRC", "%s - %s - %s Min Free heap - %u - LFB - %u Start free heap - %u", entry_exit, label, group,     \
                       heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),                                                           \
                       heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT),                                    \
                       heap_caps_get_free_size(MALLOC_CAP_8BIT));                                                                  \
    } while (0)

ESP32Backend::ESP32Backend(const char * str, ...)
{
    va_list args;
    va_start(args, str);
    mlabel = str;
    mgroup = va_arg(args, const char *);
    LOG_HEAP_INFO(mlabel, mgroup, "Entry");
}

ESP32Backend::~ESP32Backend()
{
    LOG_HEAP_INFO(mlabel, mgroup, "Exit");
}
} // namespace Insights
