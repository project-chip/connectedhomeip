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

class ApplicationBasicCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class ApplicationAttribute(val value: ApplicationBasicClusterApplicationStruct)

  class AllowedVendorListAttribute(val value: List<UShort>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun readVendorNameAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeVendorNameAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readVendorIDAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeVendorIDAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readApplicationNameAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeApplicationNameAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readProductIDAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductIDAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readApplicationAttribute(): ApplicationAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeApplicationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ApplicationAttribute {
    // Implementation needs to be added here
  }

  suspend fun readStatusAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeStatusAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readApplicationVersionAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeApplicationVersionAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readAllowedVendorListAttribute(): AllowedVendorListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAllowedVendorListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AllowedVendorListAttribute {
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
    const val CLUSTER_ID: UInt = 1293u
  }
}
