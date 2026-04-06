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
#include "SetpointDefaults.h"
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

    SetpointLimitOverride heatLimitsOverride;
    SetpointLimitOverride coolLimitsOverride;

    int16_t deadBand = 0;

    int16_t occupiedCoolingSetpoint;
    int16_t occupiedHeatingSetpoint;
    int16_t unoccupiedCoolingSetpoint;
    int16_t unoccupiedHeatingSetpoint;

    Setpoints() :
        absoluteHeatLimits(SystemModeEnum::kHeat), absoluteCoolLimits(SystemModeEnum::kCool),
        heatLimitsOverride(absoluteHeatLimits), coolLimitsOverride(absoluteCoolLimits), deadBand(kDefaultDeadBand),
        occupiedCoolingSetpoint(kDefaultCoolingSetpoint), occupiedHeatingSetpoint(kDefaultHeatingSetpoint),
        unoccupiedCoolingSetpoint(kDefaultCoolingSetpoint), unoccupiedHeatingSetpoint(kDefaultHeatingSetpoint)
    {}

    Setpoints(const Setpoints & spl) :
        autoSupported(spl.autoSupported), heatSupported(spl.heatSupported), coolSupported(spl.coolSupported),
        occupancySupported(spl.occupancySupported), eventsSupported(spl.eventsSupported),
        absoluteHeatLimits(spl.absoluteHeatLimits), absoluteCoolLimits(spl.absoluteCoolLimits),
        heatLimitsOverride(absoluteHeatLimits, spl.heatLimitsOverride),
        coolLimitsOverride(absoluteCoolLimits, spl.coolLimitsOverride), deadBand(spl.deadBand),
        occupiedCoolingSetpoint(spl.occupiedCoolingSetpoint), occupiedHeatingSetpoint(spl.occupiedHeatingSetpoint),
        unoccupiedCoolingSetpoint(spl.unoccupiedCoolingSetpoint), unoccupiedHeatingSetpoint(spl.unoccupiedHeatingSetpoint)
    {}

    bool Valid();

    SetpointRange GetRange(chip::app::Clusters::Thermostat::OccupancyBitmap occupancy);

    Protocols::InteractionModel::Status ChangeRange(SetpointRange range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool,
                                                    ClampMode clamp, chip::BitFlags<SetpointAttributes> & affectedAttributes);

    Protocols::InteractionModel::Status ChangeLimits(SetpointLimitOverride & limitOverride, chip::Optional<int16_t> min,
                                                     chip::Optional<int16_t> max,
                                                     chip::BitFlags<SetpointAttributes> & affectedAttributes);
};

Protocols::InteractionModel::Status LoadSetpoints(EndpointId endpoint, Setpoints & setpoints);
Protocols::InteractionModel::Status SaveSetpoints(EndpointId endpoint, Setpoints setpoints,
                                                  chip::BitFlags<SetpointAttributes> affectedAttributes);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
