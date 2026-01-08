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
constexpr int16_t kDefaultMinHeatSetpointLimit    = 700;  // 7C (44.5 F) is the default
constexpr int16_t kDefaultMaxHeatSetpointLimit    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultAbsMinCoolSetpointLimit = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultAbsMaxCoolSetpointLimit = 3200; // 32C (90 F) is the default
constexpr int16_t kDefaultMinCoolSetpointLimit    = 1600; // 16C (61 F) is the default
constexpr int16_t kDefaultMaxCoolSetpointLimit    = 3200; // 32C (90 F) is the default
constexpr int8_t kDefaultDeadBand                 = 20;   // 2.0C is the default
constexpr int16_t kDefaultHeatingSetpoint         = 2000;
constexpr int16_t kDefaultCoolingSetpoint         = 2600;

struct SetpointLimits
{
    bool autoSupported      = false;
    bool heatSupported      = false;
    bool coolSupported      = false;
    bool occupancySupported = false;

    int16_t absMinHeatSetpointLimit;
    int16_t absMaxHeatSetpointLimit;
    int16_t minHeatSetpointLimit;
    int16_t maxHeatSetpointLimit;
    int16_t absMinCoolSetpointLimit;
    int16_t absMaxCoolSetpointLimit;
    int16_t minCoolSetpointLimit;
    int16_t maxCoolSetpointLimit;
    int16_t deadBand = 0;
    int16_t occupiedCoolingSetpoint;
    int16_t occupiedHeatingSetpoint;
    int16_t unoccupiedCoolingSetpoint;
    int16_t unoccupiedHeatingSetpoint;
};

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
