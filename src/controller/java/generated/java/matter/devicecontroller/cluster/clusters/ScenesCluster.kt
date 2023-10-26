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

class ScenesCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 5u
  }

  fun addScene(
    callback: AddSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>
  ) {
    // Implementation needs to be added here
  }

  fun addScene(
    callback: AddSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun viewScene(callback: ViewSceneResponseCallback, groupID: Integer, sceneID: Integer) {
    // Implementation needs to be added here
  }

  fun viewScene(
    callback: ViewSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun removeScene(callback: RemoveSceneResponseCallback, groupID: Integer, sceneID: Integer) {
    // Implementation needs to be added here
  }

  fun removeScene(
    callback: RemoveSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun removeAllScenes(callback: RemoveAllScenesResponseCallback, groupID: Integer) {
    // Implementation needs to be added here
  }

  fun removeAllScenes(
    callback: RemoveAllScenesResponseCallback,
    groupID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun storeScene(callback: StoreSceneResponseCallback, groupID: Integer, sceneID: Integer) {
    // Implementation needs to be added here
  }

  fun storeScene(
    callback: StoreSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun recallScene(
    callback: DefaultClusterCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer?
  ) {
    // Implementation needs to be added here
  }

  fun recallScene(
    callback: DefaultClusterCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun getSceneMembership(callback: GetSceneMembershipResponseCallback, groupID: Integer) {
    // Implementation needs to be added here
  }

  fun getSceneMembership(
    callback: GetSceneMembershipResponseCallback,
    groupID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedAddScene(
    callback: EnhancedAddSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>
  ) {
    // Implementation needs to be added here
  }

  fun enhancedAddScene(
    callback: EnhancedAddSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    transitionTime: Integer,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enhancedViewScene(
    callback: EnhancedViewSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer
  ) {
    // Implementation needs to be added here
  }

  fun enhancedViewScene(
    callback: EnhancedViewSceneResponseCallback,
    groupID: Integer,
    sceneID: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun copyScene(
    callback: CopySceneResponseCallback,
    mode: Integer,
    groupIdentifierFrom: Integer,
    sceneIdentifierFrom: Integer,
    groupIdentifierTo: Integer,
    sceneIdentifierTo: Integer
  ) {
    // Implementation needs to be added here
  }

  fun copyScene(
    callback: CopySceneResponseCallback,
    mode: Integer,
    groupIdentifierFrom: Integer,
    sceneIdentifierFrom: Integer,
    groupIdentifierTo: Integer,
    sceneIdentifierTo: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface AddSceneResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer, sceneID: Integer)

    fun onError(error: Exception)
  }

  interface ViewSceneResponseCallback {
    fun onSuccess(
      status: Integer,
      groupID: Integer,
      sceneID: Integer,
      transitionTime: Integer?,
      sceneName: String?,
      extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>?
    )

    fun onError(error: Exception)
  }

  interface RemoveSceneResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer, sceneID: Integer)

    fun onError(error: Exception)
  }

  interface RemoveAllScenesResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer)

    fun onError(error: Exception)
  }

  interface StoreSceneResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer, sceneID: Integer)

    fun onError(error: Exception)
  }

  interface GetSceneMembershipResponseCallback {
    fun onSuccess(
      status: Integer,
      capacity: Integer?,
      groupID: Integer,
      sceneList: ArrayList<Integer>?
    )

    fun onError(error: Exception)
  }

  interface EnhancedAddSceneResponseCallback {
    fun onSuccess(status: Integer, groupID: Integer, sceneID: Integer)

    fun onError(error: Exception)
  }

  interface EnhancedViewSceneResponseCallback {
    fun onSuccess(
      status: Integer,
      groupID: Integer,
      sceneID: Integer,
      transitionTime: Integer?,
      sceneName: String?,
      extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>?
    )

    fun onError(error: Exception)
  }

  interface CopySceneResponseCallback {
    fun onSuccess(status: Integer, groupIdentifierFrom: Integer, sceneIdentifierFrom: Integer)

    fun onError(error: Exception)
  }

  interface LastConfiguredByAttributeCallback {
    fun onSuccess(value: Long?)

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

  fun readSceneCountAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSceneCountAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentSceneAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentSceneAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCurrentGroupAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeCurrentGroupAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSceneValidAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSceneValidAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
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

  fun readLastConfiguredByAttribute(callback: LastConfiguredByAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeLastConfiguredByAttribute(
    callback: LastConfiguredByAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSceneTableSizeAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSceneTableSizeAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRemainingCapacityAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeRemainingCapacityAttribute(
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
