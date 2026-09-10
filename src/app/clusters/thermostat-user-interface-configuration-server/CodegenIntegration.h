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

#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>

namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration {

ThermostatUserInterfaceConfigurationCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * Live Thermostat User Interface Configuration attribute access for the code-driven server cluster
 * (reads and writes `ThermostatUserInterfaceConfigurationCluster` runtime state).
 *
 * For startup values stored in the ZAP/ember attribute store, use the generated `GetDefault` functions in
 * `app-common/zap-generated/attributes/Accessors.h`.
 */
namespace Attributes {

namespace TemperatureDisplayMode {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value);

} // namespace TemperatureDisplayMode

namespace KeypadLockout {

Protocols::InteractionModel::Status Get(EndpointId endpoint, ThermostatUserInterfaceConfiguration::KeypadLockoutEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value);

} // namespace KeypadLockout

namespace ScheduleProgrammingVisibility {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum value);

} // namespace ScheduleProgrammingVisibility

} // namespace Attributes

} // namespace chip::app::Clusters::ThermostatUserInterfaceConfiguration
