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

class AccessControlCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class AclAttribute(val value: List<AccessControlClusterAccessControlEntryStruct>)

  class ExtensionAttribute(val value: List<AccessControlClusterAccessControlExtensionStruct>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readAclAttribute(): AclAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAclAttributeWithFabricFilter(isFabricFiltered: Boolean): AclAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeAclAttribute(
    value: List<AccessControlClusterAccessControlEntryStruct>,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeAclAttribute(minInterval: Int, maxInterval: Int): AclAttribute {
    // Implementation needs to be added here
  }

  suspend fun readExtensionAttribute(): ExtensionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readExtensionAttributeWithFabricFilter(
    isFabricFiltered: Boolean
  ): ExtensionAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeExtensionAttribute(
    value: List<AccessControlClusterAccessControlExtensionStruct>,
    timedWriteTimeoutMs: Int? = null
  ) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeExtensionAttribute(minInterval: Int, maxInterval: Int): ExtensionAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSubjectsPerAccessControlEntryAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeSubjectsPerAccessControlEntryAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readTargetsPerAccessControlEntryAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeTargetsPerAccessControlEntryAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
    // Implementation needs to be added here
  }

  suspend fun readAccessControlEntriesPerFabricAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeAccessControlEntriesPerFabricAttribute(
    minInterval: Int,
    maxInterval: Int
  ): UShort {
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
    const val CLUSTER_ID: UInt = 31u
  }
}
