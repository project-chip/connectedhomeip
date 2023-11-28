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

class ModeSelectCluster(private val controller: MatterController, private val endpointId: UShort) {
  class StandardNamespaceAttribute(val value: UInt?)

  class SupportedModesAttribute(val value: List<ModeSelectClusterModeOptionStruct>)

  class StartUpModeAttribute(val value: UByte?)

  class OnModeAttribute(val value: UByte?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun changeToMode(newMode: UByte, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readDescriptionAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeDescriptionAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readStandardNamespaceAttribute(): StandardNamespaceAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeStandardNamespaceAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StandardNamespaceAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupportedModesAttribute(): SupportedModesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedModesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): SupportedModesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readCurrentModeAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentModeAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readStartUpModeAttribute(): StartUpModeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeStartUpModeAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeStartUpModeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StartUpModeAttribute {
    // Implementation needs to be added here
  }

  suspend fun readOnModeAttribute(): OnModeAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeOnModeAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeOnModeAttribute(minInterval: Int, maxInterval: Int): OnModeAttribute {
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
    const val CLUSTER_ID: UInt = 80u
  }
}
