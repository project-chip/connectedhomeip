/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 * @brief APIs for generating events from the Thermostat cluster.
 *
 *******************************************************************************
 ******************************************************************************/

#pragma once

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

void GenerateSystemModeChangeEvent(chip::EndpointId endpoint,
                                   chip::Optional<chip::app::Clusters::Thermostat::SystemModeEnum> previousSystemMode,
                                   chip::app::Clusters::Thermostat::SystemModeEnum currentSystemMode);

void GenerateLocalTemperatureChangeEvent(chip::EndpointId endpoint,
                                         chip::app::DataModel::Nullable<int16_t> currentLocalTemperature);

void GenerateOccupancyChangeEvent(chip::EndpointId endpoint,
                                  chip::Optional<chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap>> previousOccupancy,
                                  chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap> currentOccupancy);

void GenerateSetpointChangeEvent(chip::EndpointId endpoint, chip::app::Clusters::Thermostat::SystemModeEnum systemMode,
                                 chip::Optional<chip::BitMask<chip::app::Clusters::Thermostat::OccupancyBitmap>> occupancy,
                                 chip::Optional<int16_t> previousSetpoint, int16_t currentSetpoint);

void GenerateRunningStateChangeEvent(
    chip::EndpointId endpoint,
    chip::Optional<chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap>> previousRunningState,
    chip::BitMask<chip::app::Clusters::Thermostat::RelayStateBitmap> currentRunningState);

void GenerateRunningModeChangeEvent(chip::EndpointId endpoint,
                                    chip::Optional<chip::app::Clusters::Thermostat::ThermostatRunningModeEnum> previousRunningMode,
                                    chip::app::Clusters::Thermostat::ThermostatRunningModeEnum currentRunningMode);

void GenerateActiveScheduleChangeEvent(chip::EndpointId endpoint,
                                       chip::Optional<chip::app::DataModel::Nullable<chip::ByteSpan>> previousScheduleHandle,
                                       chip::app::DataModel::Nullable<chip::ByteSpan> currentScheduleHandle);

void GenerateActivePresetChangeEvent(chip::EndpointId endpoint,
                                     chip::Optional<chip::app::DataModel::Nullable<chip::ByteSpan>> previousPresetHandle,
                                     chip::app::DataModel::Nullable<chip::ByteSpan> currentPresetHandle);

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
