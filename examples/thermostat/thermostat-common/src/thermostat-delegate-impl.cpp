/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "../include/thermostat-delegate-impl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app/clusters/thermostat-server/ThermostatClusterWithFeatures.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace System::Clock;

ThermostatDelegate ThermostatDelegate::sInstance;

ThermostatDelegate::ThermostatDelegate()
{
    // Initialize Presets state
    mNumberOfPresets                            = kMaxNumberOfPresetsSupported;
    mNextFreeIndexInPresetsList                 = 0;
    mNextFreeIndexInPendingPresetsList          = 0;
    mMaxNumberOfSchedulesAllowedPerScheduleType = kMaxNumberOfSchedulesSupported;

    InitializePresets();
    InitializeScheduleTypes();

    memset(mActivePresetHandleData, 0, sizeof(mActivePresetHandleData));
    mActivePresetHandleDataSize = 0;

    // Initialize Suggestions state
    mMaxThermostatSuggestions                 = kMaxNumberOfThermostatSuggestions;
    mIndexOfCurrentSuggestion                 = mMaxThermostatSuggestions;
    mNextFreeIndexInThermostatSuggestionsList = 0;
    mUniqueID                                 = 0;
}

ThermostatDelegate::~ThermostatDelegate()
{
    CancelExpirationTimer();
}

void ThermostatDelegate::InitializeScheduleTypes()
{
    static_assert(MATTER_ARRAY_SIZE(mScheduleTypes) == 2);

    mScheduleTypes[0] = { .systemMode           = SystemModeEnum::kHeat,
                          .numberOfSchedules    = mMaxNumberOfSchedulesAllowedPerScheduleType,
                          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) };

    mScheduleTypes[1] = { .systemMode           = SystemModeEnum::kCool,
                          .numberOfSchedules    = mMaxNumberOfSchedulesAllowedPerScheduleType,
                          .scheduleTypeFeatures = to_underlying(ScheduleTypeFeaturesBitmap::kSupportsSetpoints) };
}

CHIP_ERROR ThermostatDelegate::GetScheduleTypeAtIndex(size_t index, Structs::ScheduleTypeStruct::Type & scheduleType)
{
    if (index < MATTER_ARRAY_SIZE(mScheduleTypes))
    {
        scheduleType = mScheduleTypes[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}
