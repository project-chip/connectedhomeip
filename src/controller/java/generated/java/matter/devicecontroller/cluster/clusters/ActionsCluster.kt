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
  class ActionListAttribute(val value: ArrayList<ChipStructs.ActionsClusterActionStruct>)

  class EndpointListsAttribute(val value: ArrayList<ChipStructs.ActionsClusterEndpointListStruct>)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun instantAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun instantAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun instantActionWithTransition(
    actionID: UShort,
    invokeID: UInt?,
    transitionTime: UShort
  ) {
    // Implementation needs to be added here
  }

  suspend fun instantActionWithTransition(
    actionID: UShort,
    invokeID: UInt?,
    transitionTime: UShort,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun startAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun startAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun startActionWithDuration(actionID: UShort, invokeID: UInt?, duration: UInt) {
    // Implementation needs to be added here
  }

  suspend fun startActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun stopAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun stopAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun pauseAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun pauseAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun pauseActionWithDuration(actionID: UShort, invokeID: UInt?, duration: UInt) {
    // Implementation needs to be added here
  }

  suspend fun pauseActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun resumeAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun resumeAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun enableAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun enableAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun enableActionWithDuration(actionID: UShort, invokeID: UInt?, duration: UInt) {
    // Implementation needs to be added here
  }

  suspend fun enableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun disableAction(actionID: UShort, invokeID: UInt?) {
    // Implementation needs to be added here
  }

  suspend fun disableAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun disableActionWithDuration(actionID: UShort, invokeID: UInt?, duration: UInt) {
    // Implementation needs to be added here
  }

  suspend fun disableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun readActionListAttribute(): ActionListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeActionListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ActionListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEndpointListsAttribute(): EndpointListsAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEndpointListsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): EndpointListsAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSetupURLAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun subscribeSetupURLAttribute(minInterval: Int, maxInterval: Int): CharString {
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

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 37u
  }
}
