/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

constexpr int16_t kDefaultAbsMinHeatSetpointLimit = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultAbsMaxHeatSetpointLimit = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultDeadBand                = 200;  // 2.0C is the default
constexpr int16_t kDefaultHeatingSetpoint         = 2000;
constexpr int16_t kDefaultCoolingSetpoint         = 2600;

enum class SetpointAttributes : uint8_t
{
    kOccupiedHeating   = 0x1,
    kOccupiedCooling   = 0x2,
    kUnoccupiedHeating = 0x4,
    kUnoccupiedCooling = 0x8,
};

struct SetpointRange
{
    bool occupied;
    int16_t & heatingSetpoint;
    int16_t & coolingSetpoint;

    SetpointRange(bool occupied, int16_t & heat, int16_t & cool) : occupied(occupied), heatingSetpoint(heat), coolingSetpoint(cool)
    {}
};

struct SetpointLimits
{
    int16_t min;
    int16_t max;

    SetpointLimits(int16_t min, int16_t max) : min(min), max(max) {}

    int16_t Clamp(int16_t setpoint);
};

class Setpoints
{
public:
    bool autoSupported      = false;
    bool heatSupported      = false;
    bool coolSupported      = false;
    bool occupancySupported = false;

    int16_t absMinHeatSetpointLimit;
    int16_t absMaxHeatSetpointLimit;
    chip::Optional<int16_t> minHeatSetpointLimit;
    chip::Optional<int16_t> maxHeatSetpointLimit;
    int16_t absMinCoolSetpointLimit;
    int16_t absMaxCoolSetpointLimit;
    chip::Optional<int16_t> minCoolSetpointLimit;
    chip::Optional<int16_t> maxCoolSetpointLimit;
    int16_t deadBand = 0;

    int16_t occupiedCoolingSetpoint;
    int16_t occupiedHeatingSetpoint;
    int16_t unoccupiedCoolingSetpoint;
    int16_t unoccupiedHeatingSetpoint;

    Setpoints()
    {
        absMinHeatSetpointLimit   = kDefaultAbsMinHeatSetpointLimit;
        absMaxHeatSetpointLimit   = kDefaultAbsMaxHeatSetpointLimit;
        absMinCoolSetpointLimit   = kDefaultAbsMinCoolSetpointLimit;
        absMaxCoolSetpointLimit   = kDefaultAbsMaxCoolSetpointLimit;
        deadBand                  = kDefaultDeadBand;
        occupiedCoolingSetpoint   = kDefaultCoolingSetpoint;
        occupiedHeatingSetpoint   = kDefaultHeatingSetpoint;
        unoccupiedCoolingSetpoint = kDefaultCoolingSetpoint;
        unoccupiedHeatingSetpoint = kDefaultHeatingSetpoint;
    }

    Setpoints(const Setpoints & spl)
    {
        autoSupported             = spl.autoSupported;
        heatSupported             = spl.heatSupported;
        coolSupported             = spl.coolSupported;
        occupancySupported        = spl.occupancySupported;
        absMinHeatSetpointLimit   = spl.absMinHeatSetpointLimit;
        absMaxHeatSetpointLimit   = spl.absMaxHeatSetpointLimit;
        minHeatSetpointLimit      = spl.minHeatSetpointLimit;
        maxHeatSetpointLimit      = spl.maxHeatSetpointLimit;
        absMinCoolSetpointLimit   = spl.absMinCoolSetpointLimit;
        absMaxCoolSetpointLimit   = spl.absMaxCoolSetpointLimit;
        minCoolSetpointLimit      = spl.minCoolSetpointLimit;
        maxCoolSetpointLimit      = spl.maxCoolSetpointLimit;
        deadBand                  = spl.deadBand;
        occupiedCoolingSetpoint   = spl.occupiedCoolingSetpoint;
        occupiedHeatingSetpoint   = spl.occupiedHeatingSetpoint;
        unoccupiedCoolingSetpoint = spl.unoccupiedCoolingSetpoint;
        unoccupiedHeatingSetpoint = spl.unoccupiedHeatingSetpoint;
    }

    bool Valid();

    SetpointLimits GetEffectiveLimits(chip::app::Clusters::Thermostat::SystemModeEnum mode);
    SetpointRange GetSetpointRange(chip::app::Clusters::Thermostat::OccupancyBitmap occupancy);

    Protocols::InteractionModel::Status Change(SetpointRange range, chip::Optional<int16_t> heat, chip::Optional<int16_t> cool);
    Protocols::InteractionModel::Status ChangeLimits(chip::app::Clusters::Thermostat::SystemModeEnum mode,
                                                     chip::Optional<int16_t> min, chip::Optional<int16_t> max);

private:
    bool CheckSetpoints(int16_t occupiedSetpoint, int16_t unoccupiedSetpoint, chip::Optional<int16_t> min,
                        chip::Optional<int16_t> max, int16_t absMin, int16_t absMax);
};

Protocols::InteractionModel::Status GetSetpoints(EndpointId endpoint, Setpoints & setpoints);

int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint, EndpointId endpoint);
int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint, EndpointId endpoint);

Protocols::InteractionModel::Status CheckHeatingSetpointDeadband(bool autoSupported, int16_t newCoolingSetpoint,
                                                                 int16_t minHeatingSetpoint, int16_t deadband);

Protocols::InteractionModel::Status CheckCoolingSetpointDeadband(bool autoSupported, int16_t newHeatingSetpoint,
                                                                 int16_t maxCoolingSetpoint, int16_t deadband);

void EnsureDeadband(const ConcreteAttributePath & attributePath);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
