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

class GroupKeyManagementCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class KeySetReadResponse(val groupKeySet: GroupKeyManagementClusterGroupKeySetStruct)

  class KeySetReadAllIndicesResponse(val groupKeySetIDs: List<UShort>)

  class GroupKeyMapAttribute(val value: List<GroupKeyManagementClusterGroupKeyMapStruct>)

  class GroupTableAttribute(val value: List<GroupKeyManagementClusterGroupInfoMapStruct>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun keySetWrite(
    groupKeySet: GroupKeyManagementClusterGroupKeySetStruct,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun keySetRead(
    groupKeySetID: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): KeySetReadResponse {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun keySetRemove(groupKeySetID: UShort, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun keySetReadAllIndices(
    timedInvokeTimeoutMs: Int? = null
  ): KeySetReadAllIndicesResponse {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readGroupKeyMapAttribute(): GroupKeyMapAttribute {
    // Implementation needs to be added here
  }

  suspend fun readGroupKeyMapAttributeWithFabricFilter(
    isFabricFiltered: Boolean
  ): GroupKeyMapAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeGroupKeyMapAttribute(
    value: List<GroupKeyManagementClusterGroupKeyMapStruct>,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeGroupKeyMapAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GroupKeyMapAttribute {
    // Implementation needs to be added here
  }

  suspend fun readGroupTableAttribute(): GroupTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun readGroupTableAttributeWithFabricFilter(
    isFabricFiltered: Boolean
  ): GroupTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGroupTableAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GroupTableAttribute {
    // Implementation needs to be added here
  }

  suspend fun readMaxGroupsPerFabricAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxGroupsPerFabricAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readMaxGroupKeysPerFabricAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeMaxGroupKeysPerFabricAttribute(minInterval: Int, maxInterval: Int): UShort {
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
    const val CLUSTER_ID: UInt = 63u
  }
}
