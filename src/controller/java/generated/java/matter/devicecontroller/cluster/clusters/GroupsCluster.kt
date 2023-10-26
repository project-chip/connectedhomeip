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

class GroupsCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 4u
  }

  fun addGroup(callback: AddGroupResponseCallback, groupID: Integer, groupName: String) {
    // Implementation needs to be added here
  }

  fun addGroup(
    callback: AddGroupResponseCallback,
    groupID: Integer,
    groupName: String,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun viewGroup(callback: ViewGroupResponseCallback, groupID: Integer) {
    // Implementation needs to be added here
  }

  fun viewGroup(callback: ViewGroupResponseCallback, groupID: Integer, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun getGroupMembership(
    callback: GetGroupMembershipResponseCallback,
    groupList: ArrayList<Integer>
  ) {
    // Implementation needs to be added here
  }

  fun getGroupMembership(
    callback: GetGroupMembershipResponseCallback,
    groupList: ArrayList<Integer>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun removeGroup(callback: RemoveGroupResponseCallback, groupID: Integer) {
    // Implementation needs to be added here
  }

  fun removeGroup(
    callback: RemoveGroupResponseCallback,
    groupID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun removeAllGroups(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun removeAllGroups(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun addGroupIfIdentifying(callback: DefaultClusterCallback, groupID: Integer, groupName: String) {
    // Implementation needs to be added here
  }

  fun addGroupIfIdentifying(
    callback: DefaultClusterCallback,
    groupID: Integer,
    groupName: String,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface AddGroupResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer)

    fun onError(error: Exception)
  }

  interface ViewGroupResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer, groupName: String)

    fun onError(error: Exception)
  }

  interface GetGroupMembershipResponseCallback {
    fun onSuccess(capacity: Integer?, groupList: ArrayList<Integer>)

    fun onError(error: Exception)
  }

  interface RemoveGroupResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer)

    fun onError(error: Exception)
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

  fun readNameSupportAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeNameSupportAttribute(
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
