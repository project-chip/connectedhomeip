/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.devicecontroller.cluster.clusters

import java.util.ArrayList

class ThermostatCluster(private val endpointId: UShort) {
  class GetWeeklyScheduleResponse(
    val numberOfTransitionsForSequence: UByte,
    val dayOfWeekForSequence: UInt,
    val modeForSequence: UInt,
    val transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>
  )

  class LocalTemperatureAttribute(val value: Short?)

  class OutdoorTemperatureAttribute(val value: Short?)

  class TemperatureSetpointHoldDurationAttribute(val value: UShort?)

  class SetpointChangeAmountAttribute(val value: Short?)

  class OccupiedSetbackAttribute(val value: UByte?)

  class OccupiedSetbackMinAttribute(val value: UByte?)

  class OccupiedSetbackMaxAttribute(val value: UByte?)

  class UnoccupiedSetbackAttribute(val value: UByte?)

  class UnoccupiedSetbackMinAttribute(val value: UByte?)

  class UnoccupiedSetbackMaxAttribute(val value: UByte?)

  class ACCoilTemperatureAttribute(val value: Short?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun setpointRaiseLower(mode: UInt, amount: Byte) {
    // Implementation needs to be added here
  }

  suspend fun setpointRaiseLower(mode: UInt, amount: Byte, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun setWeeklySchedule(
    numberOfTransitionsForSequence: UByte,
    dayOfWeekForSequence: UInt,
    modeForSequence: UInt,
    transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>
  ) {
    // Implementation needs to be added here
  }

  suspend fun setWeeklySchedule(
    numberOfTransitionsForSequence: UByte,
    dayOfWeekForSequence: UInt,
    modeForSequence: UInt,
    transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun getWeeklySchedule(daysToReturn: UInt, modeToReturn: UInt): GetWeeklyScheduleResponse {
    // Implementation needs to be added here
  }

  suspend fun getWeeklySchedule(
    daysToReturn: UInt,
    modeToReturn: UInt,
    timedInvokeTimeoutMs: Int
  ): GetWeeklyScheduleResponse {
    // Implementation needs to be added here
  }

  suspend fun clearWeeklySchedule() {
    // Implementation needs to be added here
  }

  suspend fun clearWeeklySchedule(timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun readLocalTemperatureAttribute(): LocalTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLocalTemperatureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LocalTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOutdoorTemperatureAttribute(): OutdoorTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOutdoorTemperatureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OutdoorTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOccupancyAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupancyAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAbsMinHeatSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMinHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAbsMaxHeatSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMaxHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAbsMinCoolSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMinCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readAbsMaxCoolSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMaxCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPICoolingDemandAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePICoolingDemandAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readPIHeatingDemandAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribePIHeatingDemandAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readHVACSystemTypeConfigurationAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeHVACSystemTypeConfigurationAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeHVACSystemTypeConfigurationAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeHVACSystemTypeConfigurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readLocalTemperatureCalibrationAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeLocalTemperatureCalibrationAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeLocalTemperatureCalibrationAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLocalTemperatureCalibrationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedCoolingSetpointAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedCoolingSetpointAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedCoolingSetpointAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupiedCoolingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedHeatingSetpointAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedHeatingSetpointAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedHeatingSetpointAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupiedHeatingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedCoolingSetpointAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedCoolingSetpointAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedCoolingSetpointAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnoccupiedCoolingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedHeatingSetpointAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedHeatingSetpointAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedHeatingSetpointAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnoccupiedHeatingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMinHeatSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeMinHeatSetpointLimitAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeMinHeatSetpointLimitAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinHeatSetpointLimitAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMaxHeatSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeMaxHeatSetpointLimitAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeMaxHeatSetpointLimitAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxHeatSetpointLimitAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMinCoolSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeMinCoolSetpointLimitAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeMinCoolSetpointLimitAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinCoolSetpointLimitAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMaxCoolSetpointLimitAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeMaxCoolSetpointLimitAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeMaxCoolSetpointLimitAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxCoolSetpointLimitAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readMinSetpointDeadBandAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeMinSetpointDeadBandAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeMinSetpointDeadBandAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinSetpointDeadBandAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readRemoteSensingAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeRemoteSensingAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeRemoteSensingAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemoteSensingAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readControlSequenceOfOperationAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeControlSequenceOfOperationAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeControlSequenceOfOperationAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeControlSequenceOfOperationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readSystemModeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeSystemModeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeSystemModeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeSystemModeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readThermostatRunningModeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeThermostatRunningModeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readStartOfWeekAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeStartOfWeekAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfWeeklyTransitionsAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfWeeklyTransitionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfDailyTransitionsAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfDailyTransitionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readTemperatureSetpointHoldAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeTemperatureSetpointHoldAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readTemperatureSetpointHoldDurationAttribute():
    TemperatureSetpointHoldDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldDurationAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldDurationAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeTemperatureSetpointHoldDurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TemperatureSetpointHoldDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readThermostatProgrammingOperationModeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeThermostatProgrammingOperationModeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeThermostatProgrammingOperationModeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeThermostatProgrammingOperationModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readThermostatRunningStateAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeThermostatRunningStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Integer {
    // Implementation needs to be added here
  }

  suspend fun readSetpointChangeSourceAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointChangeSourceAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readSetpointChangeAmountAttribute(): SetpointChangeAmountAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointChangeAmountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SetpointChangeAmountAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSetpointChangeSourceTimestampAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointChangeSourceTimestampAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedSetbackAttribute(): OccupiedSetbackAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedSetbackAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedSetbackAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupiedSetbackAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OccupiedSetbackAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedSetbackMinAttribute(): OccupiedSetbackMinAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupiedSetbackMinAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OccupiedSetbackMinAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedSetbackMaxAttribute(): OccupiedSetbackMaxAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupiedSetbackMaxAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OccupiedSetbackMaxAttribute {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedSetbackAttribute(): UnoccupiedSetbackAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedSetbackAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedSetbackAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnoccupiedSetbackAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UnoccupiedSetbackAttribute {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedSetbackMinAttribute(): UnoccupiedSetbackMinAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnoccupiedSetbackMinAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UnoccupiedSetbackMinAttribute {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedSetbackMaxAttribute(): UnoccupiedSetbackMaxAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnoccupiedSetbackMaxAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UnoccupiedSetbackMaxAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEmergencyHeatDeltaAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeEmergencyHeatDeltaAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeEmergencyHeatDeltaAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEmergencyHeatDeltaAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACTypeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACTypeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeACTypeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACTypeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACCapacityAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACCapacityAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACRefrigerantTypeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACRefrigerantTypeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeACRefrigerantTypeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACRefrigerantTypeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACCompressorTypeAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACCompressorTypeAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeACCompressorTypeAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACCompressorTypeAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACErrorCodeAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeACErrorCodeAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeACErrorCodeAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACErrorCodeAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readACLouverPositionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACLouverPositionAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeACLouverPositionAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACLouverPositionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readACCoilTemperatureAttribute(): ACCoilTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeACCoilTemperatureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ACCoilTemperatureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readACCapacityformatAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityformatAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityformatAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeACCapacityformatAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 513u
  }
}
