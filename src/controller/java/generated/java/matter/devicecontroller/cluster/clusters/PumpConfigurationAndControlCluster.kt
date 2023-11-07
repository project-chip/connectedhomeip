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

class PumpConfigurationAndControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class MaxPressureAttribute(val value: Short?)

  class MaxSpeedAttribute(val value: UShort?)

  class MaxFlowAttribute(val value: UShort?)

  class MinConstPressureAttribute(val value: Short?)

  class MaxConstPressureAttribute(val value: Short?)

  class MinCompPressureAttribute(val value: Short?)

  class MaxCompPressureAttribute(val value: Short?)

  class MinConstSpeedAttribute(val value: UShort?)

  class MaxConstSpeedAttribute(val value: UShort?)

  class MinConstFlowAttribute(val value: UShort?)

  class MaxConstFlowAttribute(val value: UShort?)

  class MinConstTempAttribute(val value: Short?)

  class MaxConstTempAttribute(val value: Short?)

  class CapacityAttribute(val value: Short?)

  class SpeedAttribute(val value: UShort?)

  class LifetimeRunningHoursAttribute(val value: UInt?)

  class PowerAttribute(val value: UInt?)

  class LifetimeEnergyConsumedAttribute(val value: UInt?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readMaxPressureAttribute(): MaxPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxPressureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxSpeedAttribute(): MaxSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxSpeedAttribute(minInterval: Int, maxInterval: Int): MaxSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxFlowAttribute(): MaxFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxFlowAttribute(minInterval: Int, maxInterval: Int): MaxFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinConstPressureAttribute(): MinConstPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinConstPressureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinConstPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxConstPressureAttribute(): MaxConstPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxConstPressureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxConstPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinCompPressureAttribute(): MinCompPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinCompPressureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinCompPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxCompPressureAttribute(): MaxCompPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxCompPressureAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxCompPressureAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinConstSpeedAttribute(): MinConstSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinConstSpeedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinConstSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxConstSpeedAttribute(): MaxConstSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxConstSpeedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxConstSpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinConstFlowAttribute(): MinConstFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinConstFlowAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinConstFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxConstFlowAttribute(): MaxConstFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxConstFlowAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxConstFlowAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMinConstTempAttribute(): MinConstTempAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinConstTempAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MinConstTempAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxConstTempAttribute(): MaxConstTempAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxConstTempAttribute(
    minInterval: Int,
    maxInterval: Int
  ): MaxConstTempAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPumpStatusAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribePumpStatusAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readEffectiveOperationModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeEffectiveOperationModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readEffectiveControlModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeEffectiveControlModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCapacityAttribute(): CapacityAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCapacityAttribute(minInterval: Int, maxInterval: Int): CapacityAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSpeedAttribute(): SpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSpeedAttribute(minInterval: Int, maxInterval: Int): SpeedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLifetimeRunningHoursAttribute(): LifetimeRunningHoursAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeLifetimeRunningHoursAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeLifetimeRunningHoursAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LifetimeRunningHoursAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPowerAttribute(): PowerAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePowerAttribute(minInterval: Int, maxInterval: Int): PowerAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLifetimeEnergyConsumedAttribute(): LifetimeEnergyConsumedAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeLifetimeEnergyConsumedAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeLifetimeEnergyConsumedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LifetimeEnergyConsumedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOperationModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeOperationModeAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOperationModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readControlModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeControlModeAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeControlModeAttribute(minInterval: Int, maxInterval: Int): UByte {
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
    const val CLUSTER_ID: UInt = 512u
  }
}
