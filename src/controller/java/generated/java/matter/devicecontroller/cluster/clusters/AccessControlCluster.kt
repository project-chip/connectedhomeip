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

class AccessControlCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 31u
  }

  interface AclAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.AccessControlClusterAccessControlEntryStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ExtensionAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.AccessControlClusterAccessControlExtensionStruct>?)

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

  fun readAclAttribute(callback: AclAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readAclAttributeWithFabricFilter(callback: AclAttributeCallback, isFabricFiltered: Boolean) {
    // Implementation needs to be added here
  }

  fun writeAclAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.AccessControlClusterAccessControlEntryStruct>
  ) {
    // Implementation needs to be added here
  }

  fun writeAclAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.AccessControlClusterAccessControlEntryStruct>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeAclAttribute(callback: AclAttributeCallback, minInterval: Int, maxInterval: Int) {
    // Implementation needs to be added here
  }

  fun readExtensionAttribute(callback: ExtensionAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readExtensionAttributeWithFabricFilter(
    callback: ExtensionAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun writeExtensionAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.AccessControlClusterAccessControlExtensionStruct>
  ) {
    // Implementation needs to be added here
  }

  fun writeExtensionAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.AccessControlClusterAccessControlExtensionStruct>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeExtensionAttribute(
    callback: ExtensionAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSubjectsPerAccessControlEntryAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSubjectsPerAccessControlEntryAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTargetsPerAccessControlEntryAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeTargetsPerAccessControlEntryAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAccessControlEntriesPerFabricAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAccessControlEntriesPerFabricAttribute(
    callback: IntegerAttributeCallback,
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
