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

import chip.devicecontroller.ChipStructs
import chip.devicecontroller.model.NodeState
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class CommissionedNodeRegistryTest {

  private fun addAttr(
    nodeState: NodeState,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    value: Any?,
    tlv: ByteArray = ByteArray(0),
  ) {
    val m =
      NodeState::class
        .java
        .getDeclaredMethod(
          "addAttribute",
          Int::class.javaPrimitiveType,
          Long::class.javaPrimitiveType,
          Long::class.javaPrimitiveType,
          Any::class.java,
          ByteArray::class.java,
          String::class.java,
        )
    m.isAccessible = true
    m.invoke(nodeState, endpointId, clusterId, attributeId, value, tlv, null)
  }

  @Test
  fun testRegistryStartsEmptyByDefault() {
    val registry = CommissionedNodeRegistry()
    assertEquals(0, registry.count())
    assertTrue(registry.getAllNodes().isEmpty())
  }

  @Test
  fun testClearAllNodesAndResetFabric() {
    val registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
    assertEquals(14, registry.count())

    registry.clearAllNodes()
    assertEquals(0, registry.count())
    assertTrue(registry.getAllNodes().isEmpty())

    registry.loadTestSmartHomeFabric()
    assertEquals(14, registry.count())

    registry.resetFabric()
    assertEquals(0, registry.count())
    assertTrue(registry.getAllNodes().isEmpty())
  }

  @Test
  fun testAutoRegisterSeedsOnlyRootNodeAndAwaitsDescriptor() {
    val registry = CommissionedNodeRegistry()
    registry.autoRegisterCommissionedNode(1L)
    val node = registry.getNode(1L)!!
    assertFalse(node.descriptorVerified)
    assertEquals(1, node.endpoints.size)
    assertEquals(0, node.endpoints.first().endpointId)
    assertFalse(node.getAllSupportedClusterIds().contains(0x0006L))
  }

  @Test
  fun testRecordDiscoveredEndpointClusterDoesNotInventDeviceTypes() {
    val registry = CommissionedNodeRegistry()
    registry.autoRegisterCommissionedNode(1L)
    registry.recordDiscoveredEndpointCluster(null, 1L, 0, 0x0028L, 0x0001L, "Aqara")
    registry.recordDiscoveredEndpointCluster(null, 1L, 0, 0x0028L, 0x0003L, "W100 Climate Sensor")
    registry.recordDiscoveredEndpointCluster(null, 1L, 2, 0x0402L, 0x0000L, 2345L)

    val node = registry.getNode(1L)!!
    assertEquals("Aqara", node.vendorName)
    assertEquals("W100 Climate Sensor", node.productName)
    val ep2 = node.endpoints.first { it.endpointId == 2 }
    assertEquals(0L, ep2.deviceTypeId)
    assertEquals("Device Type 0x0000", ep2.deviceTypeName)
    assertTrue(ep2.serverClusters.contains(0x0402L))
    assertFalse(ep2.serverClusters.contains(0x0006L))
  }

  @Test
  fun testApplyDescriptorReportReplacesEndpointsAndPreservesUserFieldsForSensorAndBulb() {
    val registry = CommissionedNodeRegistry()
    // Seed sensor node 1 with stale/polluted clusters (including OnOff 0x0006) and user-edited
    // metadata
    registry.registerNode(
      CommissionedNode(
        nodeId = 1L,
        nodeLabel = "Hallway Climate Monitor",
        roomName = "Hallway",
        vendorName = "Aqara",
        productName = "Aqara W100",
        aliases = listOf("Upstairs Sensor"),
        descriptorVerified = false,
        endpoints =
          listOf(
            CommissionedEndpoint(
              endpointId = 1,
              deviceTypeId = MatterDeviceTypes.ON_OFF_LIGHT,
              deviceTypeName = "On/Off Light",
              serverClusters = setOf(0x0006L, 0x0008L, 0x0402L),
            )
          ),
      )
    )
    // Seed bulb node 3 with minimal ep0 and user-edited metadata
    registry.registerNode(
      CommissionedNode(
        nodeId = 3L,
        nodeLabel = "Desk Lamp",
        roomName = "Study",
        vendorName = "Leedarson",
        productName = "Smart RGBTW Bulb",
        aliases = listOf("Reading Light"),
        descriptorVerified = false,
        endpoints =
          listOf(
            CommissionedEndpoint(
              endpointId = 0,
              deviceTypeId = 0x0016L,
              deviceTypeName = "Root Node",
              serverClusters = setOf(0x0028L),
            )
          ),
      )
    )

    // 1. Fake Descriptor (0x001D) report for Sensor (node 1): ep0 PartsList [1], ep1 ServerList
    // [0x0402, 0x0405, 0x0003], DeviceType 0x0302
    val sensorReport = NodeState()
    addAttr(sensorReport, 0, 0x001DL, 0x0003L, arrayListOf(1L))
    addAttr(sensorReport, 0, 0x001DL, 0x0001L, arrayListOf(0x001DL, 0x001FL, 0x0028L))
    addAttr(
      sensorReport,
      0,
      0x001DL,
      0x0000L,
      arrayListOf(ChipStructs.DescriptorClusterDeviceTypeStruct(0x0016L, 1)),
    )
    addAttr(sensorReport, 1, 0x001DL, 0x0001L, arrayListOf(0x0402L, 0x0405L, 0x0003L))
    addAttr(
      sensorReport,
      1,
      0x001DL,
      0x0000L,
      arrayListOf(ChipStructs.DescriptorClusterDeviceTypeStruct(0x0302L, 1)),
    )

    assertTrue(registry.applyDescriptorReport(null, 1L, sensorReport))
    val updatedSensor = registry.getNode(1L)!!
    assertTrue(updatedSensor.descriptorVerified)
    // User fields preserved
    assertEquals("Hallway Climate Monitor", updatedSensor.nodeLabel)
    assertEquals("Hallway", updatedSensor.roomName)
    assertEquals(listOf("Upstairs Sensor"), updatedSensor.aliases)
    // Replacement semantics: stale 0x0006 and 0x0008 are gone
    val sensorEp1 = updatedSensor.endpoints.first { it.endpointId == 1 }
    assertEquals(setOf(0x0402L, 0x0405L, 0x0003L), sensorEp1.serverClusters)
    assertEquals(0x0302L, sensorEp1.deviceTypeId)
    assertEquals("Temperature Sensor", sensorEp1.deviceTypeName)
    assertFalse(registry.isLightNode(updatedSensor))

    // 2. Fake Descriptor (0x001D) report for Bulb (node 3): ep0 PartsList [1], ep1 ServerList
    // [0x0006, 0x0008, 0x0300], DeviceType 0x010D
    val bulbReport = NodeState()
    addAttr(bulbReport, 0, 0x001DL, 0x0003L, arrayListOf(1L))
    addAttr(bulbReport, 0, 0x001DL, 0x0001L, arrayListOf(0x001DL, 0x0028L))
    addAttr(bulbReport, 1, 0x001DL, 0x0001L, arrayListOf(0x0006L, 0x0008L, 0x0300L))
    addAttr(
      bulbReport,
      1,
      0x001DL,
      0x0000L,
      arrayListOf(ChipStructs.DescriptorClusterDeviceTypeStruct(0x010DL, 2)),
    )

    assertTrue(registry.applyDescriptorReport(null, 3L, bulbReport))
    val updatedBulb = registry.getNode(3L)!!
    assertTrue(updatedBulb.descriptorVerified)
    assertEquals("Desk Lamp", updatedBulb.nodeLabel)
    assertEquals("Study", updatedBulb.roomName)
    assertEquals(listOf("Reading Light"), updatedBulb.aliases)
    val bulbEp1 = updatedBulb.endpoints.first { it.endpointId == 1 }
    assertEquals(setOf(0x0006L, 0x0008L, 0x0300L), bulbEp1.serverClusters)
    assertEquals(0x010DL, bulbEp1.deviceTypeId)
    assertEquals("Extended Color Light", bulbEp1.deviceTypeName)
    assertTrue(registry.isLightNode(updatedBulb))

    // Verify light resolution targets only the bulb (node 3), never the sensor (node 1)
    val lightTargets = registry.resolveTargets("light", 0x0006L)
    assertEquals(listOf(3L), lightTargets.map { it.first.nodeId })
  }

  @Test
  fun testApplyDescriptorReportFailureKeepsPriorData() {
    val registry = CommissionedNodeRegistry()
    val priorEndpoints =
      listOf(
        CommissionedEndpoint(
          endpointId = 1,
          deviceTypeId = 0x0302L,
          deviceTypeName = "Temperature Sensor",
          serverClusters = setOf(0x0402L, 0x0405L),
        )
      )
    registry.registerNode(
      CommissionedNode(
        nodeId = 2L,
        nodeLabel = "Bedroom W100",
        roomName = "Bedroom",
        aliases = listOf("Sleepy Sensor"),
        descriptorVerified = false,
        endpoints = priorEndpoints,
      )
    )

    // Empty report simulating ICD timeout / read failure
    val emptyReport = NodeState()
    assertFalse(registry.applyDescriptorReport(null, 2L, emptyReport))

    val nodeAfterFailure = registry.getNode(2L)!!
    assertFalse(nodeAfterFailure.descriptorVerified)
    assertEquals("Bedroom W100", nodeAfterFailure.nodeLabel)
    assertEquals("Bedroom", nodeAfterFailure.roomName)
    assertEquals(listOf("Sleepy Sensor"), nodeAfterFailure.aliases)
    assertEquals(priorEndpoints, nodeAfterFailure.endpoints)
  }

  @Test
  fun testRemoveUnsupportedClusterAndSyncWithDeviceIds() {
    val registry = CommissionedNodeRegistry()
    registry.registerNode(
      CommissionedNode(
        nodeId = 1L,
        nodeLabel = "Node 1",
        endpoints =
          listOf(
            CommissionedEndpoint(
              endpointId = 1,
              deviceTypeId = 0x0302L,
              deviceTypeName = "Temperature Sensor",
              serverClusters = setOf(0x0006L, 0x0402L),
            )
          ),
      )
    )
    registry.registerNode(
      CommissionedNode(
        nodeId = 2L,
        nodeLabel = "Synthetic Light",
        vendorName = "SyntheticVendor",
        endpoints = emptyList(),
      )
    )

    registry.removeUnsupportedCluster(null, 1L, 1, 0x0006L)
    assertFalse(registry.getNode(1L)!!.getAllSupportedClusterIds().contains(0x0006L))
    assertTrue(registry.getNode(1L)!!.getAllSupportedClusterIds().contains(0x0402L))

    registry.syncWithDeviceIds(setOf(1L))
    assertEquals(setOf(1L), registry.getAllNodes().map { it.nodeId }.toSet())
  }

  @Test
  fun testSdkDeviceTypeNamesAndUnknownFallback() {
    assertEquals("Root Node", MatterDeviceTypes.nameForId(0x0016L))
    assertEquals("OTA Requestor", MatterDeviceTypes.nameForId(0x0012L))
    assertEquals("Generic Switch", MatterDeviceTypes.nameForId(0x000FL))
    assertEquals("Power Source", MatterDeviceTypes.nameForId(0x0011L))
    assertEquals("Thermostat Controller", MatterDeviceTypes.nameForId(0x030AL))
    assertEquals("Temperature Sensor", MatterDeviceTypes.nameForId(0x0302L))
    assertEquals("Humidity Sensor", MatterDeviceTypes.nameForId(0x0307L))
    assertEquals("Extended Color Light", MatterDeviceTypes.nameForId(0x010DL))
    assertEquals("Device Type 0x9999", MatterDeviceTypes.nameForId(0x9999L))
    assertEquals("Device Type 0x0000", MatterDeviceTypes.nameForId(0x0000L))

    val registry = CommissionedNodeRegistry()
    registry.registerNode(
      CommissionedNode(
        nodeId = 2L,
        nodeLabel = "w100",
        roomName = "Living Room",
        endpoints =
          listOf(
            CommissionedEndpoint(3, 0x000FL, "Generic Switch", setOf(0x003BL)),
            CommissionedEndpoint(4, 0x000FL, "Generic Switch", setOf(0x003BL)),
            CommissionedEndpoint(5, 0x000FL, "Generic Switch", setOf(0x003BL)),
          ),
      )
    )
    assertEquals("Button 1", registry.getSwitchButtonLabel(2L, 3))
    assertEquals("Button 2", registry.getSwitchButtonLabel(2L, 4))
    assertEquals("Button 3", registry.getSwitchButtonLabel(2L, 5))
  }

  @Test
  fun testRecordDiscoveredEndpointClusterReturnsNullWhenNodeRemoved() {
    val registry = CommissionedNodeRegistry()
    registry.autoRegisterCommissionedNode(99L)
    registry.resetFabric()
    val result = registry.recordDiscoveredEndpointCluster(null, 99L, 0, 0x0028L, 0x0001L, "Aqara")
    assertEquals(null, result)
    assertEquals(null, registry.getNode(99L))
  }
}
