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

#include "ThermostatSetpointsDelegate.h"

using chip::Protocols::InteractionModel::Status;

namespace chip::app {

Protocols::InteractionModel::Status
ThermostatSetpointsDelegate::GetOccupiedHeatingSetpoint(temperature & occupiedHeatingSetpoint) const
{
    occupiedHeatingSetpoint = mOccupiedHeatingSetpoint;
    return Status::Success;
}

Protocols::InteractionModel::Status ThermostatSetpointsDelegate::SetOccupiedHeatingSetpoint(temperature occupiedHeatingSetpoint,
                                                                                            bool & changed)
{
    changed = false;
    if (mOccupiedHeatingSetpoint == occupiedHeatingSetpoint)
    {
        return Status::Success;
    }

    mOccupiedHeatingSetpoint = occupiedHeatingSetpoint;
    changed                  = true;
    return Status::Success;
}

} // namespace chip::app
