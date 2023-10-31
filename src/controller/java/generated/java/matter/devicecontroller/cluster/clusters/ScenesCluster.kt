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
  class AddSceneResponse(val status: UShort, val groupID: UShort, val sceneID: UByte)

  class ViewSceneResponse(
    val status: UShort,
    val groupID: UShort,
    val sceneID: UByte,
    val transitionTime: UShort?,
    val sceneName: String?,
    val extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>?
  )

  class RemoveSceneResponse(val status: UShort, val groupID: UShort, val sceneID: UByte)

  class RemoveAllScenesResponse(val status: UShort, val groupID: UShort)

  class StoreSceneResponse(val status: UShort, val groupID: UShort, val sceneID: UByte)

  class GetSceneMembershipResponse(
    val status: UShort,
    val capacity: UByte?,
    val groupID: UShort,
    val sceneList: ArrayList<UByte>?
  )

  class EnhancedAddSceneResponse(val status: UShort, val groupID: UShort, val sceneID: UByte)

  class EnhancedViewSceneResponse(
    val status: UShort,
    val groupID: UShort,
    val sceneID: UByte,
    val transitionTime: UShort?,
    val sceneName: String?,
    val extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>?
  )

  class CopySceneResponse(
    val status: UShort,
    val groupIdentifierFrom: UShort,
    val sceneIdentifierFrom: UByte
  )

  class LastConfiguredByAttribute(val value: ULong?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun addScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int? = null
  ): AddSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun viewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): ViewSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun removeScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): RemoveSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun removeAllScenes(
    groupID: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): RemoveAllScenesResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun storeScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): StoreSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun recallScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort?,
    timedInvokeTimeoutMs: Int? = null
  ) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun getSceneMembership(
    groupID: UShort,
    timedInvokeTimeoutMs: Int? = null
  ): GetSceneMembershipResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun enhancedAddScene(
    groupID: UShort,
    sceneID: UByte,
    transitionTime: UShort,
    sceneName: String,
    extensionFieldSets: ArrayList<ChipStructs.ScenesClusterExtensionFieldSet>,
    timedInvokeTimeoutMs: Int? = null
  ): EnhancedAddSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun enhancedViewScene(
    groupID: UShort,
    sceneID: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): EnhancedViewSceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun copyScene(
    mode: UInt,
    groupIdentifierFrom: UShort,
    sceneIdentifierFrom: UByte,
    groupIdentifierTo: UShort,
    sceneIdentifierTo: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): CopySceneResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readSceneCountAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeSceneCountAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCurrentSceneAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentSceneAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readCurrentGroupAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeCurrentGroupAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readSceneValidAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun subscribeSceneValidAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readNameSupportAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeNameSupportAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readLastConfiguredByAttribute(): LastConfiguredByAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeLastConfiguredByAttribute(
    minInterval: Int,
    maxInterval: Int
  ): LastConfiguredByAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSceneTableSizeAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeSceneTableSizeAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRemainingCapacityAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun subscribeRemainingCapacityAttribute(minInterval: Int, maxInterval: Int): UByte {
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
    const val CLUSTER_ID: UInt = 5u
  }
}
