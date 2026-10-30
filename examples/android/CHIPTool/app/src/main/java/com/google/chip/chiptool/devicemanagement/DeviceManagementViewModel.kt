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

import android.content.Context
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import java.util.concurrent.ConcurrentHashMap
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.combine
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

/** Bulk action options for all devices within a single room. */
enum class BulkRoomAction {
  TURN_ALL_ON,
  TURN_ALL_OFF,
  LOCK_ALL_DOORS,
}

/**
 * Root ViewModel coordinating the Material 3 Room & Device Dashboard. Groups commissioned nodes by
 * room, enables search and category filtering, handles room reassignments, and manages fabric-wide
 * subscriptions.
 */
class DeviceManagementViewModel(
  private val nodeRegistry: CommissionedNodeRegistry = CommissionedNodeRegistry(),
  private val stateSynchronizer: MatterStateSynchronizer = MatterStateSynchronizer(nodeRegistry),
  private val dispatcher: UniversalMatterDispatcher = UniversalMatterDispatcher(nodeRegistry),
  private val coroutineScope: CoroutineScope = CoroutineScope(Dispatchers.Default),
  val centerConnectionManager: CenterConnectionManager = CenterConnectionManager,
) : ViewModel() {

  class Factory(private val context: Context) : ViewModelProvider.Factory {
    @Suppress("UNCHECKED_CAST")
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
      val appContext = context.applicationContext ?: context
      val reg = CenterConnectionManager.registry(appContext)
      val sync = MatterStateSynchronizer.getInstance(reg)
      val disp = UniversalMatterDispatcher(reg)
      return DeviceManagementViewModel(
        nodeRegistry = reg,
        stateSynchronizer = sync,
        dispatcher = disp,
        centerConnectionManager = CenterConnectionManager,
      )
        as T
    }
  }

  private var registryListener: ((CommissionedNode) -> Unit)? = null
  private val cardViewModels = ConcurrentHashMap<Long, DeviceCardViewModel>()

  fun getOrCreateCardViewModel(node: CommissionedNode): DeviceCardViewModel {
    return cardViewModels.getOrPut(node.nodeId) {
      DeviceCardViewModel(node, nodeRegistry, stateSynchronizer, dispatcher)
    }
  }

  private val _allNodes = MutableStateFlow<List<CommissionedNode>>(nodeRegistry.getAllNodes())
  val allNodes: StateFlow<List<CommissionedNode>> = _allNodes.asStateFlow()

  private val _selectedRoomFilter = MutableStateFlow<String?>(null) // null = All Rooms
  val selectedRoomFilter: StateFlow<String?> = _selectedRoomFilter.asStateFlow()

  private val _searchQuery = MutableStateFlow("")
  val searchQuery: StateFlow<String> = _searchQuery.asStateFlow()

  val linkStates: StateFlow<Map<Long, LinkState>> = centerConnectionManager.linkStates
  val connectAllProgress: StateFlow<ConnectAllProgress?> =
    centerConnectionManager.connectAllProgress

  val summaryStripText: StateFlow<String> =
    combine(
        _allNodes,
        centerConnectionManager.linkStates,
        centerConnectionManager.connectAllProgress,
      ) { nodes, states, progress ->
        if (progress?.active == true) {
          val cur = minOf(progress.settled + 1, progress.total)
          return@combine "Connecting $cur of ${progress.total}…"
        }
        if (nodes.isEmpty()) {
          return@combine "No devices commissioned"
        }
        val live = nodes.count { states[it.nodeId]?.phase == LinkPhase.LIVE }
        val asleep = nodes.count { states[it.nodeId]?.phase == LinkPhase.ASLEEP }
        val offline =
          nodes.count {
            val p = states[it.nodeId]?.phase
            p == LinkPhase.OFFLINE || p == LinkPhase.RECOVERING
          }
        val parts = mutableListOf<String>()
        if (live > 0) parts.add("$live live")
        if (asleep > 0) parts.add("$asleep asleep")
        if (offline > 0) parts.add("$offline offline")

        if (parts.isEmpty()) {
          "${nodes.size} devices · not connected"
        } else {
          parts.joinToString(" · ")
        }
      }
      .stateIn(coroutineScope, SharingStarted.Eagerly, "")

  // Combined reactive stream of filtered room hierarchies
  val filteredRoomHierarchy: StateFlow<List<RoomHierarchyItem>> =
    combine(
        _allNodes,
        _selectedRoomFilter,
        _searchQuery,
        stateSynchronizer.allStates,
        centerConnectionManager.linkStates,
      ) { args: Array<Any?> ->
        @Suppress("UNCHECKED_CAST") val nodes = args[0] as List<CommissionedNode>
        val roomFilter = args[1] as String?
        val query = args[2] as String
        @Suppress("UNCHECKED_CAST") val liveStates = args[3] as Map<String, Any?>
        @Suppress("UNCHECKED_CAST") val states = args[4] as Map<Long, LinkState>

        val cleanQuery = query.trim().lowercase()

        // 1. Filter by query
        val matchingNodes =
          if (cleanQuery.isEmpty()) nodes else nodes.filter { matchesSearchQuery(it, cleanQuery) }

        // 2. Group by room
        val groupedByRoom = matchingNodes.groupBy { it.roomName.trim().ifEmpty { "Unassigned" } }

        // 3. Construct RoomHierarchyItems with "Unassigned" guaranteed at the top
        val roomItems =
          groupedByRoom
            .map { (roomName, roomNodes) ->
              val deviceCards =
                roomNodes.map { node ->
                  RoomDeviceHierarchyMapper.toDeviceCardState(
                    node = node,
                    liveAttributeValues = liveStates,
                    linkState = states[node.nodeId],
                  )
                }
              RoomHierarchyItem(roomName = roomName, deviceCards = deviceCards)
            }
            .sortedWith { a, b -> compareRoomNames(a.roomName, b.roomName) }

        // 4. Apply Room Filter if set
        if (roomFilter != null) {
          roomItems.filter { it.roomName.equals(roomFilter, ignoreCase = true) }
        } else {
          roomItems
        }
      }
      .stateIn(coroutineScope, SharingStarted.Eagerly, emptyList())

  val availableRooms: StateFlow<List<String>> =
    combine(_allNodes) { nodeRegistry.getAllRooms().sortedWith(::compareRoomNames) }
      .stateIn(coroutineScope, SharingStarted.Eagerly, emptyList())

  // Internal for unit tests.
  internal fun matchesSearchQuery(node: CommissionedNode, cleanQuery: String): Boolean {
    val label = node.nodeLabel.lowercase()
    val room = node.roomName.lowercase()
    // Reverse containment ("living room lamp" matches label "lamp") must ignore empty fields,
    // otherwise every query matches a node with an empty label or room.
    fun matches(field: String) =
      field.isNotEmpty() && (field.contains(cleanQuery) || cleanQuery.contains(field))
    return matches(label) ||
      matches(room) ||
      node.aliases.any { matches(it.lowercase()) } ||
      node.endpoints.any { it.deviceTypeName.lowercase().contains(cleanQuery) }
  }

  private fun compareRoomNames(a: String, b: String): Int {
    val aUnassigned = a.equals("Unassigned", ignoreCase = true)
    val bUnassigned = b.equals("Unassigned", ignoreCase = true)
    return when {
      aUnassigned && !bUnassigned -> -1
      !aUnassigned && bUnassigned -> 1
      else -> a.compareTo(b, ignoreCase = true)
    }
  }

  init {
    val listener: (CommissionedNode) -> Unit = { _allNodes.value = nodeRegistry.getAllNodes() }
    registryListener = listener
    nodeRegistry.addNodeRegistryChangeListener(listener)

    // Sync updates
    coroutineScope.launch {
      stateSynchronizer.stateUpdates.collectLatest { _allNodes.value = nodeRegistry.getAllNodes() }
    }
  }

  override fun onCleared() {
    super.onCleared()
    registryListener?.let { nodeRegistry.removeNodeRegistryChangeListener(it) }
    cardViewModels.values.forEach { it.onCleared() }
    cardViewModels.clear()
    coroutineScope.cancel()
  }

  fun connectAll(): Boolean = centerConnectionManager.connectAll()

  fun cancelConnectAll() = centerConnectionManager.cancelConnectAll()

  fun connectDevice(nodeId: Long): Boolean = centerConnectionManager.connect(nodeId)

  fun refreshFabric() {
    _allNodes.value = nodeRegistry.getAllNodes()
  }

  fun setRoomFilter(roomName: String?) {
    _selectedRoomFilter.value = roomName
  }

  fun setSearchQuery(query: String) {
    _searchQuery.value = query
  }

  fun updateDeviceMetadata(
    nodeId: Long,
    nodeLabel: String,
    newRoomName: String,
    aliases: List<String>,
  ) {
    nodeRegistry.updateNodeMetadata(nodeId, nodeLabel, newRoomName, aliases)
    _allNodes.value = nodeRegistry.getAllNodes()
  }

  fun reassignDeviceRoom(nodeId: Long, newRoomName: String) {
    nodeRegistry.updateNodeRoom(nodeId, newRoomName)
    _allNodes.value = nodeRegistry.getAllNodes()
  }

  fun updateDeviceLabel(nodeId: Long, newLabel: String) {
    nodeRegistry.updateNodeLabel(nodeId, newLabel)
    _allNodes.value = nodeRegistry.getAllNodes()
  }

  fun updateDeviceAliases(nodeId: Long, aliases: List<String>) {
    nodeRegistry.updateNodeAliases(nodeId, aliases)
    _allNodes.value = nodeRegistry.getAllNodes()
  }

  /** Dispatches bulk actions across all devices in the designated room. */
  fun executeBulkRoomAction(context: Context, roomName: String, action: BulkRoomAction) {
    val roomNodes = nodeRegistry.getNodesInRoom(roomName)

    viewModelScope.launch {
      for (node in roomNodes) {
        val primaryEp = node.endpoints.firstOrNull { it.endpointId != 0 } ?: continue

        when (action) {
          BulkRoomAction.TURN_ALL_ON -> {
            if (primaryEp.supportsCluster(MatterClusterMetaRegistry.CLUSTER_ON_OFF)) {
              val res =
                dispatcher.invokeCommand(
                  context = context,
                  nodeId = node.nodeId,
                  endpointId = primaryEp.endpointId,
                  clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                  commandId = 0x01L, // On
                  targetDescription = "${node.nodeLabel} ($roomName)",
                )
              if (res.isSuccess) {
                stateSynchronizer.recordConfirmedAttribute(
                  node.nodeId,
                  primaryEp.endpointId,
                  MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                  0x0000L,
                  true,
                )
              }
            }
          }
          BulkRoomAction.TURN_ALL_OFF -> {
            if (primaryEp.supportsCluster(MatterClusterMetaRegistry.CLUSTER_ON_OFF)) {
              val res =
                dispatcher.invokeCommand(
                  context = context,
                  nodeId = node.nodeId,
                  endpointId = primaryEp.endpointId,
                  clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                  commandId = 0x00L, // Off
                  targetDescription = "${node.nodeLabel} ($roomName)",
                )
              if (res.isSuccess) {
                stateSynchronizer.recordConfirmedAttribute(
                  node.nodeId,
                  primaryEp.endpointId,
                  MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                  0x0000L,
                  false,
                )
              }
            }
          }
          BulkRoomAction.LOCK_ALL_DOORS -> {
            if (primaryEp.supportsCluster(MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK)) {
              val res =
                dispatcher.invokeCommand(
                  context = context,
                  nodeId = node.nodeId,
                  endpointId = primaryEp.endpointId,
                  clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
                  commandId = 0x00L, // LockDoor
                  targetDescription = "${node.nodeLabel} ($roomName)",
                )
              if (res.isSuccess) {
                stateSynchronizer.recordConfirmedAttribute(
                  node.nodeId,
                  primaryEp.endpointId,
                  MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
                  0x0000L,
                  1,
                )
              }
            }
          }
        }
      }
      _allNodes.value = nodeRegistry.getAllNodes()
    }
  }
}
