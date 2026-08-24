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

#include "../include/thermostat-hold-delegate-impl.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace Protocols::InteractionModel;

TemperatureSetpointHoldEnum ThermostatHoldDelegate::GetTemperatureSetpointHold() const
{
    return mTemperatureSetpointHold;
}

Protocols::InteractionModel::Status ThermostatHoldDelegate::SetTemperatureSetpointHold(TemperatureSetpointHoldEnum hold, bool & changed)
{
    if (mTemperatureSetpointHold == hold)
    {
        return Status::Success;
    }
    mTemperatureSetpointHold = hold;
    changed                  = true;
    return Status::Success;
}

DataModel::Nullable<uint16_t> ThermostatHoldDelegate::GetTemperatureSetpointHoldDuration() const
{
    return mTemperatureSetpointHoldDuration;
}

Protocols::InteractionModel::Status ThermostatHoldDelegate::SetTemperatureSetpointHoldDuration(DataModel::Nullable<uint16_t> duration,
                                                                                           bool & changed)
{
    if (mTemperatureSetpointHoldDuration == duration)
    {
        return Status::Success;
    }
    mTemperatureSetpointHoldDuration = duration;
    changed                          = true;
    return Status::Success;
}

DataModel::Nullable<uint32_t> ThermostatHoldDelegate::GetSetpointHoldExpiryTimestamp() const
{
    return mSetpointHoldExpiryTimestamp;
}

Protocols::InteractionModel::Status ThermostatHoldDelegate::SetSetpointHoldExpiryTimestamp(DataModel::Nullable<uint32_t> timestamp,
                                                                                       bool & changed)
{
    if (mSetpointHoldExpiryTimestamp == timestamp)
    {
        return Status::Success;
    }
    mSetpointHoldExpiryTimestamp = timestamp;
    changed                      = true;
    return Status::Success;
}
