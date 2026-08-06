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
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace Protocols::InteractionModel;
using namespace System::Clock;

ThermostatDelegate::ThermostatDelegate(EndpointId endpoint, AttributePersistenceProvider * provider)
{
    mEndpointId = endpoint;
    mProvider   = provider;

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

Protocols::InteractionModel::Status ThermostatDelegate::LoadSetpoints(Setpoints & setpoints)
{
    ChipLogDetail(Zcl, "Loading setpoints");
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::Failure);
    AttributePersistence persistence(*provider);

    if (setpoints.autoSupported)
    {
        int8_t deadBand;
        persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinSetpointDeadBand::Id }, deadBand,
                                          static_cast<int8_t>(kDefaultDeadBand / 10));
        setpoints.deadBand = static_cast<int16_t>(deadBand * 10);
    }

    int16_t absMinHeatLimit = kDefaultAbsMinHeatSetpointLimit;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, AbsMinHeatSetpointLimit::Id }, absMinHeatLimit,
                                      static_cast<int16_t>(kDefaultAbsMinHeatSetpointLimit));
    setpoints.absoluteHeatLimits.minimum.SetTemperature(absMinHeatLimit);

    int16_t absMaxHeatLimit = kDefaultAbsMaxHeatSetpointLimit;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, AbsMaxHeatSetpointLimit::Id }, absMaxHeatLimit,
                                      static_cast<int16_t>(kDefaultAbsMaxHeatSetpointLimit));
    setpoints.absoluteHeatLimits.maximum.SetTemperature(absMaxHeatLimit);

    int16_t absMinCoolLimit = kDefaultAbsMinCoolSetpointLimit;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, AbsMinCoolSetpointLimit::Id }, absMinCoolLimit,
                                      static_cast<int16_t>(kDefaultAbsMinCoolSetpointLimit));
    setpoints.absoluteCoolLimits.minimum.SetTemperature(absMinCoolLimit);

    int16_t absMaxCoolLimit = kDefaultAbsMaxCoolSetpointLimit;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, AbsMaxCoolSetpointLimit::Id }, absMaxCoolLimit,
                                      static_cast<int16_t>(kDefaultAbsMaxCoolSetpointLimit));
    setpoints.absoluteCoolLimits.maximum.SetTemperature(absMaxCoolLimit);

    if (setpoints.heatSupported)
    {
        int16_t minHeatLimit;
        if (persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinHeatSetpointLimit::Id }, minHeatLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userHeatLimits.minimum.SetTemperature(minHeatLimit);
        }
        int16_t maxHeatLimit;
        if (persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MaxHeatSetpointLimit::Id }, maxHeatLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userHeatLimits.maximum.SetTemperature(maxHeatLimit);
        }
    }
    if (setpoints.coolSupported)
    {
        int16_t minCoolLimit;
        if (persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinCoolSetpointLimit::Id }, minCoolLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userCoolLimits.minimum.SetTemperature(minCoolLimit);
        }
        int16_t maxCoolLimit;
        if (persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MaxCoolSetpointLimit::Id }, maxCoolLimit,
                                              static_cast<int16_t>(0)))
        {
            setpoints.userCoolLimits.maximum.SetTemperature(maxCoolLimit);
        }
    }

    int16_t occupiedCooling;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, OccupiedCoolingSetpoint::Id }, occupiedCooling,
                                      static_cast<int16_t>(kDefaultCoolingSetpoint));
    setpoints.occupiedRange.cooling.SetTemperature(occupiedCooling);

    int16_t occupiedHeating;
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, OccupiedHeatingSetpoint::Id }, occupiedHeating,
                                      static_cast<int16_t>(kDefaultHeatingSetpoint));
    setpoints.occupiedRange.heating.SetTemperature(occupiedHeating);

    if (setpoints.occupancySupported)
    {
        int16_t unoccupiedCooling;
        persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, UnoccupiedCoolingSetpoint::Id }, unoccupiedCooling,
                                          static_cast<int16_t>(kDefaultCoolingSetpoint));
        setpoints.unoccupiedRange.cooling.SetTemperature(unoccupiedCooling);

        int16_t unoccupiedHeating;
        persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, UnoccupiedHeatingSetpoint::Id }, unoccupiedHeating,
                                          static_cast<int16_t>(kDefaultHeatingSetpoint));
        setpoints.unoccupiedRange.heating.SetTemperature(unoccupiedHeating);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::SaveSetpoint(const Setpoint & oldSetpoint, const Setpoint & newSetpoint)
{
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::Failure);
    AttributePersistence persistence(*provider);
    temperature newTemp = newSetpoint.Temperature();

    auto status = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, oldSetpoint.AttributeId() }, newTemp);
    if (status != CHIP_NO_ERROR)
    {
        return chip::Protocols::InteractionModel::ClusterStatusCode(status).GetStatus();
    }

    return Status::Success;
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
