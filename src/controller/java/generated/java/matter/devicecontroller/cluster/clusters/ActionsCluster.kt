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

class ActionsCluster(private val controller: MatterController, private val endpointId: UShort) {
  class ActionListAttribute(val value: List<ActionsClusterActionStruct>)

  class EndpointListsAttribute(val value: List<ActionsClusterEndpointListStruct>)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun instantAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 0L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun instantActionWithTransition(
    actionID: UShort,
    invokeID: UInt?,
    transitionTime: UShort,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 1L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun startAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 2L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun startActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 3L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun stopAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 4L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun pauseAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 5L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun pauseActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 6L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun resumeAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 7L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun enableAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 8L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun enableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 9L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun disableAction(actionID: UShort, invokeID: UInt?, timedInvokeTimeoutMs: Int? = null) {
    val commandId = 10L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun disableActionWithDuration(
    actionID: UShort,
    invokeID: UInt?,
    duration: UInt,
    timedInvokeTimeoutMs: Int? = null
  ) {
    val commandId = 11L

    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
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
    const val CLUSTER_ID: UInt = 37u
  }
}
