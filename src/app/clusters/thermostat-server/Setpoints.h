/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

#include "SetpointAttributes.h"
#include "Temperature.h"
#include "SetpointLimits.h"
#include "SetpointRange.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class Setpoints
{
public:

    enum class ClampMode : uint8_t
    {
        kDontClamp,
        kClamp,
    };

    bool autoSupported      = false;
    bool heatSupported      = false;
    bool coolSupported      = false;
    bool occupancySupported = false;
    bool eventsSupported    = false;

    SetpointLimits absoluteHeatLimits;
    SetpointLimits absoluteCoolLimits;

    UserSetpointLimits userHeatLimits;
    UserSetpointLimits userCoolLimits;

    SetpointRange occupied;
    SetpointRange unoccupied;

    int16_t deadBand;

    Setpoints() :
        absoluteHeatLimits(SystemModeEnum::kHeat, kDefaultAbsMinHeatSetpointLimit, kDefaultAbsMaxHeatSetpointLimit),
        absoluteCoolLimits(SystemModeEnum::kCool, kDefaultAbsMinCoolSetpointLimit, kDefaultAbsMaxCoolSetpointLimit),
        userHeatLimits(absoluteHeatLimits), userCoolLimits(absoluteCoolLimits),
        occupied(SetpointAttributes::kOccupiedHeating, SetpointAttributes::kOccupiedCooling, kDefaultHeatingSetpoint, kDefaultCoolingSetpoint),
        unoccupied(SetpointAttributes::kUnoccupiedHeating, SetpointAttributes::kUnoccupiedCooling, kDefaultHeatingSetpoint, kDefaultCoolingSetpoint), 
        deadBand(kDefaultDeadBand)
    {}

    Setpoints(const Setpoints & spl) :
        autoSupported(spl.autoSupported), heatSupported(spl.heatSupported), coolSupported(spl.coolSupported),
        occupancySupported(spl.occupancySupported), eventsSupported(spl.eventsSupported),
        absoluteHeatLimits(spl.absoluteHeatLimits), absoluteCoolLimits(spl.absoluteCoolLimits),
        userHeatLimits(absoluteHeatLimits, spl.userHeatLimits),
        userCoolLimits(absoluteCoolLimits, spl.userCoolLimits), 
        occupied(spl.occupied), unoccupied(spl.unoccupied),
        deadBand(spl.deadBand)
    {}

    bool Valid();

    SetpointRange & GetRange(chip::app::Clusters::Thermostat::OccupancyBitmap occupancy);

    Protocols::InteractionModel::Status ChangeRange(SetpointRange range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool,
                                                    ClampMode clamp, chip::BitFlags<SetpointAttributes> & affectedAttributes);

    Protocols::InteractionModel::Status ChangeLimits(UserSetpointLimits & limitOverride, chip::Optional<int16_t> min,
                                                     chip::Optional<int16_t> max,
                                                     chip::BitFlags<SetpointAttributes> & affectedAttributes);

private: 
Protocols::InteractionModel::Status Fix(chip::BitFlags<SetpointAttributes> & changedAttributes);
void FixUserLimitDeadband( 
    Optional<temperature> & heatLimit, 
    Optional<temperature> & coolLimit,
    temperature absoluteHeatLimit,
    temperature absoluteCoolLimit,
    SetpointAttributes heatAttribute,
    SetpointAttributes coolAttribute,
    chip::BitFlags<SetpointAttributes> & changedAttributes,
    chip::BitFlags<SetpointAttributes> & fixedAttributes);
Protocols::InteractionModel::Status FixUserLimits(SetpointLimits & absoluteLimits, 
    UserSetpointLimits & userLimits, 
    SetpointAttributes minAttribute,
    SetpointAttributes maxAttribute,
    chip::BitFlags<SetpointAttributes> & changedAttributes,
    chip::BitFlags<SetpointAttributes> & fixedAttributes);
};

Protocols::InteractionModel::Status LoadSetpoints(EndpointId endpoint, Setpoints & setpoints);
Protocols::InteractionModel::Status SaveSetpoints(EndpointId endpoint, Setpoints setpoints,
                                                  chip::BitFlags<SetpointAttributes> affectedAttributes);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
