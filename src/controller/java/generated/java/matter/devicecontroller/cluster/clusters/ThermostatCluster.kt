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
  companion object {
    const val CLUSTER_ID: UInt = 513u
  }

  fun setpointRaiseLower(callback: DefaultClusterCallback, mode: Integer, amount: Integer) {
    // Implementation needs to be added here
  }

  fun setpointRaiseLower(
    callback: DefaultClusterCallback,
    mode: Integer,
    amount: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun setWeeklySchedule(
    callback: DefaultClusterCallback,
    numberOfTransitionsForSequence: Integer,
    dayOfWeekForSequence: Integer,
    modeForSequence: Integer,
    transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>
  ) {
    // Implementation needs to be added here
  }

  fun setWeeklySchedule(
    callback: DefaultClusterCallback,
    numberOfTransitionsForSequence: Integer,
    dayOfWeekForSequence: Integer,
    modeForSequence: Integer,
    transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getWeeklySchedule(
    callback: GetWeeklyScheduleResponseCallback,
    daysToReturn: Integer,
    modeToReturn: Integer
  ) {
    // Implementation needs to be added here
  }

  fun getWeeklySchedule(
    callback: GetWeeklyScheduleResponseCallback,
    daysToReturn: Integer,
    modeToReturn: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun clearWeeklySchedule(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun clearWeeklySchedule(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface GetWeeklyScheduleResponseCallback {
    fun onSuccess(
      numberOfTransitionsForSequence: Integer,
      dayOfWeekForSequence: Integer,
      modeForSequence: Integer,
      transitions: ArrayList<ChipStructs.ThermostatClusterThermostatScheduleTransition>
    )

    fun onError(error: Exception)
  }

  interface LocalTemperatureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OutdoorTemperatureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface TemperatureSetpointHoldDurationAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface SetpointChangeAmountAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OccupiedSetbackAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OccupiedSetbackMinAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface OccupiedSetbackMaxAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface UnoccupiedSetbackAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface UnoccupiedSetbackMinAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface UnoccupiedSetbackMaxAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ACCoilTemperatureAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readLocalTemperatureAttribute(callback: LocalTemperatureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLocalTemperatureAttribute(
    callback: LocalTemperatureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOutdoorTemperatureAttribute(callback: OutdoorTemperatureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOutdoorTemperatureAttribute(
    callback: OutdoorTemperatureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupancyAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupancyAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAbsMinHeatSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAbsMinHeatSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAbsMaxHeatSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAbsMaxHeatSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAbsMinCoolSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAbsMinCoolSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAbsMaxCoolSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAbsMaxCoolSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPICoolingDemandAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePICoolingDemandAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPIHeatingDemandAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePIHeatingDemandAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readHVACSystemTypeConfigurationAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeHVACSystemTypeConfigurationAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeHVACSystemTypeConfigurationAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeHVACSystemTypeConfigurationAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocalTemperatureCalibrationAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLocalTemperatureCalibrationAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeLocalTemperatureCalibrationAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLocalTemperatureCalibrationAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupiedCoolingSetpointAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOccupiedCoolingSetpointAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOccupiedCoolingSetpointAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOccupiedCoolingSetpointAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupiedHeatingSetpointAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOccupiedHeatingSetpointAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOccupiedHeatingSetpointAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOccupiedHeatingSetpointAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnoccupiedCoolingSetpointAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedCoolingSetpointAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedCoolingSetpointAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUnoccupiedCoolingSetpointAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnoccupiedHeatingSetpointAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedHeatingSetpointAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedHeatingSetpointAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUnoccupiedHeatingSetpointAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinHeatSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMinHeatSetpointLimitAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMinHeatSetpointLimitAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMinHeatSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxHeatSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMaxHeatSetpointLimitAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMaxHeatSetpointLimitAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMaxHeatSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinCoolSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMinCoolSetpointLimitAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMinCoolSetpointLimitAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMinCoolSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxCoolSetpointLimitAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMaxCoolSetpointLimitAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMaxCoolSetpointLimitAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMaxCoolSetpointLimitAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMinSetpointDeadBandAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeMinSetpointDeadBandAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeMinSetpointDeadBandAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeMinSetpointDeadBandAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRemoteSensingAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRemoteSensingAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeRemoteSensingAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRemoteSensingAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readControlSequenceOfOperationAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeControlSequenceOfOperationAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeControlSequenceOfOperationAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeControlSequenceOfOperationAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSystemModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeSystemModeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeSystemModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeSystemModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readThermostatRunningModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeThermostatRunningModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStartOfWeekAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeStartOfWeekAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfWeeklyTransitionsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfWeeklyTransitionsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNumberOfDailyTransitionsAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNumberOfDailyTransitionsAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTemperatureSetpointHoldAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeTemperatureSetpointHoldAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeTemperatureSetpointHoldAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeTemperatureSetpointHoldAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTemperatureSetpointHoldDurationAttribute(
    callback: TemperatureSetpointHoldDurationAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeTemperatureSetpointHoldDurationAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeTemperatureSetpointHoldDurationAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeTemperatureSetpointHoldDurationAttribute(
    callback: TemperatureSetpointHoldDurationAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readThermostatProgrammingOperationModeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeThermostatProgrammingOperationModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeThermostatProgrammingOperationModeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeThermostatProgrammingOperationModeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readThermostatRunningStateAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeThermostatRunningStateAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSetpointChangeSourceAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSetpointChangeSourceAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSetpointChangeAmountAttribute(callback: SetpointChangeAmountAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSetpointChangeAmountAttribute(
    callback: SetpointChangeAmountAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSetpointChangeSourceTimestampAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSetpointChangeSourceTimestampAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupiedSetbackAttribute(callback: OccupiedSetbackAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOccupiedSetbackAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeOccupiedSetbackAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOccupiedSetbackAttribute(
    callback: OccupiedSetbackAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupiedSetbackMinAttribute(callback: OccupiedSetbackMinAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupiedSetbackMinAttribute(
    callback: OccupiedSetbackMinAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOccupiedSetbackMaxAttribute(callback: OccupiedSetbackMaxAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeOccupiedSetbackMaxAttribute(
    callback: OccupiedSetbackMaxAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnoccupiedSetbackAttribute(callback: UnoccupiedSetbackAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedSetbackAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeUnoccupiedSetbackAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUnoccupiedSetbackAttribute(
    callback: UnoccupiedSetbackAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnoccupiedSetbackMinAttribute(callback: UnoccupiedSetbackMinAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeUnoccupiedSetbackMinAttribute(
    callback: UnoccupiedSetbackMinAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnoccupiedSetbackMaxAttribute(callback: UnoccupiedSetbackMaxAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeUnoccupiedSetbackMaxAttribute(
    callback: UnoccupiedSetbackMaxAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEmergencyHeatDeltaAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEmergencyHeatDeltaAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeEmergencyHeatDeltaAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEmergencyHeatDeltaAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACTypeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACTypeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACCapacityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACCapacityAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACCapacityAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACCapacityAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACRefrigerantTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACRefrigerantTypeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACRefrigerantTypeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACRefrigerantTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACCompressorTypeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACCompressorTypeAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACCompressorTypeAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACCompressorTypeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACErrorCodeAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACErrorCodeAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeACErrorCodeAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACErrorCodeAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACLouverPositionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACLouverPositionAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACLouverPositionAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACLouverPositionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACCoilTemperatureAttribute(callback: ACCoilTemperatureAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeACCoilTemperatureAttribute(
    callback: ACCoilTemperatureAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readACCapacityformatAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeACCapacityformatAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeACCapacityformatAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeACCapacityformatAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
