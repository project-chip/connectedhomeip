/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

import chip.devicecontroller.model.NodeState
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.conflate
import kotlinx.coroutines.launch

data class LiveAttributeUpdate(
  val nodeId: Long,
  val endpointId: Int,
  val clusterId: Long,
  val attributeId: Long,
  val value: Any?,
  val isOptimistic: Boolean = false,
  val timestampMs: Long = System.currentTimeMillis(),
)

class MatterStateSynchronizer(
  private val nodeRegistry: CommissionedNodeRegistry,
  private val coroutineScope: CoroutineScope = CoroutineScope(Dispatchers.Default),
) {
  companion object {
    private const val TAG = "MatterStateSync"
    @Volatile private var instance: MatterStateSynchronizer? = null

    fun getInstance(nodeRegistry: CommissionedNodeRegistry): MatterStateSynchronizer {
      return instance
        ?: synchronized(this) {
          instance ?: MatterStateSynchronizer(nodeRegistry).also { instance = it }
        }
    }
  }

  private val _stateUpdates =
    MutableSharedFlow<LiveAttributeUpdate>(replay = 10, extraBufferCapacity = 50)
  val stateUpdates: SharedFlow<LiveAttributeUpdate> = _stateUpdates.asSharedFlow()

  private val _allStates = MutableStateFlow<Map<String, Any?>>(HomeStateCache.snapshotValues())
  val allStates: StateFlow<Map<String, Any?>> = _allStates.asStateFlow()

  init {
    // HomeStateCache is the single source of truth and is written before a change is emitted, so
    // the aggregate map is rebuilt from a fresh snapshot. Conflating means a burst of reports (for
    // example a priming report) costs one snapshot instead of one full-map copy per attribute, and
    // a
    // change dropped by the cache's DROP_OLDEST buffer is still picked up by the next snapshot.
    coroutineScope.launch {
      HomeStateCache.changes.conflate().collect {
        _allStates.value = HomeStateCache.snapshotValues()
      }
    }
    coroutineScope.launch {
      HomeStateCache.changes.collect { attr ->
        if (
          attr.clusterId == MatterClusterMetaRegistry.CLUSTER_ON_OFF && attr.attributeId == 0x0000L
        ) {
          nodeRegistry.updateNodeLiveState(attr.nodeId, "onOff", attr.value)
        } else if (
          attr.clusterId == MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL &&
            attr.attributeId == 0x0000L
        ) {
          nodeRegistry.updateNodeLiveState(attr.nodeId, "currentLevel", attr.value)
        }
        _stateUpdates.emit(
          LiveAttributeUpdate(
            nodeId = attr.nodeId,
            endpointId = attr.endpointId,
            clusterId = attr.clusterId,
            attributeId = attr.attributeId,
            value = attr.value,
            isOptimistic = false,
          )
        )
      }
    }
  }

  fun buildAttributeKey(nodeId: Long, endpointId: Int, clusterId: Long, attributeId: Long): String {
    return "$nodeId:$endpointId:$clusterId:$attributeId"
  }

  fun getCachedAttributeValue(
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
  ): Any? {
    return HomeStateCache.get(nodeId, endpointId, clusterId, attributeId)?.value
  }

  fun getAllCachedStates(): Map<String, Any?> = HomeStateCache.snapshotValues()

  fun clearCache() {
    _allStates.value = emptyMap()
  }

  fun applyOptimisticUpdate(
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    optimisticValue: Any?,
  ): () -> Unit {
    val previousValue = getCachedAttributeValue(nodeId, endpointId, clusterId, attributeId)
    HomeStateCache.put(
      nodeId,
      endpointId,
      clusterId,
      attributeId,
      optimisticValue,
      System.currentTimeMillis(),
      Source.UNICAST_READ,
    )
    coroutineScope.launch {
      _stateUpdates.emit(
        LiveAttributeUpdate(nodeId, endpointId, clusterId, attributeId, optimisticValue, true)
      )
    }
    return {
      HomeStateCache.put(
        nodeId,
        endpointId,
        clusterId,
        attributeId,
        previousValue,
        System.currentTimeMillis(),
        Source.UNICAST_READ,
      )
      coroutineScope.launch {
        _stateUpdates.emit(
          LiveAttributeUpdate(nodeId, endpointId, clusterId, attributeId, previousValue, false)
        )
      }
    }
  }

  fun recordConfirmedAttribute(
    nodeId: Long,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    confirmedValue: Any?,
  ) {
    HomeStateCache.put(
      nodeId,
      endpointId,
      clusterId,
      attributeId,
      confirmedValue,
      System.currentTimeMillis(),
      Source.SUBSCRIPTION_REPORT,
    )
    if (clusterId == MatterClusterMetaRegistry.CLUSTER_ON_OFF && attributeId == 0x0000L) {
      nodeRegistry.updateNodeLiveState(nodeId, "onOff", confirmedValue)
    } else if (
      clusterId == MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL && attributeId == 0x0000L
    ) {
      nodeRegistry.updateNodeLiveState(nodeId, "currentLevel", confirmedValue)
    }
  }

  fun processNodeStateReport(nodeId: Long, nodeState: NodeState) {
    for ((epId, endpointState) in nodeState.endpointStates) {
      for ((clusterId, clusterState) in endpointState.clusterStates) {
        for ((attrId, attributeState) in clusterState.attributeStates) {
          recordConfirmedAttribute(nodeId, epId, clusterId, attrId, attributeState.value)
        }
      }
    }
  }
}
