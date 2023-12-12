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

class LevelControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class CurrentLevelAttribute(val value: UByte?)

  class OnLevelAttribute(val value: UByte?)

  class OnTransitionTimeAttribute(val value: UShort?)

  class OffTransitionTimeAttribute(val value: UShort?)

  class DefaultMoveRateAttribute(val value: UByte?)

  class StartUpCurrentLevelAttribute(val value: UByte?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun moveToLevel(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun move(
    moveMode: UInt,
    rate: UByte?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun step(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stop(optionsMask: UInt, optionsOverride: UInt, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun moveToLevelWithOnOff(
    level: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun moveWithOnOff(
    moveMode: UInt,
    rate: UByte?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stepWithOnOff(
    stepMode: UInt,
    stepSize: UByte,
    transitionTime: UShort?,
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stopWithOnOff(
    optionsMask: UInt,
    optionsOverride: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun moveToClosestFrequency(frequency: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readCurrentLevelAttribute(): CurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentLevelAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRemainingTimeAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemainingTimeAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readMinLevelAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinLevelAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readMaxLevelAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxLevelAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCurrentFrequencyAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentFrequencyAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readMinFrequencyAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeMinFrequencyAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readMaxFrequencyAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxFrequencyAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readOptionsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeOptionsAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOptionsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readOnOffTransitionTimeAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeOnOffTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOnOffTransitionTimeAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readOnLevelAttribute(): OnLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOnLevelAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOnLevelAttribute(minInterval: Int, maxInterval: Int): OnLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOnTransitionTimeAttribute(): OnTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOnTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOnTransitionTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OnTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOffTransitionTimeAttribute(): OffTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOffTransitionTimeAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOffTransitionTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OffTransitionTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readDefaultMoveRateAttribute(): DefaultMoveRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeDefaultMoveRateAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeDefaultMoveRateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): DefaultMoveRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStartUpCurrentLevelAttribute(): StartUpCurrentLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpCurrentLevelAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeStartUpCurrentLevelAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StartUpCurrentLevelAttribute {
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
    const val CLUSTER_ID: UInt = 8u
  }
}
