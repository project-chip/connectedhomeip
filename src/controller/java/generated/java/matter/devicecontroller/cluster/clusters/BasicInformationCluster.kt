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

class BasicInformationCluster(
  private val controller: MatterController,
  private val endpointId: UShort
) {
  class CapabilityMinimaAttribute(val value: BasicInformationClusterCapabilityMinimaStruct)

  class ProductAppearanceAttribute(val value: BasicInformationClusterProductAppearanceStruct?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun mfgSpecificPing(timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readDataModelRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeDataModelRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

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

  suspend fun readProductNameAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductNameAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readProductIDAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductIDAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readNodeLabelAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun writeNodeLabelAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeNodeLabelAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readLocationAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun writeLocationAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeLocationAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readHardwareVersionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeHardwareVersionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readHardwareVersionStringAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeHardwareVersionStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
    // Implementation needs to be added here
  }

  suspend fun readSoftwareVersionAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeSoftwareVersionAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readSoftwareVersionStringAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeSoftwareVersionStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CharString {
    // Implementation needs to be added here
  }

  suspend fun readManufacturingDateAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeManufacturingDateAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readPartNumberAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribePartNumberAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readProductURLAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductURLAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readProductLabelAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductLabelAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readSerialNumberAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeSerialNumberAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readLocalConfigDisabledAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeLocalConfigDisabledAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    if (timedWriteTimeoutMs != null) {
      // Do the action with timedWriteTimeoutMs
    } else {
      // Do the action without timedWriteTimeoutMs
    }
  }

  suspend fun subscribeLocalConfigDisabledAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readReachableAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeReachableAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readUniqueIDAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeUniqueIDAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readCapabilityMinimaAttribute(): CapabilityMinimaAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeCapabilityMinimaAttribute(
    minInterval: Int,
    maxInterval: Int
  ): CapabilityMinimaAttribute {
    // Implementation needs to be added here
  }

  suspend fun readProductAppearanceAttribute(): ProductAppearanceAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeProductAppearanceAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ProductAppearanceAttribute {
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
    const val CLUSTER_ID: UInt = 40u
  }
}
