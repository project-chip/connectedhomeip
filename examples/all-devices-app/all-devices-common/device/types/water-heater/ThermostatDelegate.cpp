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

#include "ThermostatDelegate.h"
 
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

 
 SystemModeEnum ThermostatDelegate::GetSystemMode() const
 {
     return mSystemMode;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetSystemMode(SystemModeEnum systemMode, bool & changed)
 {
     changed = false;
     if (mSystemMode == systemMode)
     {
         return Status::Success;
     }

     mSystemMode = systemMode;
     changed     = true;
     return Status::Success;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::GetRunningMode(ThermostatRunningModeEnum & runningMode) const
 {
     return Status::UnsupportedAttribute;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetRunningMode(ThermostatRunningModeEnum runningMode, bool & changed)
 {
     return Status::UnsupportedAttribute;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::GetRunningState(BitMask<RelayStateBitmap> & runningState) const
 {
     return Status::UnsupportedAttribute;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetRunningState(BitMask<RelayStateBitmap> runningState, bool & changed)
 {
     return Status::UnsupportedAttribute;
 }
 
 ControlSequenceOfOperationEnum ThermostatDelegate::GetControlSequenceOfOperation() const
 {
     return mControlSequenceOfOperation;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetControlSequenceOfOperation(ControlSequenceOfOperationEnum seq,
                                                                                       bool & changed)
 {
     changed = false;
     if (mControlSequenceOfOperation == seq)
     {
         return Status::Success;
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
     changed = false;
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
     return Status::UnsupportedAttribute;
 }
 
 int8_t ThermostatDelegate::GetLocalTemperatureCalibration() const
 {
     return 0;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetLocalTemperatureCalibration(int8_t temp, bool & changed)
 {
     return Status::UnsupportedAttribute;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::GetRemoteSensing(BitMask<RemoteSensingBitmap> & remoteSensing) const
 {
     return Status::UnsupportedAttribute;
 }
 
 Protocols::InteractionModel::Status ThermostatDelegate::SetRemoteSensing(BitMask<RemoteSensingBitmap> sensing, bool & changed)
 {
     return Status::UnsupportedAttribute;
 }
 