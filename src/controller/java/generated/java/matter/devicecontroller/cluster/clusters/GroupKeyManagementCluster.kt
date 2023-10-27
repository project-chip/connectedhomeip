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

class GroupKeyManagementCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 63u
  }

  fun keySetWrite(
    callback: DefaultClusterCallback,
    groupKeySet: ChipStructs.GroupKeyManagementClusterGroupKeySetStruct
  ) {
    // Implementation needs to be added here
  }

  fun keySetWrite(
    callback: DefaultClusterCallback,
    groupKeySet: ChipStructs.GroupKeyManagementClusterGroupKeySetStruct,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun keySetRead(callback: KeySetReadResponseCallback, groupKeySetID: Integer) {
    // Implementation needs to be added here
  }

  fun keySetRead(
    callback: KeySetReadResponseCallback,
    groupKeySetID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun keySetRemove(callback: DefaultClusterCallback, groupKeySetID: Integer) {
    // Implementation needs to be added here
  }

  fun keySetRemove(
    callback: DefaultClusterCallback,
    groupKeySetID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun keySetReadAllIndices(callback: KeySetReadAllIndicesResponseCallback) {
    // Implementation needs to be added here
  }

  fun keySetReadAllIndices(
    callback: KeySetReadAllIndicesResponseCallback,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface KeySetReadResponseCallback {
    fun onSuccess(groupKeySet: ChipStructs.GroupKeyManagementClusterGroupKeySetStruct)

    fun onError(error: Exception)
  }

  interface KeySetReadAllIndicesResponseCallback {
    fun onSuccess(groupKeySetIDs: ArrayList<Integer>)

    fun onError(error: Exception)
  }

  interface GroupKeyMapAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.GroupKeyManagementClusterGroupKeyMapStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GroupTableAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.GroupKeyManagementClusterGroupInfoMapStruct>)

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

  fun readGroupKeyMapAttribute(callback: GroupKeyMapAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readGroupKeyMapAttributeWithFabricFilter(
    callback: GroupKeyMapAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun writeGroupKeyMapAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.GroupKeyManagementClusterGroupKeyMapStruct>
  ) {
    // Implementation needs to be added here
  }

  fun writeGroupKeyMapAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.GroupKeyManagementClusterGroupKeyMapStruct>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeGroupKeyMapAttribute(
    callback: GroupKeyMapAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGroupTableAttribute(callback: GroupTableAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readGroupTableAttributeWithFabricFilter(
    callback: GroupTableAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun subscribeGroupTableAttribute(
    callback: GroupTableAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxGroupsPerFabricAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxGroupsPerFabricAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readMaxGroupKeysPerFabricAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeMaxGroupKeysPerFabricAttribute(
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
