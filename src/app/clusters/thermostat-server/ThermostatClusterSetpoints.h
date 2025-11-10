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
#include <app/persistence/AttributePersistence.h>
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

class Setpoints
{

public:
    int16_t mAbsMinHeatSetpointLimit;
    int16_t mAbsMaxHeatSetpointLimit;
    int16_t mMinHeatSetpointLimit;
    int16_t mMaxHeatSetpointLimit;
    int16_t mAbsMinCoolSetpointLimit;
    int16_t mAbsMaxCoolSetpointLimit;
    int16_t mMinCoolSetpointLimit;
    int16_t mMaxCoolSetpointLimit;
    int16_t mDeadBand;
    int16_t mOccupiedCoolingSetpoint;
    int16_t mOccupiedHeatingSetpoint;
    int16_t mUnoccupiedCoolingSetpoint;
    int16_t mUnoccupiedHeatingSetpoint;

    CHIP_ERROR LoadSetpointLimits(EndpointId endpoint, AttributePersistence & persistence);

    DataModel::ActionReturnStatus ChangeSetpoint(int16_t setpoint, bool isHeating, bool autoSupported, bool occupied,
                                                 bool & deadbandShift, AttributeValueDecoder & decoder,
                                                 AttributePersistence & persistence);

    DataModel::ActionReturnStatus ChangeSetpointLimit(AttributeId attribute, int16_t setpoint, bool autoSupported,
                                                      AttributeValueDecoder & decoder, AttributePersistence & persistence);

    DataModel::ActionReturnStatus ChangeSetpointDeadBand(int16_t deadBand, bool autoSupported, AttributeValueDecoder & decoder,
                                                         AttributePersistence & persistence);

    DataModel::ActionReturnStatus RaiseLowerSetpoint(int16_t amount, SetpointRaiseLowerModeEnum mode,
                                                     const BitFlags<Thermostat::Feature> features, bool occupied,
                                                     AttributePersistenceProvider & persistenceProvider);

    int16_t EnforceHeatingSetpointLimits(int16_t HeatingSetpoint);
    int16_t EnforceCoolingSetpointLimits(int16_t CoolingSetpoint);

private:
    EndpointId mEndpoint;
};

Protocols::InteractionModel::Status CheckHeatingSetpointDeadband(bool autoSupported, int16_t newCoolingSetpoint,
                                                                 int16_t minHeatingSetpoint, int16_t deadband);

Protocols::InteractionModel::Status CheckCoolingSetpointDeadband(bool autoSupported, int16_t newHeatingSetpoint,
                                                                 int16_t maxCoolingSetpoint, int16_t deadband);

// void EnsureDeadband(const ConcreteAttributePath & attributePath);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
