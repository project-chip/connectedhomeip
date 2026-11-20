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
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch

/**
 * ViewModel managing the lifecycle, live telemetry aggregation, quick controls, and metadata
 * synchronization for an individual device card.
 */
class DeviceCardViewModel(
  val initialNode: CommissionedNode,
  private val nodeRegistry: CommissionedNodeRegistry,
  private val stateSynchronizer: MatterStateSynchronizer,
  private val dispatcher: UniversalMatterDispatcher,
  private val coroutineScope: CoroutineScope = CoroutineScope(Dispatchers.Default),
) : ViewModel() {

  private val _cardState =
    MutableStateFlow(
      RoomDeviceHierarchyMapper.toDeviceCardState(
        initialNode,
        stateSynchronizer.getAllCachedStates(),
      )
    )
  val cardState: StateFlow<DeviceCardState> = _cardState.asStateFlow()

  init {
    // Listen to real-time IM attribute updates for this specific node
    coroutineScope.launch {
      stateSynchronizer.stateUpdates.collectLatest { update ->
        if (update.nodeId == _cardState.value.nodeId) {
          refreshCardState()
        }
      }
    }
  }

  fun refreshCardState() {
    val node = nodeRegistry.getNode(_cardState.value.nodeId) ?: return
    val updated =
      RoomDeviceHierarchyMapper.toDeviceCardState(node, stateSynchronizer.getAllCachedStates())
    _cardState.value = updated
  }

  /** Toggle On/Off state with optimistic UI update and IM dispatch. */
  fun togglePower(context: Context) {
    val current = _cardState.value
    val primaryEp = current.primaryEndpointId
    val currentAction = current.quickControl
    val currentIsOn =
      when (currentAction) {
        is QuickControlAction.ToggleSwitch -> currentAction.isOn
        is QuickControlAction.BrightnessSlider -> currentAction.isOn ?: false
        else -> false
      }
    val targetIsOn = !currentIsOn

    // 1. Optimistic update
    val rollback =
      stateSynchronizer.applyOptimisticUpdate(
        nodeId = current.nodeId,
        endpointId = primaryEp,
        clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
        attributeId = 0x0000L,
        optimisticValue = targetIsOn,
      )

    // 2. Dispatch IM Command
    coroutineScope.launch {
      val cmdId = if (targetIsOn) 0x01L else 0x00L // 1=On, 0=Off
      val result =
        dispatcher.invokeCommand(
          context = context,
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
          commandId = cmdId,
          targetDescription = "${current.nodeLabel} (${current.roomName})",
        )

      if (!result.isSuccess) {
        rollback()
      } else {
        stateSynchronizer.recordConfirmedAttribute(
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
          attributeId = 0x0000L,
          confirmedValue = targetIsOn,
        )
      }
      refreshCardState()
    }
  }

  /** Set Brightness Level ($0\dots 100\%$) with optimistic UI update and IM dispatch. */
  fun setBrightness(context: Context, percentage: Int) {
    val current = _cardState.value
    val primaryEp = current.primaryEndpointId
    val clampedPercent = percentage.coerceIn(0, 100)
    val rawLevel = (clampedPercent * 254) / 100

    val rollback =
      stateSynchronizer.applyOptimisticUpdate(
        nodeId = current.nodeId,
        endpointId = primaryEp,
        clusterId = MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL,
        attributeId = 0x0000L,
        optimisticValue = rawLevel,
      )

    coroutineScope.launch {
      val result =
        dispatcher.invokeCommand(
          context = context,
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL,
          commandId = 0x04L, // MoveToLevelWithOnOff
          params = mapOf("level" to rawLevel, "transitionTime" to 0),
          targetDescription = "${current.nodeLabel} (${current.roomName})",
        )

      if (!result.isSuccess) {
        rollback()
      } else {
        stateSynchronizer.recordConfirmedAttribute(
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL,
          attributeId = 0x0000L,
          confirmedValue = rawLevel,
        )
      }
      refreshCardState()
    }
  }

  /** Toggle Door Lock (Lock/Unlock) with optimistic UI update and IM dispatch. */
  fun toggleLock(context: Context) {
    val current = _cardState.value
    val primaryEp = current.primaryEndpointId
    val isLocked = (current.quickControl as? QuickControlAction.LockToggle)?.isLocked ?: false
    val targetLocked = !isLocked

    val rollback =
      stateSynchronizer.applyOptimisticUpdate(
        nodeId = current.nodeId,
        endpointId = primaryEp,
        clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
        attributeId = 0x0000L,
        optimisticValue = if (targetLocked) 1 else 2, // 1=Locked, 2=Unlocked
      )

    coroutineScope.launch {
      val cmdId = if (targetLocked) 0x00L else 0x01L // 0=LockDoor, 1=UnlockDoor
      val result =
        dispatcher.invokeCommand(
          context = context,
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
          commandId = cmdId,
          targetDescription = "${current.nodeLabel} (${current.roomName})",
        )

      if (!result.isSuccess) {
        rollback()
      } else {
        stateSynchronizer.recordConfirmedAttribute(
          nodeId = current.nodeId,
          endpointId = primaryEp,
          clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
          attributeId = 0x0000L,
          confirmedValue = if (targetLocked) 1 else 2,
        )
      }
      refreshCardState()
    }
  }

  /**
   * Updates the node's assigned room and synchronizes immediately with CommissionedNodeRegistry.
   */
  fun updateRoom(newRoomName: String) {
    val updated = nodeRegistry.updateNodeRoom(_cardState.value.nodeId, newRoomName)
    if (updated != null) {
      refreshCardState()
    }
  }

  /** Updates the node's semantic label and synchronizes with CommissionedNodeRegistry. */
  fun updateLabel(newLabel: String) {
    val updated = nodeRegistry.updateNodeLabel(_cardState.value.nodeId, newLabel)
    if (updated != null) {
      refreshCardState()
    }
  }

  /** Updates the node's aliases and synchronizes with CommissionedNodeRegistry. */
  fun updateAliases(aliases: List<String>) {
    val updated = nodeRegistry.updateNodeAliases(_cardState.value.nodeId, aliases)
    if (updated != null) {
      refreshCardState()
    }
  }

  public override fun onCleared() {
    super.onCleared()
    coroutineScope.cancel()
  }
}
