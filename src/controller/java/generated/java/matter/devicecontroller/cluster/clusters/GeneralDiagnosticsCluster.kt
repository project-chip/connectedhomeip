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

class GeneralDiagnosticsCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class TimeSnapshotResponse(val systemTimeUs: ULong, val UTCTimeUs: ULong?)

  class NetworkInterfacesAttribute(val value: List<GeneralDiagnosticsClusterNetworkInterface>)

  class ActiveHardwareFaultsAttribute(val value: List<UInt>?)

  class ActiveRadioFaultsAttribute(val value: List<UInt>?)

  class ActiveNetworkFaultsAttribute(val value: List<UInt>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun testEventTrigger(
    enableKey: ByteArray,
    eventTrigger: ULong,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun timeSnapshot(timedInvokeTimeoutMs: Int? = null): TimeSnapshotResponse {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readNetworkInterfacesAttribute(): NetworkInterfacesAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeNetworkInterfacesAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NetworkInterfacesAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRebootCountAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeRebootCountAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readUpTimeAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribeUpTimeAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readTotalOperationalHoursAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeTotalOperationalHoursAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readBootReasonAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeBootReasonAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readActiveHardwareFaultsAttribute(): ActiveHardwareFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveHardwareFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveHardwareFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readActiveRadioFaultsAttribute(): ActiveRadioFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveRadioFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveRadioFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readActiveNetworkFaultsAttribute(): ActiveNetworkFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActiveNetworkFaultsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActiveNetworkFaultsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTestEventTriggersEnabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeTestEventTriggersEnabledAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Boolean {
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
    const val CLUSTER_ID: UInt = 51u
  }
}
