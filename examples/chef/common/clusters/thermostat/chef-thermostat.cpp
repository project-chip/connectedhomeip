/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "chef-thermostat.h"

#if MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0

#include "DeviceTypes.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <devices/Types.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Chef {

FabricTable & ChefThermostatDelegate::GetFabricTable() const
{
    return Server::GetInstance().GetFabricTable();
}

DataModel::Nullable<temperature> ChefThermostatDelegate::GetLocalTemperature() const
{
    return mLocalTemperature;
}

Status ChefThermostatDelegate::SetLocalTemperature(DataModel::Nullable<temperature> localTemperature, bool & changed)
{
    changed = (mLocalTemperature != localTemperature);
    if (changed)
    {
        mLocalTemperature = localTemperature;
#if MATTER_DM_WATER_HEATER_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        if (chef::DeviceTypes::EndpointHasDeviceType(mEndpointId, Device::kWaterHeaterDeviceTypeId))
        {
            MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id,
                                                   WaterHeaterManagement::Attributes::EstimatedHeatRequired::Id);
        }
#endif
    }
    return Status::Success;
}

Status ChefThermostatDelegate::GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemperature) const
{
    outdoorTemperature = mOutdoorTemperature;
    return Status::Success;
}

void ChefThermostatDelegate::SetOutdoorTemperatureValue(DataModel::Nullable<temperature> outdoorTemperature, bool & changed)
{
    changed = (mOutdoorTemperature != outdoorTemperature);
    if (changed)
    {
        mOutdoorTemperature = outdoorTemperature;
    }
}

int8_t ChefThermostatDelegate::GetLocalTemperatureCalibration() const
{
    return mLocalTemperatureCalibration;
}

Status ChefThermostatDelegate::SetLocalTemperatureCalibration(int8_t localTemperatureCalibration, bool & changed)
{
    changed = (mLocalTemperatureCalibration != localTemperatureCalibration);
    if (changed)
    {
        mLocalTemperatureCalibration = localTemperatureCalibration;
    }
    return Status::Success;
}

Status ChefThermostatDelegate::GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const
{
    remoteSensing = mRemoteSensing;
    return Status::Success;
}

Status ChefThermostatDelegate::SetRemoteSensing(BitMask<RemoteSensingBitmap> remoteSensing, bool & changed)
{
    changed = (mRemoteSensing != remoteSensing);
    if (changed)
    {
        mRemoteSensing = remoteSensing;
    }
    return Status::Success;
}

ControlSequenceOfOperationEnum ChefThermostatDelegate::GetControlSequenceOfOperation() const
{
    return mControlSequenceOfOperation;
}

Status ChefThermostatDelegate::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum controlSequenceOfOperation,
                                                             bool & changed)
{
    changed = (mControlSequenceOfOperation != controlSequenceOfOperation);
    if (changed)
    {
        mControlSequenceOfOperation = controlSequenceOfOperation;
    }
    return Status::Success;
}

SystemModeEnum ChefThermostatDelegate::GetSystemMode() const
{
    return mSystemMode;
}

Status ChefThermostatDelegate::SetSystemMode(SystemModeEnum systemMode, bool & changed)
{
    changed = (mSystemMode != systemMode);
    if (changed)
    {
        mSystemMode = systemMode;
    }
    return Status::Success;
}

Status ChefThermostatDelegate::GetRunningMode(ThermostatRunningModeEnum & runningMode) const
{
    runningMode = mRunningMode;
    return Status::Success;
}

Status ChefThermostatDelegate::SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed)
{
    changed = (mRunningMode != runningMode);
    if (changed)
    {
        mRunningMode = runningMode;
    }
    return Status::Success;
}

Status ChefThermostatDelegate::GetRunningState(BitMask<RelayStateBitmap> & runningState) const
{
    runningState = mRunningState;
    return Status::Success;
}

Status ChefThermostatDelegate::SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed)
{
    changed = (mRunningState != runningState);
    if (changed)
    {
        mRunningState = runningState;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const
{
    occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint, bool & changed)
{
    changed = (mOccupiedHeatingSetpoint != occupiedHeatingSetpoint);
    if (changed)
    {
        mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
#if MATTER_DM_WATER_HEATER_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        if (chef::DeviceTypes::EndpointHasDeviceType(mEndpointId, Device::kWaterHeaterDeviceTypeId))
        {
            MatterReportingAttributeChangeCallback(mEndpointId, WaterHeaterManagement::Id,
                                                   WaterHeaterManagement::Attributes::EstimatedHeatRequired::Id);
        }
#endif
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetUnoccupiedHeatingSetpoint(temperature & unoccupiedHeatingSetpoint) const
{
    unoccupiedHeatingSetpoint = mUnoccupiedHeatingSetpoint;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetUnoccupiedHeatingSetpoint(temperature unoccupiedHeatingSetpoint, bool & changed)
{
    changed = (mUnoccupiedHeatingSetpoint != unoccupiedHeatingSetpoint);
    if (changed)
    {
        mUnoccupiedHeatingSetpoint = unoccupiedHeatingSetpoint;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetAbsMinHeatSetpointLimit(temperature & absMinHeatSetpointLimit) const
{
    absMinHeatSetpointLimit = mAbsMinHeatSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetAbsMaxHeatSetpointLimit(temperature & absMaxHeatSetpointLimit) const
{
    absMaxHeatSetpointLimit = mAbsMaxHeatSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetMinHeatSetpointLimit(temperature & minHeatSetpointLimit) const
{
    minHeatSetpointLimit = mMinHeatSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetMinHeatSetpointLimit(temperature minHeatSetpointLimit, bool & changed)
{
    changed = (mMinHeatSetpointLimit != minHeatSetpointLimit);
    if (changed)
    {
        mMinHeatSetpointLimit = minHeatSetpointLimit;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetMaxHeatSetpointLimit(temperature & maxHeatSetpointLimit) const
{
    maxHeatSetpointLimit = mMaxHeatSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetMaxHeatSetpointLimit(temperature maxHeatSetpointLimit, bool & changed)
{
    changed = (mMaxHeatSetpointLimit != maxHeatSetpointLimit);
    if (changed)
    {
        mMaxHeatSetpointLimit = maxHeatSetpointLimit;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetOccupiedCoolingSetpoint(temperature & occupiedCoolingSetpoint) const
{
    occupiedCoolingSetpoint = mOccupiedCoolingSetpoint;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetOccupiedCoolingSetpoint(temperature occupiedCoolingSetpoint, bool & changed)
{
    changed = (mOccupiedCoolingSetpoint != occupiedCoolingSetpoint);
    if (changed)
    {
        mOccupiedCoolingSetpoint = occupiedCoolingSetpoint;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetUnoccupiedCoolingSetpoint(temperature & unoccupiedCoolingSetpoint) const
{
    unoccupiedCoolingSetpoint = mUnoccupiedCoolingSetpoint;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetUnoccupiedCoolingSetpoint(temperature unoccupiedCoolingSetpoint, bool & changed)
{
    changed = (mUnoccupiedCoolingSetpoint != unoccupiedCoolingSetpoint);
    if (changed)
    {
        mUnoccupiedCoolingSetpoint = unoccupiedCoolingSetpoint;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetAbsMinCoolSetpointLimit(temperature & absMinCoolSetpointLimit) const
{
    absMinCoolSetpointLimit = mAbsMinCoolSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetAbsMaxCoolSetpointLimit(temperature & absMaxCoolSetpointLimit) const
{
    absMaxCoolSetpointLimit = mAbsMaxCoolSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetMinCoolSetpointLimit(temperature & minCoolSetpointLimit) const
{
    minCoolSetpointLimit = mMinCoolSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetMinCoolSetpointLimit(temperature minCoolSetpointLimit, bool & changed)
{
    changed = (mMinCoolSetpointLimit != minCoolSetpointLimit);
    if (changed)
    {
        mMinCoolSetpointLimit = minCoolSetpointLimit;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetMaxCoolSetpointLimit(temperature & maxCoolSetpointLimit) const
{
    maxCoolSetpointLimit = mMaxCoolSetpointLimit;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetMaxCoolSetpointLimit(temperature maxCoolSetpointLimit, bool & changed)
{
    changed = (mMaxCoolSetpointLimit != maxCoolSetpointLimit);
    if (changed)
    {
        mMaxCoolSetpointLimit = maxCoolSetpointLimit;
    }
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::GetMinDeadband(temperature & minDeadband) const
{
    minDeadband = mMinDeadband;
    return Status::Success;
}

Status ChefThermostatSetpointsDelegate::SetMinDeadband(temperature minDeadband, bool & changed)
{
    changed = (mMinDeadband != minDeadband);
    if (changed)
    {
        mMinDeadband = minDeadband;
    }
    return Status::Success;
}

TemperatureSetpointHoldEnum ChefThermostatHoldDelegate::GetTemperatureSetpointHold() const
{
    return mTemperatureSetpointHold;
}

Status ChefThermostatHoldDelegate::SetTemperatureSetpointHold(TemperatureSetpointHoldEnum temperatureSetpointHold, bool & changed)
{
    changed = (mTemperatureSetpointHold != temperatureSetpointHold);
    if (changed)
    {
        mTemperatureSetpointHold = temperatureSetpointHold;
    }
    return Status::Success;
}

DataModel::Nullable<uint16_t> ChefThermostatHoldDelegate::GetTemperatureSetpointHoldDuration() const
{
    return mTemperatureSetpointHoldDuration;
}

Status ChefThermostatHoldDelegate::SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration, bool & changed)
{
    changed = (mTemperatureSetpointHoldDuration != duration);
    if (changed)
    {
        mTemperatureSetpointHoldDuration = duration;
    }
    return Status::Success;
}

DataModel::Nullable<uint32_t> ChefThermostatHoldDelegate::GetSetpointHoldExpiryTimestamp() const
{
    return mSetpointHoldExpiryTimestamp;
}

Status ChefThermostatHoldDelegate::SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp, bool & changed)
{
    changed = (mSetpointHoldExpiryTimestamp != timestamp);
    if (changed)
    {
        mSetpointHoldExpiryTimestamp = timestamp;
    }
    return Status::Success;
}

BitMask<OccupancyBitmap> ChefThermostatOccupancyDelegate::GetOccupancy() const
{
    return mOccupancy;
}

Status ChefThermostatOccupancyDelegate::SetOccupancy(BitMask<OccupancyBitmap> occupied, bool & changed)
{
    changed = (mOccupancy != occupied);
    if (changed)
    {
        mOccupancy = occupied;
    }
    return Status::Success;
}

} // namespace Chef
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
