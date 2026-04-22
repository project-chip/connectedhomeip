/**
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

/****************************************************************************
 * @file
 * @brief Backwards-compatibility functions for various previously ember-backed attributes
 *
 ******************************************************************************/

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

namespace Clusters {

namespace Thermostat {
namespace Attributes {

namespace ControlSequenceOfOperation {
Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum value);
} // namespace ControlSequenceOfOperation

namespace LocalTemperature {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);

} // namespace LocalTemperature

namespace OccupiedCoolingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value); // temperature
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);

} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {

Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t * value); // temperature
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);

} // namespace OccupiedHeatingSetpoint

namespace SystemMode {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        chip::app::Clusters::Thermostat::SystemModeEnum * value); // SystemModeEnum
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum value);

} // namespace SystemMode

namespace ThermostatRunningMode {

Protocols::InteractionModel::Status
Get(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum * value); // ThermostatRunningModeEnum
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::Thermostat::ThermostatRunningModeEnum value);

} // namespace ThermostatRunningMode

namespace FeatureMap {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
}

} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
