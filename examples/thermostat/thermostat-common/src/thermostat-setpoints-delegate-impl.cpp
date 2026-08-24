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

#include "../include/thermostat-setpoints-delegate-impl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/persistence/AttributePersistence.h>
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app/clusters/thermostat-server/Temperature.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace Protocols::InteractionModel;
using namespace System::Clock;

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::Init()
{
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::Failure);
    AttributePersistence persistence(*provider);

    if (auto status = AbsMinCoolSetpointLimit::GetDefault(mEndpointId, &mAbsMinCoolSetpointLimit); status != Status::Success)
    {
        mAbsMinCoolSetpointLimit = kDefaultAbsMinCoolSetpointLimit;
    }

    if (auto status = AbsMaxCoolSetpointLimit::GetDefault(mEndpointId, &mAbsMaxCoolSetpointLimit); status != Status::Success)
    {
        mAbsMaxCoolSetpointLimit = kDefaultAbsMaxCoolSetpointLimit;
    }

    temperature defaultMinCoolLimit;
    if (auto status = MinCoolSetpointLimit::GetDefault(mEndpointId, &defaultMinCoolLimit); status != Status::Success)
    {
        defaultMinCoolLimit = mAbsMinCoolSetpointLimit;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinCoolSetpointLimit::Id }, mMinCoolSetpointLimit,
                                      defaultMinCoolLimit);

    temperature defaultMaxCoolLimit;
    if (auto status = MaxCoolSetpointLimit::GetDefault(mEndpointId, &defaultMaxCoolLimit); status != Status::Success)
    {
        defaultMaxCoolLimit = mAbsMaxCoolSetpointLimit;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MaxCoolSetpointLimit::Id }, mMaxCoolSetpointLimit,
                                      defaultMaxCoolLimit);

    if (auto status = OccupiedCoolingSetpoint::GetDefault(mEndpointId, &mOccupiedCoolingSetpoint); status != Status::Success)
    {
        mOccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, OccupiedCoolingSetpoint::Id }, mOccupiedCoolingSetpoint,
                                      mOccupiedCoolingSetpoint);

    if (auto status = UnoccupiedCoolingSetpoint::GetDefault(mEndpointId, &mUnoccupiedCoolingSetpoint); status != Status::Success)
    {
        mUnoccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, UnoccupiedCoolingSetpoint::Id }, mUnoccupiedCoolingSetpoint,
                                      mUnoccupiedCoolingSetpoint);

    if (auto status = AbsMinHeatSetpointLimit::GetDefault(mEndpointId, &mAbsMinHeatSetpointLimit); status != Status::Success)
    {
        mAbsMinHeatSetpointLimit = kDefaultAbsMinHeatSetpointLimit;
    }

    if (auto status = AbsMaxHeatSetpointLimit::GetDefault(mEndpointId, &mAbsMaxHeatSetpointLimit); status != Status::Success)
    {
        mAbsMaxHeatSetpointLimit = kDefaultAbsMaxHeatSetpointLimit;
    }

    temperature defaultMinHeatLimit;
    if (auto status = MinHeatSetpointLimit::GetDefault(mEndpointId, &defaultMinHeatLimit); status != Status::Success)
    {
        defaultMinHeatLimit = mAbsMinHeatSetpointLimit;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinHeatSetpointLimit::Id }, mMinHeatSetpointLimit,
                                      defaultMinHeatLimit);

    temperature defaultMaxHeatLimit;
    if (auto status = MaxHeatSetpointLimit::GetDefault(mEndpointId, &defaultMaxHeatLimit); status != Status::Success)
    {
        defaultMaxHeatLimit = mAbsMaxHeatSetpointLimit;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MaxHeatSetpointLimit::Id }, mMaxHeatSetpointLimit,
                                      defaultMaxHeatLimit);

    if (auto status = OccupiedHeatingSetpoint::GetDefault(mEndpointId, &mOccupiedHeatingSetpoint); status != Status::Success)
    {
        mOccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, OccupiedHeatingSetpoint::Id }, mOccupiedHeatingSetpoint,
                                      mOccupiedHeatingSetpoint);

    if (auto status = UnoccupiedHeatingSetpoint::GetDefault(mEndpointId, &mUnoccupiedHeatingSetpoint); status != Status::Success)
    {
        mUnoccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    }
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, UnoccupiedHeatingSetpoint::Id }, mUnoccupiedHeatingSetpoint,
                                      mUnoccupiedHeatingSetpoint);

    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::GetMinDeadband(temperature & minDeadband) const
{
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::Failure);
    AttributePersistence persistence(*provider);

    int8_t deadBand = static_cast<int8_t>(kDefaultDeadBand / 10);
    MinSetpointDeadBand::GetDefault(mEndpointId, &deadBand);
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, MinSetpointDeadBand::Id }, deadBand, deadBand);
    minDeadband = static_cast<int16_t>(deadBand * 10);
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SaveSetpoint(AttributeId attributeId, temperature value)
{
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::Failure);
    AttributePersistence persistence(*provider);
    if (auto status = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, attributeId }, value);
        status != CHIP_NO_ERROR)
    {
        return ClusterStatusCode(status).GetStatus();
    }
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const
{
    absMinCoolSetpointLimit = mAbsMinCoolSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const
{
    absMaxCoolSetpointLimit = mAbsMaxCoolSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const
{
    minCoolSetpointLimit = mMinCoolSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetMinCoolSetpointLimit(temperature minCoolSetpointLimit,
                                                                                         bool & changed)
{
    if (mMinCoolSetpointLimit == minCoolSetpointLimit)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(MinCoolSetpointLimit::Id, minCoolSetpointLimit); status != Status::Success)
    {
        return status;
    }
    mMinCoolSetpointLimit = minCoolSetpointLimit;
    changed               = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const
{
    maxCoolSetpointLimit = mMaxCoolSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit,
                                                                                         bool & changed)
{
    if (mMaxCoolSetpointLimit == maxCoolSetpointLimit)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(MaxCoolSetpointLimit::Id, maxCoolSetpointLimit); status != Status::Success)
    {
        return status;
    }
    mMaxCoolSetpointLimit = maxCoolSetpointLimit;
    changed               = true;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const
{
    occupiedCoolingSetpoint = mOccupiedCoolingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint,
                                                                                            bool & changed)
{
    if (mOccupiedCoolingSetpoint == occupiedCoolingSetpoint)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(OccupiedCoolingSetpoint::Id, occupiedCoolingSetpoint); status != Status::Success)
    {
        return status;
    }
    mOccupiedCoolingSetpoint = occupiedCoolingSetpoint;
    changed                  = true;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const
{
    unoccupiedCoolingSetpoint = mUnoccupiedCoolingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint,
                                                                                              bool & changed)
{
    if (mUnoccupiedCoolingSetpoint == unoccupiedCoolingSetpoint)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(UnoccupiedCoolingSetpoint::Id, unoccupiedCoolingSetpoint); status != Status::Success)
    {
        return status;
    }
    mUnoccupiedCoolingSetpoint = unoccupiedCoolingSetpoint;
    changed                    = true;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const
{
    absMinHeatSetpointLimit = mAbsMinHeatSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const
{
    absMaxHeatSetpointLimit = mAbsMaxHeatSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const
{
    minHeatSetpointLimit = mMinHeatSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetMinHeatSetpointLimit(temperature minHeatSetpointLimit,
                                                                                         bool & changed)
{
    if (mMinHeatSetpointLimit == minHeatSetpointLimit)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(MinHeatSetpointLimit::Id, minHeatSetpointLimit); status != Status::Success)
    {
        return status;
    }
    mMinHeatSetpointLimit = minHeatSetpointLimit;
    changed               = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const
{
    maxHeatSetpointLimit = mMaxHeatSetpointLimit;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit,
                                                                                         bool & changed)
{
    if (mMaxHeatSetpointLimit == maxHeatSetpointLimit)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(MaxHeatSetpointLimit::Id, maxHeatSetpointLimit); status != Status::Success)
    {
        return status;
    }
    mMaxHeatSetpointLimit = maxHeatSetpointLimit;
    changed               = true;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const
{
    occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint,
                                                                                            bool & changed)
{
    if (mOccupiedHeatingSetpoint == occupiedHeatingSetpoint)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(OccupiedHeatingSetpoint::Id, occupiedHeatingSetpoint); status != Status::Success)
    {
        return status;
    }
    mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
    changed                  = true;
    return Status::Success;
}

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const
{
    unoccupiedHeatingSetpoint = mUnoccupiedHeatingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint,
                                                                                              bool & changed)
{
    if (mUnoccupiedHeatingSetpoint == unoccupiedHeatingSetpoint)
    {
        return Status::Success;
    }
    if (auto status = SaveSetpoint(UnoccupiedHeatingSetpoint::Id, unoccupiedHeatingSetpoint); status != Status::Success)
    {
        return status;
    }
    mUnoccupiedHeatingSetpoint = unoccupiedHeatingSetpoint;
    changed                    = true;
    return Status::Success;
}
