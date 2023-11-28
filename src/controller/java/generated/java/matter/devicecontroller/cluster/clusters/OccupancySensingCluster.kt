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

class OccupancySensingCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readOccupancyAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupancyAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readOccupancySensorTypeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupancySensorTypeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readOccupancySensorTypeBitmapAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeOccupancySensorTypeBitmapAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readPIROccupiedToUnoccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writePIROccupiedToUnoccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePIROccupiedToUnoccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPIRUnoccupiedToOccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writePIRUnoccupiedToOccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePIRUnoccupiedToOccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPIRUnoccupiedToOccupiedThresholdAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writePIRUnoccupiedToOccupiedThresholdAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePIRUnoccupiedToOccupiedThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readUltrasonicOccupiedToUnoccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeUltrasonicOccupiedToUnoccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeUltrasonicOccupiedToUnoccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readUltrasonicUnoccupiedToOccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeUltrasonicUnoccupiedToOccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeUltrasonicUnoccupiedToOccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readUltrasonicUnoccupiedToOccupiedThresholdAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeUltrasonicUnoccupiedToOccupiedThresholdAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeUltrasonicUnoccupiedToOccupiedThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalContactOccupiedToUnoccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writePhysicalContactOccupiedToUnoccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePhysicalContactOccupiedToUnoccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalContactUnoccupiedToOccupiedDelayAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writePhysicalContactUnoccupiedToOccupiedDelayAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePhysicalContactUnoccupiedToOccupiedDelayAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readPhysicalContactUnoccupiedToOccupiedThresholdAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writePhysicalContactUnoccupiedToOccupiedThresholdAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribePhysicalContactUnoccupiedToOccupiedThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UByte {
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
    const val CLUSTER_ID: UInt = 1030u
  }
}
