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

class DemandResponseLoadControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class LoadControlProgramsAttribute(
    val value: List<DemandResponseLoadControlClusterLoadControlProgramStruct>
  )

  class EventsAttribute(val value: List<DemandResponseLoadControlClusterLoadControlEventStruct>)

  class ActiveEventsAttribute(
    val value: List<DemandResponseLoadControlClusterLoadControlEventStruct>
  )

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun registerLoadControlProgramRequest(
    loadControlProgram: DemandResponseLoadControlClusterLoadControlProgramStruct,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun unregisterLoadControlProgramRequest(
    loadControlProgramID: ByteArray,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun addLoadControlEventRequest(
    event: DemandResponseLoadControlClusterLoadControlEventStruct,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun removeLoadControlEventRequest(
    eventID: ByteArray,
    cancelControl: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun clearLoadControlEventsRequest(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readDeviceClassAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeDeviceClassAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readLoadControlProgramsAttribute(): LoadControlProgramsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLoadControlProgramsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LoadControlProgramsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfLoadControlProgramsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfLoadControlProgramsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readEventsAttribute(): EventsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventsAttribute(minInterval: Int, maxInterval: Int): EventsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readActiveEventsAttribute(): ActiveEventsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveEventsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveEventsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfEventsPerProgramAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfEventsPerProgramAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readNumberOfTransistionsAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNumberOfTransistionsAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readDefaultRandomStartAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeDefaultRandomStartAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeDefaultRandomStartAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readDefaultRandomDurationAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeDefaultRandomDurationAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeDefaultRandomDurationAttribute(minInterval: Int, maxInterval: Int): UByte {
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
    const val CLUSTER_ID: UInt = 150u
  }
}
