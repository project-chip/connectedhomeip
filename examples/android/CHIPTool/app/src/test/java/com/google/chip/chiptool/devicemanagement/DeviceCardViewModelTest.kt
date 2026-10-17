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

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class DeviceCardViewModelTest {

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var synchronizer: MatterStateSynchronizer
  private lateinit var dispatcher: UniversalMatterDispatcher
  private lateinit var viewModel: DeviceCardViewModel

  @Before
  fun setUp() {
    registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
    synchronizer = MatterStateSynchronizer(registry)
    dispatcher = UniversalMatterDispatcher(registry)

    val lightNode = registry.getNode(0x1001L)!!
    viewModel = DeviceCardViewModel(lightNode, registry, synchronizer, dispatcher)
  }

  @Test
  fun testInitialCardState() {
    val state = viewModel.cardState.value
    assertEquals(0x1001L, state.nodeId)
    assertEquals("Living Room Ceiling Light", state.nodeLabel)
    assertEquals("Living Room", state.roomName)
    // No link state yet: the card must not claim the device is online.
    assertFalse(state.isOnline)
  }

  @Test
  fun testUpdateRoomSyncsWithRegistry() {
    viewModel.updateRoom("Master Bedroom")

    assertEquals("Master Bedroom", viewModel.cardState.value.roomName)
    val nodeInRegistry = registry.getNode(0x1001L)
    assertNotNull(nodeInRegistry)
    assertEquals("Master Bedroom", nodeInRegistry?.roomName)
  }

  @Test
  fun testUpdateAliasesSyncsWithRegistry() {
    val aliases = listOf("ceiling chandelier", "living lamp", "reading bulb")
    viewModel.updateAliases(aliases)

    assertEquals(aliases, viewModel.cardState.value.aliases)
    val nodeInRegistry = registry.getNode(0x1001L)
    assertEquals(aliases, nodeInRegistry?.aliases)

    // Verify finding by new alias
    val found = registry.findNodesByQuery("chandelier")
    assertTrue(found.any { it.nodeId == 0x1001L })
  }

  @Test
  fun testUpdateLabelSyncsWithRegistry() {
    viewModel.updateLabel("Chandelier Alpha")

    assertEquals("Chandelier Alpha", viewModel.cardState.value.nodeLabel)
    val nodeInRegistry = registry.getNode(0x1001L)
    assertEquals("Chandelier Alpha", nodeInRegistry?.nodeLabel)
  }

  @Test
  fun testOptimisticStateRefreshOnLiveUpdate() {
    synchronizer.recordConfirmedAttribute(
      nodeId = 0x1001L,
      endpointId = 1,
      clusterId = 6L, // OnOff
      attributeId = 0L,
      confirmedValue = true,
    )

    viewModel.refreshCardState()
    val state = viewModel.cardState.value
    val onOffBadge = state.badges.firstOrNull { it.type == DeviceBadgeType.STATE_ON }
    assertNotNull(onOffBadge)
    assertEquals("ON", onOffBadge?.label)
  }
}
