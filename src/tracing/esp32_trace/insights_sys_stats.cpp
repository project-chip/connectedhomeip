/*
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

#include "insights_sys_stats.h"
#include <esp_diagnostics_metrics.h>
#include <esp_err.h>
#include <esp_log.h>
#include <platform/CHIPDeviceLayer.h>
#include <string.h>
#include <system/SystemStats.h>

namespace chip {
namespace System {
namespace Stats {

InsightsSystemMetrics & InsightsSystemMetrics::GetInstance()
{
    static InsightsSystemMetrics instance;
    return instance;
}

void InsightsSystemMetrics::SamplingHandler(Layer * systemLayer, void * context)
{
    auto instance            = static_cast<InsightsSystemMetrics *>(context);
    count_t * highwatermarks = GetHighWatermarks();
    for (int i = 0; i < System::Stats::kNumEntries; i++)
    {
        esp_err_t err = esp_diag_metrics_add_uint(instance->mLabels[i], static_cast<uint32_t>(highwatermarks[i]));
        if (err != ESP_OK)
        {
            ESP_LOGE(kTag, "Failed to add the metric:%s, err:%d", instance->mLabels[i], err);
        }
    }
    DeviceLayer::SystemLayer().StartTimer(instance->GetSamplingInterval(), SamplingHandler, instance);
}

CHIP_ERROR InsightsSystemMetrics::Unregister(intptr_t arg)
{
    InsightsSystemMetrics * instance = reinterpret_cast<InsightsSystemMetrics *>(arg);
    if (!mRegistered)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    for (int i = 0; i < System::Stats::kNumEntries; i++)
    {
        if (mLabels[i] != nullptr)
        {
            esp_err_t err = esp_diag_metrics_unregister(mLabels[i]);
            if (err != ESP_OK)
            {
                ESP_LOGE(kTag, "Failed to unregister metric:%s, err:%d", mLabels[i], err);
            }
            free(mLabels[i]);
            mLabels[i] = nullptr;
        }
    }
    mRegistered = false;
    DeviceLayer::SystemLayer().CancelTimer(SamplingHandler, instance);
    return CHIP_NO_ERROR;
}

void InsightsSystemMetrics::SetSamplingHandler(intptr_t arg)
{
    InsightsSystemMetrics * instance = reinterpret_cast<InsightsSystemMetrics *>(arg);

    if (instance->mTimeout == System::Clock::kZero)
    {
        DeviceLayer::SystemLayer().CancelTimer(SamplingHandler, instance);
    }
    else
    {
        DeviceLayer::SystemLayer().CancelTimer(SamplingHandler, instance);
        CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(instance->mTimeout, SamplingHandler, instance);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(kTag, "Failed to start the new timer %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

CHIP_ERROR InsightsSystemMetrics::SetSamplingInterval(chip::System::Clock::Timeout aTimeout)
{
    if (!mRegistered)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mTimeout = aTimeout;

    return DeviceLayer::PlatformMgr().ScheduleWork(SetSamplingHandler, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR InsightsSystemMetrics::RegisterAndEnable(chip::System::Clock::Timeout aTimeout)
{
    if (mRegistered)
    {
        return CHIP_NO_ERROR;
    }

    if (aTimeout == System::Clock::kZero)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const Label * labels = GetStrings();
    for (int i = 0; i < System::Stats::kNumEntries; i++)
    {
        size_t labelLength = strlen(labels[i]);
        if (labelLength >= kMaxStringLength)
        {
            labelLength = kMaxStringLength;
            mLabels[i]  = strndup(labels[i], labelLength - 1);
        }
        else
        {
            mLabels[i] = strndup(labels[i], labelLength);
        }

        if (mLabels[i] == NULL)
        {
            Unregister(reinterpret_cast<intptr_t>(this));
            return CHIP_ERROR_NO_MEMORY;
        }

        esp_err_t err = esp_diag_metrics_register(kTag, mLabels[i], labels[i], kPath, ESP_DIAG_DATA_TYPE_UINT);
        if (err != ESP_OK)
        {
            ESP_LOGE(kTag, "Failed to register metric:%s, err:%d", mLabels[i], err);
            Unregister(reinterpret_cast<intptr_t>(this));
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    mTimeout = System::Clock::Milliseconds32(aTimeout);

    CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(GetSamplingInterval(), SamplingHandler, this);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(kTag, "Failed to start the timer, err:%" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    mRegistered = true;
    return CHIP_NO_ERROR;
}

} // namespace Stats
} // namespace System
} // namespace chip
