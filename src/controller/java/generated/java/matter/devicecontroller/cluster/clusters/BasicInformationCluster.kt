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

import java.util.ArrayList

class BasicInformationCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 40u
  }

  fun mfgSpecificPing(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun mfgSpecificPing(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  interface CapabilityMinimaAttributeCallback {
    fun onSuccess(value: ChipStructs.BasicInformationClusterCapabilityMinimaStruct)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ProductAppearanceAttributeCallback {
    fun onSuccess(value: ChipStructs.BasicInformationClusterProductAppearanceStruct?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readDataModelRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeDataModelRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readVendorNameAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeVendorNameAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readVendorIDAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeVendorIDAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductNameAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductNameAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductIDAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductIDAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNodeLabelAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNodeLabelAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeNodeLabelAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNodeLabelAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocationAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLocationAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeLocationAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLocationAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readHardwareVersionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeHardwareVersionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readHardwareVersionStringAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeHardwareVersionStringAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSoftwareVersionAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSoftwareVersionAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSoftwareVersionStringAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSoftwareVersionStringAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readManufacturingDateAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeManufacturingDateAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readPartNumberAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribePartNumberAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductURLAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductURLAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductLabelAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductLabelAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSerialNumberAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSerialNumberAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLocalConfigDisabledAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLocalConfigDisabledAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeLocalConfigDisabledAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLocalConfigDisabledAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readReachableAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeReachableAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUniqueIDAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeUniqueIDAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCapabilityMinimaAttribute(callback: CapabilityMinimaAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCapabilityMinimaAttribute(
    callback: CapabilityMinimaAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readProductAppearanceAttribute(callback: ProductAppearanceAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeProductAppearanceAttribute(
    callback: ProductAppearanceAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
