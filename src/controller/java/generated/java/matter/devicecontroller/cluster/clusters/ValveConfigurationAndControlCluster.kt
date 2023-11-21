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

class ValveConfigurationAndControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class OpenDurationAttribute(val value: UInt?)

  class AutoCloseTimeAttribute(val value: ULong?)

  class RemainingDurationAttribute(val value: UInt?)

  class CurrentStateAttribute(val value: UInt?)

  class TargetStateAttribute(val value: UInt?)

  class CurrentLevelAttribute(val value: UByte?)

  class TargetLevelAttribute(val value: UByte?)

  class OpenLevelAttribute(val value: UByte?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun open(openDuration: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun close(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun setLevel(level: UByte, openDuration: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readOpenDurationAttribute(): OpenDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOpenDurationAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOpenDurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): OpenDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAutoCloseTimeAttribute(): AutoCloseTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAutoCloseTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AutoCloseTimeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRemainingDurationAttribute(): RemainingDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemainingDurationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): RemainingDurationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentStateAttribute(): CurrentStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CurrentStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTargetStateAttribute(): TargetStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeTargetStateAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TargetStateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStartUpStateAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpStateAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeStartUpStateAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
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

  suspend fun readTargetLevelAttribute(): TargetLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeTargetLevelAttribute(
    minInterval: Int,
    maxInterval: Int
  ): TargetLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOpenLevelAttribute(): OpenLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOpenLevelAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOpenLevelAttribute(minInterval: Int, maxInterval: Int): OpenLevelAttribute {
    // Implementation needs to be added here
  }

  suspend fun readValveFaultAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeValveFaultAttribute(minInterval: Int, maxInterval: Int): UShort {
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
    const val CLUSTER_ID: UInt = 129u
  }
}
