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

import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.first
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class PostCommissioningSetupTest {

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var synchronizer: MatterStateSynchronizer
  private lateinit var dispatcher: UniversalMatterDispatcher
  private lateinit var viewModel: DeviceManagementViewModel

  @Before
  fun setUp() {
    registry = CommissionedNodeRegistry()
    synchronizer = MatterStateSynchronizer(registry)
    dispatcher = UniversalMatterDispatcher(registry)
    viewModel =
      DeviceManagementViewModel(
        nodeRegistry = registry,
        stateSynchronizer = synchronizer,
        dispatcher = dispatcher,
        coroutineScope = CoroutineScope(Dispatchers.Default),
      )
  }

  @Test
  fun testAutoRegisterNewlyCommissionedNode() {
    val newNodeId = 0x9001L
    val deviceInfo =
      CHIPDeviceInfo(
        version = 0,
        vendorId = 0xFFF1,
        productId = 0x8001,
        discriminator = 3840,
        setupPinCode = 20202021L,
      )

    val registered = PostCommissioningDialogHelper.autoRegisterNode(registry, newNodeId, deviceInfo)
    assertNotNull(registered)
    assertEquals(newNodeId, registered.nodeId)
    assertEquals("Unassigned", registered.roomName)
    assertTrue(
      registered.nodeLabel.contains("Matter Device") || registered.nodeLabel.contains("9001")
    )

    val retrieved = registry.getNode(newNodeId)
    assertNotNull(retrieved)
    assertEquals(newNodeId, retrieved?.nodeId)
  }

  @Test
  fun testUpdateNodeMetadataAndAliases() {
    val newNodeId = 0x9002L
    registry.autoRegisterCommissionedNode(
      nodeId = newNodeId,
      defaultLabel = "Test Light",
      productName = "Smart LED",
      roomName = "Unassigned",
    )

    val updated =
      registry.updateNodeMetadata(
        nodeId = newNodeId,
        nodeLabel = "Kitchen Island Pendant",
        roomName = "Kitchen",
        aliases = listOf("island light", "counter spot", "cooking light"),
      )

    assertNotNull(updated)
    assertEquals("Kitchen Island Pendant", updated?.nodeLabel)
    assertEquals("Kitchen", updated?.roomName)
    assertEquals(3, updated?.aliases?.size)
    assertTrue(updated?.aliases?.contains("island light") == true)

    // Test query resolution
    val queryMatches = registry.findNodesByQuery("counter spot")
    assertEquals(1, queryMatches.size)
    assertEquals(newNodeId, queryMatches.first().nodeId)
  }

  @Test
  fun testUnassignedRoomGroupingPinnedToTop() {
    registry.clear()

    // Add nodes in various rooms
    registry.registerNode(
      CommissionedNode(nodeId = 0x1001L, nodeLabel = "Living Room Light", roomName = "Living Room")
    )
    registry.registerNode(
      CommissionedNode(nodeId = 0x1002L, nodeLabel = "Kitchen Light", roomName = "Kitchen")
    )
    registry.registerNode(
      CommissionedNode(nodeId = 0x1003L, nodeLabel = "Bedroom Fan", roomName = "Bedroom")
    )
    // Add unassigned node
    registry.registerNode(
      CommissionedNode(
        nodeId = 0x9003L,
        nodeLabel = "Newly Commissioned Plug",
        roomName = "Unassigned",
      )
    )

    viewModel.refreshFabric()

    val hierarchy =
      kotlinx.coroutines.runBlocking {
        kotlinx.coroutines.withTimeout(5_000) {
          viewModel.filteredRoomHierarchy.first { it.size == 4 }
        }
      }
    assertEquals(4, hierarchy.size)

    // Verify "Unassigned" is sorted to the very TOP
    assertEquals("Unassigned", hierarchy.first().roomName)
    assertEquals(1, hierarchy.first().deviceCards.size)
    assertEquals(0x9003L, hierarchy.first().deviceCards.first().nodeId)
  }

  @Test
  fun testUnassignedStatusBadgeGeneration() {
    val unassignedNode =
      CommissionedNode(nodeId = 0x9004L, nodeLabel = "New Sensor", roomName = "Unassigned")

    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(unassignedNode)
    assertEquals("Unassigned", cardState.roomName)

    val unassignedBadge = cardState.badges.firstOrNull { it.type == DeviceBadgeType.UNASSIGNED }
    assertNotNull(unassignedBadge)
    assertEquals("Unassigned", unassignedBadge?.label)

    // Assigned node should NOT have UNASSIGNED badge
    val assignedNode = unassignedNode.copy(roomName = "Living Room")
    val assignedCardState = RoomDeviceHierarchyMapper.toDeviceCardState(assignedNode)
    val noBadge = assignedCardState.badges.firstOrNull { it.type == DeviceBadgeType.UNASSIGNED }
    assertEquals(null, noBadge)
  }

  @Test
  fun testBasicInformationClusterMetadataRegistered() {
    val basicInfo =
      MatterClusterMetaRegistry.getCluster(MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION)
    assertNotNull(basicInfo)
    assertEquals("BasicInformation", basicInfo?.name)

    val nodeLabelAttr = basicInfo?.attributes?.get(0x0005L)
    assertNotNull(nodeLabelAttr)
    assertEquals("NodeLabel", nodeLabelAttr?.name)
    assertTrue(nodeLabelAttr?.isWritable == true)
  }
}
