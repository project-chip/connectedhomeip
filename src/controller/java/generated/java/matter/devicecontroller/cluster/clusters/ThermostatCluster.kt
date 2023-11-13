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

import matter.controller.MatterController
import matter.devicecontroller.cluster.structs.*

class ThermostatCluster(private val controller: MatterController, private val endpointId: UShort) {
  class GetWeeklyScheduleResponse(
    val numberOfTransitionsForSequence: UByte, 
    val dayOfWeekForSequence: UInt, 
    val modeForSequence: UInt, 
    val transitions: List<ThermostatClusterThermostatScheduleTransition>
  )

  class LocalTemperatureAttribute(
    val value: Short?
  )

  class OutdoorTemperatureAttribute(
    val value: Short?
  )

  class TemperatureSetpointHoldDurationAttribute(
    val value: UShort?
  )

  class SetpointChangeAmountAttribute(
    val value: Short?
  )

  class OccupiedSetbackAttribute(
    val value: UByte?
  )

  class OccupiedSetbackMinAttribute(
    val value: UByte?
  )

  class OccupiedSetbackMaxAttribute(
    val value: UByte?
  )

  class UnoccupiedSetbackAttribute(
    val value: UByte?
  )

  class UnoccupiedSetbackMinAttribute(
    val value: UByte?
  )

  class UnoccupiedSetbackMaxAttribute(
    val value: UByte?
  )

  class ACCoilTemperatureAttribute(
    val value: Short?
  )

  class PresetTypesAttribute(
    val value: List<ThermostatClusterPresetTypeStruct>?
  )

  class ScheduleTypesAttribute(
    val value: List<ThermostatClusterScheduleTypeStruct>?
  )

  class NumberOfScheduleTransitionsPerDayAttribute(
    val value: UByte?
  )

  class PresetsAttribute(
    val value: List<ThermostatClusterPresetStruct>?
  )

  class ActivePresetHandleAttribute(
    val value: ByteArray?
  )

  class SchedulesAttribute(
    val value: List<ThermostatClusterScheduleStruct>?
  )

  class ActiveScheduleHandleAttribute(
    val value: ByteArray?
  )

  class SetpointHoldPolicyExpiryTimestampAttribute(
    val value: UInt?
  )

  class QueuedPresetAttribute(
    val value: ThermostatClusterQueuedPresetStruct?
  )

  class GeneratedCommandListAttribute(
    val value: List<UInt>
  )

  class AcceptedCommandListAttribute(
    val value: List<UInt>
  )

  class EventListAttribute(
    val value: List<UInt>
  )

  class AttributeListAttribute(
    val value: List<UInt>
  )

  suspend fun setpointRaiseLower(mode: UInt, amount: Byte, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun setWeeklySchedule(numberOfTransitionsForSequence: UByte, dayOfWeekForSequence: UInt, modeForSequence: UInt, transitions: List<ThermostatClusterThermostatScheduleTransition>, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun getWeeklySchedule(daysToReturn: UInt, modeToReturn: UInt, timedInvokeTimeoutMs: Int? = null): GetWeeklyScheduleResponse {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun clearWeeklySchedule(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun getRelayStatusLog(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun setActiveScheduleRequest(scheduleHandle: ByteArray, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun setActivePresetRequest(presetHandle: ByteArray, delayMinutes: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun startSchedulesEditRequest(timeoutSeconds: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun cancelSchedulesEditRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun commitSchedulesEditRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 9L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun startPresetsEditRequest(timeoutSeconds: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 10L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun cancelPresetsEditRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 11L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun commitPresetsEditRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 12L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun cancelSetActivePresetRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 14L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
  }

  suspend fun setTemperatureSetpointHoldPolicy(temperatureSetpointHoldPolicy: UInt, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 15L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }    
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

  suspend fun readOccupancyAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupancyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readAbsMinHeatSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMinHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readAbsMaxHeatSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMaxHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readAbsMinCoolSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMinCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readAbsMaxCoolSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun subscribeAbsMaxCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readPICoolingDemandAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribePICoolingDemandAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readPIHeatingDemandAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribePIHeatingDemandAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readHVACSystemTypeConfigurationAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeHVACSystemTypeConfigurationAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeHVACSystemTypeConfigurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readLocalTemperatureCalibrationAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun writeLocalTemperatureCalibrationAttribute(
    value: Byte,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeLocalTemperatureCalibrationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Byte {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedCoolingSetpointAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedCoolingSetpointAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeOccupiedCoolingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedHeatingSetpointAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedHeatingSetpointAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeOccupiedHeatingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedCoolingSetpointAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedCoolingSetpointAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeUnoccupiedCoolingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readUnoccupiedHeatingSetpointAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeUnoccupiedHeatingSetpointAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeUnoccupiedHeatingSetpointAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMinHeatSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeMinHeatSetpointLimitAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeMinHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMaxHeatSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeMaxHeatSetpointLimitAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeMaxHeatSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMinCoolSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeMinCoolSetpointLimitAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeMinCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMaxCoolSetpointLimitAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeMaxCoolSetpointLimitAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeMaxCoolSetpointLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Short {
    // Implementation needs to be added here
  }

  suspend fun readMinSetpointDeadBandAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun writeMinSetpointDeadBandAttribute(
    value: Byte,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeMinSetpointDeadBandAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Byte {
    // Implementation needs to be added here
  }

  suspend fun readRemoteSensingAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeRemoteSensingAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeRemoteSensingAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readControlSequenceOfOperationAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeControlSequenceOfOperationAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeControlSequenceOfOperationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSystemModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeSystemModeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeSystemModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readThermostatRunningModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeThermostatRunningModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readStartOfWeekAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeStartOfWeekAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfWeeklyTransitionsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfWeeklyTransitionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfDailyTransitionsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfDailyTransitionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readTemperatureSetpointHoldAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeTemperatureSetpointHoldAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readTemperatureSetpointHoldDurationAttribute(): TemperatureSetpointHoldDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeTemperatureSetpointHoldDurationAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeTemperatureSetpointHoldDurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TemperatureSetpointHoldDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readThermostatProgrammingOperationModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeThermostatProgrammingOperationModeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeThermostatProgrammingOperationModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readThermostatRunningStateAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeThermostatRunningStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readSetpointChangeSourceAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointChangeSourceAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
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

  suspend fun readSetpointChangeSourceTimestampAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointChangeSourceTimestampAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UInt {
    // Implementation needs to be added here
  }

  suspend fun readOccupiedSetbackAttribute(): OccupiedSetbackAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOccupiedSetbackAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
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

  suspend fun writeUnoccupiedSetbackAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
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

  suspend fun readEmergencyHeatDeltaAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeEmergencyHeatDeltaAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeEmergencyHeatDeltaAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readACTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeACTypeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACTypeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readACCapacityAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACCapacityAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readACRefrigerantTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeACRefrigerantTypeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACRefrigerantTypeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readACCompressorTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeACCompressorTypeAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACCompressorTypeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readACErrorCodeAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeACErrorCodeAttribute(
    value: ULong,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACErrorCodeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UInt {
    // Implementation needs to be added here
  }

  suspend fun readACLouverPositionAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeACLouverPositionAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACLouverPositionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
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

  suspend fun readACCapacityformatAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeACCapacityformatAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null    
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }    
  }

  suspend fun subscribeACCapacityformatAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readPresetTypesAttribute(): PresetTypesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePresetTypesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PresetTypesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readScheduleTypesAttribute(): ScheduleTypesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeScheduleTypesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ScheduleTypesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfPresetsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfPresetsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfSchedulesAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfSchedulesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfScheduleTransitionsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfScheduleTransitionsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfScheduleTransitionsPerDayAttribute(): NumberOfScheduleTransitionsPerDayAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfScheduleTransitionsPerDayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NumberOfScheduleTransitionsPerDayAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPresetsAttribute(): PresetsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePresetsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): PresetsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPresetsEditableAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribePresetsEditableAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readActivePresetHandleAttribute(): ActivePresetHandleAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActivePresetHandleAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActivePresetHandleAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSchedulesAttribute(): SchedulesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSchedulesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SchedulesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSchedulesEditableAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeSchedulesEditableAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readActiveScheduleHandleAttribute(): ActiveScheduleHandleAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveScheduleHandleAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveScheduleHandleAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTemperatureSetpointHoldPolicyAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeTemperatureSetpointHoldPolicyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readSetpointHoldPolicyExpiryTimestampAttribute(): SetpointHoldPolicyExpiryTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetpointHoldPolicyExpiryTimestampAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SetpointHoldPolicyExpiryTimestampAttribute {
    // Implementation needs to be added here
  }

  suspend fun readQueuedPresetAttribute(): QueuedPresetAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeQueuedPresetAttribute(
    minInterval: Int,
    maxInterval: Int
  ): QueuedPresetAttribute {
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

  suspend fun subscribeEventListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): EventListAttribute {
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

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 513u
  }
}
