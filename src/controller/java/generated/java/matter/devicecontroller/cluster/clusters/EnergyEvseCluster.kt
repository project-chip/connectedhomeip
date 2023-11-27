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

class EnergyEvseCluster(private val controller: MatterController, private val endpointId: UShort) {
  class GetTargetsResponse(
    val dayOfWeekforSequence: UInt,
    val chargingTargets: List<EnergyEvseClusterChargingTargetStruct>
  )

  class StateAttribute(val value: UInt?)

  class ChargingEnabledUntilAttribute(val value: UInt?)

  class DischargingEnabledUntilAttribute(val value: UInt?)

  class NextChargeStartTimeAttribute(val value: UInt?)

  class NextChargeTargetTimeAttribute(val value: UInt?)

  class NextChargeRequiredEnergyAttribute(val value: Long?)

  class NextChargeTargetSoCAttribute(val value: UByte?)

  class ApproximateEVEfficiencyAttribute(val value: UShort?)

  class StateOfChargeAttribute(val value: UByte?)

  class BatteryCapacityAttribute(val value: Long?)

  class VehicleIDAttribute(val value: String?)

  class SessionIDAttribute(val value: UInt?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun disable(timedInvokeTimeoutMs: Int) {
    val commandId = 1L

    // Implementation needs to be added here
  }

  suspend fun enableCharging(
    chargingEnabledUntil: UInt?,
    minimumChargeCurrent: Long,
    maximumChargeCurrent: Long,
    timedInvokeTimeoutMs: Int
  ) {
    val commandId = 2L

    // Implementation needs to be added here
  }

  suspend fun enableDischarging(
    dischargingEnabledUntil: UInt?,
    maximumDischargeCurrent: Long,
    timedInvokeTimeoutMs: Int
  ) {
    val commandId = 3L

    // Implementation needs to be added here
  }

  suspend fun startDiagnostics(timedInvokeTimeoutMs: Int) {
    val commandId = 4L

    // Implementation needs to be added here
  }

  suspend fun setTargets(
    dayOfWeekforSequence: UInt,
    chargingTargets: List<EnergyEvseClusterChargingTargetStruct>,
    timedInvokeTimeoutMs: Int
  ) {
    val commandId = 5L

    // Implementation needs to be added here
  }

  suspend fun getTargets(daysToReturn: UInt, timedInvokeTimeoutMs: Int): GetTargetsResponse {
    val commandId = 6L

    // Implementation needs to be added here
  }

  suspend fun clearTargets(timedInvokeTimeoutMs: Int) {
    val commandId = 7L

    // Implementation needs to be added here
  }

  suspend fun readStateAttribute(): StateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeStateAttribute(minInterval: Int, maxInterval: Int): StateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupplyStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupplyStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readFaultStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeFaultStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readChargingEnabledUntilAttribute(): ChargingEnabledUntilAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeChargingEnabledUntilAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ChargingEnabledUntilAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDischargingEnabledUntilAttribute(): DischargingEnabledUntilAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeDischargingEnabledUntilAttribute(
    minInterval: Int,
    maxInterval: Int
  ): DischargingEnabledUntilAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCircuitCapacityAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeCircuitCapacityAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readMinimumChargeCurrentAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinimumChargeCurrentAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readMaximumChargeCurrentAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaximumChargeCurrentAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readMaximumDischargeCurrentAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaximumDischargeCurrentAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readUserMaximumChargeCurrentAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeUserMaximumChargeCurrentAttribute(
    value: Long,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeUserMaximumChargeCurrentAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readRandomizationDelayWindowAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeRandomizationDelayWindowAttribute(
    value: UInt,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeRandomizationDelayWindowAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfWeeklyTargetsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfWeeklyTargetsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfDailyTargetsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfDailyTargetsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNextChargeStartTimeAttribute(): NextChargeStartTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNextChargeStartTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NextChargeStartTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNextChargeTargetTimeAttribute(): NextChargeTargetTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNextChargeTargetTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NextChargeTargetTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNextChargeRequiredEnergyAttribute(): NextChargeRequiredEnergyAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNextChargeRequiredEnergyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NextChargeRequiredEnergyAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNextChargeTargetSoCAttribute(): NextChargeTargetSoCAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNextChargeTargetSoCAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NextChargeTargetSoCAttribute {
    // Implementation needs to be added here
  }

  suspend fun readApproximateEVEfficiencyAttribute(): ApproximateEVEfficiencyAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeApproximateEVEfficiencyAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeApproximateEVEfficiencyAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ApproximateEVEfficiencyAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStateOfChargeAttribute(): StateOfChargeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeStateOfChargeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StateOfChargeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readBatteryCapacityAttribute(): BatteryCapacityAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeBatteryCapacityAttribute(
    minInterval: Int,
    maxInterval: Int
  ): BatteryCapacityAttribute {
    // Implementation needs to be added here
  }

  suspend fun readVehicleIDAttribute(): VehicleIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeVehicleIDAttribute(minInterval: Int, maxInterval: Int): VehicleIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSessionIDAttribute(): SessionIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSessionIDAttribute(minInterval: Int, maxInterval: Int): SessionIDAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSessionDurationAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeSessionDurationAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readSessionEnergyChargedAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeSessionEnergyChargedAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readSessionEnergyDischargedAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeSessionEnergyDischargedAttribute(minInterval: Int, maxInterval: Int): Long {
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

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 153u
  }
}
