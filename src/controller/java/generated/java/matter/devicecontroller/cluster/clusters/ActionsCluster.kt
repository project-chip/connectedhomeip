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

class ActionsCluster(private val endpointId: UShort) {
  companion object {
    const val CLUSTER_ID: UInt = 37u
  }

  fun instantAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun instantAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun instantActionWithTransition(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    transitionTime: Integer
  ) {
    // Implementation needs to be added here
  }

  fun instantActionWithTransition(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    transitionTime: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun startAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun startAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun startActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long
  ) {
    // Implementation needs to be added here
  }

  fun startActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun stopAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun stopAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun pauseAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun pauseAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun pauseActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long
  ) {
    // Implementation needs to be added here
  }

  fun pauseActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun resumeAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun resumeAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enableAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun enableAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun enableActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long
  ) {
    // Implementation needs to be added here
  }

  fun enableActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun disableAction(callback: DefaultClusterCallback, actionID: Integer, invokeID: Long?) {
    // Implementation needs to be added here
  }

  fun disableAction(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun disableActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long
  ) {
    // Implementation needs to be added here
  }

  fun disableActionWithDuration(
    callback: DefaultClusterCallback,
    actionID: Integer,
    invokeID: Long?,
    duration: Long,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface ActionListAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.ActionsClusterActionStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EndpointListsAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.ActionsClusterEndpointListStruct>)

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

  fun readActionListAttribute(callback: ActionListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeActionListAttribute(
    callback: ActionListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEndpointListsAttribute(callback: EndpointListsAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEndpointListsAttribute(
    callback: EndpointListsAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readSetupURLAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeSetupURLAttribute(
    callback: CharStringAttributeCallback,
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
