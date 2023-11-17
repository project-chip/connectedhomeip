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

class EthernetNetworkDiagnosticsCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class PHYRateAttribute(val value: UInt?)

  class FullDuplexAttribute(val value: Boolean?)

  class CarrierDetectAttribute(val value: Boolean?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun resetCounts(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readPHYRateAttribute(): PHYRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribePHYRateAttribute(minInterval: Int, maxInterval: Int): PHYRateAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFullDuplexAttribute(): FullDuplexAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeFullDuplexAttribute(
    minInterval: Int,
    maxInterval: Int
  ): FullDuplexAttribute {
    // Implementation needs to be added here
  }

  suspend fun readPacketRxCountAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketRxCountAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readPacketTxCountAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribePacketTxCountAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readTxErrCountAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribeTxErrCountAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readCollisionCountAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribeCollisionCountAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readOverrunCountAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribeOverrunCountAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readCarrierDetectAttribute(): CarrierDetectAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCarrierDetectAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CarrierDetectAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTimeSinceResetAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun subscribeTimeSinceResetAttribute(minInterval: Int, maxInterval: Int): ULong {
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
    const val CLUSTER_ID: UInt = 55u
  }
}
