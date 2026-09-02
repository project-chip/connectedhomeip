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
#include "app/data-model/Nullable.h"
#include "app/server-cluster/ServerClusterContext.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
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

FabricTable & ThermostatDelegate::GetFabricTable() const
{
    return mFabricTable != nullptr ? *mFabricTable : Server::GetInstance().GetFabricTable();
}

CHIP_ERROR ThermostatDelegate::Startup(ServerClusterContext & context)
{
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    AttributePersistence persistence(*provider);

    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, SystemMode::Id }, mSystemMode, mSystemMode);
    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, ControlSequenceOfOperation::Id }, mControlSequenceOfOperation,
                                      mControlSequenceOfOperation);
    uint8_t remoteSensing = 0;
    if (persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, RemoteSensing::Id }, remoteSensing, remoteSensing))
    {
        mRemoteSensing = BitMask<RemoteSensingBitmap>(remoteSensing);
    }

    persistence.LoadNativeEndianValue({ mEndpointId, Thermostat::Id, LocalTemperatureCalibration::Id },
                                      mLocalTemperatureCalibration, mLocalTemperatureCalibration);

    return CHIP_NO_ERROR;
}

SystemModeEnum ThermostatDelegate::GetSystemMode() const
{
    return mSystemMode;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetSystemMode(SystemModeEnum systemMode, bool & changed)
{
    if (mSystemMode == systemMode)
    {
        return Status::Success;
    }
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::InvalidInState);
    AttributePersistence persistence(*provider);
    CHIP_ERROR result = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, SystemMode::Id }, systemMode);
    if (result != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to store SystemMode attribute");
        return Status::Failure;
    }
    mSystemMode = systemMode;
    changed     = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::GetRunningMode(ThermostatRunningModeEnum & runningMode) const
{
    runningMode = mRunningMode;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed)
{
    if (mRunningMode == runningMode)
    {
        return Status::Success;
    }
    mRunningMode = runningMode;
    changed      = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::GetRunningState(BitMask<RelayStateBitmap> & runningState) const
{
    runningState = mRunningState;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed)
{
    if (mRunningState == runningState)
    {
        return Status::Success;
    }
    mRunningState = runningState;
    changed       = true;
    return Status::Success;
}

ControlSequenceOfOperationEnum ThermostatDelegate::GetControlSequenceOfOperation() const
{
    return mControlSequenceOfOperation;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq,
                                                                                      bool & changed)
{
    if (mControlSequenceOfOperation == seq)
    {
        return Status::Success;
    }
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::InvalidInState);
    AttributePersistence persistence(*provider);
    CHIP_ERROR result = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, ControlSequenceOfOperation::Id }, seq);
    if (result != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to store ControlSequenceOfOperation attribute");
        return Status::Failure;
    }
    mControlSequenceOfOperation = seq;
    changed                     = true;
    return Status::Success;
}

DataModel::Nullable<temperature> ThermostatDelegate::GetLocalTemperature() const
{
    return mLocalTemperature;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetLocalTemperature(DataModel::Nullable<temperature> temp, bool & changed)
{
    if (mLocalTemperature == temp)
    {
        return Status::Success;
    }
    mLocalTemperature = temp;
    changed           = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::GetOutdoorTemperature(DataModel::Nullable<temperature> & outdoorTemp) const
{
    outdoorTemp = DataModel::NullNullable;
    return Status::Success;
}

int8_t ThermostatDelegate::GetLocalTemperatureCalibration() const
{
    return mLocalTemperatureCalibration;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetLocalTemperatureCalibration(int8_t temp, bool & changed)
{
    if (mLocalTemperatureCalibration == temp)
    {
        return Status::Success;
    }
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::InvalidInState);
    AttributePersistence persistence(*provider);
    CHIP_ERROR result = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, LocalTemperatureCalibration::Id }, temp);
    if (result != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to store LocalTemperatureCalibration attribute");
        return Status::Failure;
    }
    mLocalTemperatureCalibration = temp;
    changed                      = true;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const
{
    remoteSensing = mRemoteSensing;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatDelegate::SetRemoteSensing(BitMask<RemoteSensingBitmap> sensing, bool & changed)
{
    if (mRemoteSensing == sensing)
    {
        return Status::Success;
    }
    AttributePersistenceProvider * provider = mProvider != nullptr ? mProvider : GetAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, Status::InvalidInState);
    AttributePersistence persistence(*provider);
    CHIP_ERROR result = persistence.StoreNativeEndianValue({ mEndpointId, Thermostat::Id, RemoteSensing::Id }, sensing.Raw());
    if (result != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to store RemoteSensing attribute");
        return Status::Failure;
    }
    mRemoteSensing = sensing;
    changed        = true;
    return Status::Success;
}
