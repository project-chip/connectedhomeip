/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 */

#pragma once
#include <esp_diagnostics_metrics.h>

esp_err_t matter_esp_insights_add_uint([[maybe_unused]] const char * tag, const char * key, uint32_t u)
{
#ifdef CONFIG_ESP_INSIGHTS_META_VERSION_10
    return esp_diag_metrics_add_uint(key, u);
#else
    return esp_diag_metrics_report_uint(tag, key, u);
#endif
}

esp_err_t matter_esp_insights_add_int([[maybe_unused]] const char * tag, const char * key, int32_t i)
{
#ifdef CONFIG_ESP_INSIGHTS_META_VERSION_10
    return esp_diag_metrics_add_int(key, i);
#else
    return esp_diag_metrics_report_int(tag, key, i);
#endif
}
