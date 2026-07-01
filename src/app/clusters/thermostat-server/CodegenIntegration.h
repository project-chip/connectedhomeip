/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/// Returns the code-driven Thermostat cluster instance registered for the given endpoint, or
/// nullptr if no cluster is registered there.
ThermostatCluster * FindClusterOnEndpoint(EndpointId endpointId);

namespace Attributes {

namespace LocalTemperature {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value);
} // namespace LocalTemperature

namespace OutdoorTemperature {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value);
} // namespace OutdoorTemperature

namespace Occupancy {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<OccupancyBitmap> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<OccupancyBitmap> value);
} // namespace Occupancy

namespace LocalTemperatureCalibration {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int8_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int8_t value);
} // namespace LocalTemperatureCalibration

namespace OccupiedCoolingSetpoint {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace OccupiedCoolingSetpoint

namespace OccupiedHeatingSetpoint {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace OccupiedHeatingSetpoint

namespace UnoccupiedCoolingSetpoint {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace UnoccupiedCoolingSetpoint

namespace UnoccupiedHeatingSetpoint {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace UnoccupiedHeatingSetpoint

namespace MinHeatSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace MinHeatSetpointLimit

namespace MaxHeatSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace MaxHeatSetpointLimit

namespace MinCoolSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace MinCoolSetpointLimit

namespace MaxCoolSetpointLimit {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int16_t value);
} // namespace MaxCoolSetpointLimit

namespace MinSetpointDeadBand {
Protocols::InteractionModel::Status Get(EndpointId endpoint, int8_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, int8_t value);
} // namespace MinSetpointDeadBand

namespace RemoteSensing {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<RemoteSensingBitmap> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<RemoteSensingBitmap> value);
} // namespace RemoteSensing

namespace ControlSequenceOfOperation {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ControlSequenceOfOperationEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ControlSequenceOfOperationEnum value);
} // namespace ControlSequenceOfOperation

namespace SystemMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, SystemModeEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, SystemModeEnum value);
} // namespace SystemMode

namespace ThermostatRunningMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ThermostatRunningModeEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ThermostatRunningModeEnum value);
} // namespace ThermostatRunningMode

namespace TemperatureSetpointHold {
Protocols::InteractionModel::Status Get(EndpointId endpoint, TemperatureSetpointHoldEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, TemperatureSetpointHoldEnum value);
} // namespace TemperatureSetpointHold

namespace TemperatureSetpointHoldDuration {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<uint16_t> value);
} // namespace TemperatureSetpointHoldDuration

namespace ThermostatRunningState {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<RelayStateBitmap> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<RelayStateBitmap> value);
} // namespace ThermostatRunningState

namespace SetpointChangeSource {
Protocols::InteractionModel::Status Get(EndpointId endpoint, SetpointChangeSourceEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, SetpointChangeSourceEnum value);
} // namespace SetpointChangeSource

namespace SetpointChangeAmount {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value);
} // namespace SetpointChangeAmount

namespace SetpointChangeSourceTimestamp {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
} // namespace SetpointChangeSourceTimestamp

namespace EmergencyHeatDelta {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
} // namespace EmergencyHeatDelta

namespace ACType {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ACTypeEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ACTypeEnum value);
} // namespace ACType

namespace ACCapacity {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value);
} // namespace ACCapacity

namespace ACRefrigerantType {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ACRefrigerantTypeEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ACRefrigerantTypeEnum value);
} // namespace ACRefrigerantType

namespace ACCompressorType {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ACCompressorTypeEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ACCompressorTypeEnum value);
} // namespace ACCompressorType

namespace ACErrorCode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<ACErrorCodeBitmap> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<ACErrorCodeBitmap> value);
} // namespace ACErrorCode

namespace ACLouverPosition {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ACLouverPositionEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ACLouverPositionEnum value);
} // namespace ACLouverPosition

namespace ACCoilTemperature {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<int16_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<int16_t> value);
} // namespace ACCoilTemperature

namespace ACCapacityformat {
Protocols::InteractionModel::Status Get(EndpointId endpoint, ACCapacityFormatEnum & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, ACCapacityFormatEnum value);
} // namespace ACCapacityformat

namespace NumberOfSchedules {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
} // namespace NumberOfSchedules

namespace NumberOfScheduleTransitions {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
} // namespace NumberOfScheduleTransitions

namespace NumberOfScheduleTransitionPerDay {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint8_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<uint8_t> value);
} // namespace NumberOfScheduleTransitionPerDay

namespace SetpointHoldExpiryTimestamp {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DataModel::Nullable<uint32_t> value);
} // namespace SetpointHoldExpiryTimestamp

} // namespace Attributes

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
