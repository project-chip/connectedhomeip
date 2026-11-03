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

import com.google.chip.chiptool.R
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class RoomDeviceHierarchyTest {

  private lateinit var registry: CommissionedNodeRegistry

  @Before
  fun setUp() {
    registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
  }

  @Test
  fun testHierarchyMappingForColorLight() {
    val lightNode = registry.getNode(0x1001L)
    assertNotNull(lightNode)

    val liveStates = mapOf("0x1001:1:6:0" to true, "0x1001:1:8:0" to 190)
    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(lightNode!!, liveStates)

    assertEquals("Living Room Ceiling Light", cardState.nodeLabel)
    assertEquals("Living Room", cardState.roomName)
    // No link state yet: not online, and no green "Live" status.
    assertFalse(cardState.isOnline)
    assertEquals("Not connected", cardState.linkStatusText)
    assertTrue(cardState.endpoints.isNotEmpty())

    // Verify endpoint 1 descriptor
    val ep1 = cardState.endpoints.first { it.endpointId == 1 }
    assertEquals("Extended Color Light", ep1.deviceTypeName)
    assertEquals(3, ep1.serverClusters.size)

    // Check cluster categories
    val onOffCluster =
      ep1.serverClusters.first { it.clusterId == MatterClusterMetaRegistry.CLUSTER_ON_OFF }
    assertEquals("Lighting & Power", onOffCluster.category)

    // Verify Quick Control: a dimmable color light (OnOff + LevelControl) gets the brightness
    // slider together with its power state.
    val qc = cardState.quickControl
    assertTrue(qc is QuickControlAction.BrightnessSlider)
    qc as QuickControlAction.BrightnessSlider
    assertEquals(true, qc.isOn)
    assertEquals(190 * 100 / 254, qc.levelPercent)
    assertEquals(1, qc.endpointId)
  }

  @Test
  fun testLevelOnlyEndpointGetsSliderWithoutPowerToggle() {
    val node =
      CommissionedNode(
        nodeId = 0x9101L,
        nodeLabel = "Level Only",
        roomName = "Lab",
        endpoints =
          listOf(
            CommissionedEndpoint(
              1,
              0x0101L,
              "Dimmable Light",
              setOf(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL),
            )
          ),
      )
    val card = RoomDeviceHierarchyMapper.toDeviceCardState(node, mapOf("0x9101:1:8:0" to 127))
    val qc = card.quickControl as QuickControlAction.BrightnessSlider
    assertNull(qc.isOn)
  }

  @Test
  fun testOnlineOnlyFromLinkState() {
    val lightNode = registry.getNode(0x1001L)!!
    for (phase in LinkPhase.values()) {
      val card =
        RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = LinkState(phase = phase))
      assertEquals(
        "isOnline for $phase",
        phase == LinkPhase.LIVE || phase == LinkPhase.ASLEEP,
        card.isOnline,
      )
    }
  }

  @Test
  fun testInspectorFormatsScaledSensorValues() {
    val ep =
      CommissionedEndpoint(
        endpointId = 1,
        deviceTypeId = 0x0302L,
        deviceTypeName = "Temperature Sensor",
        serverClusters =
          setOf(
            MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT,
            MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
          ),
      )
    val desc =
      RoomDeviceHierarchyMapper.toEndpointDescriptor(
        ep,
        mapOf("1:1026:0" to 2150, "1:1029:0" to 4500),
      )
    fun formatted(clusterId: Long) =
      desc.serverClusters
        .first { it.clusterId == clusterId }
        .attributes
        .first { it.attributeId == 0x0000L }
        .formattedValue
    assertEquals("21.5°C", formatted(MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT))
    assertEquals(
      "45.0%",
      formatted(MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT),
    )
  }

  @Test
  fun testHierarchyMappingForDoorLock() {
    val lockNode = registry.getNode(0x2001L)
    assertNotNull(lockNode)

    val liveStates = mapOf("0x2001:1:257:0" to 1) // 1=Locked
    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(lockNode!!, liveStates)

    assertEquals("Front Door Deadbolt", cardState.nodeLabel)
    assertEquals("Entryway", cardState.roomName)

    val lockBadge = cardState.badges.firstOrNull { it.type == DeviceBadgeType.STATE_LOCKED }
    assertNotNull(lockBadge)
    assertEquals("Locked", lockBadge?.value)

    assertTrue(cardState.quickControl is QuickControlAction.LockToggle)
  }

  @Test
  fun testHierarchyMappingForThermostat() {
    val thermostatNode = registry.getNode(0x3001L)
    assertNotNull(thermostatNode)

    val liveStates = mapOf("0x3001:1:513:0" to 2250) // 22.50 °C
    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(thermostatNode!!, liveStates)

    val tempBadge = cardState.badges.firstOrNull { it.type == DeviceBadgeType.TEMPERATURE }
    assertNotNull(tempBadge)
    assertEquals("22.5°C", tempBadge?.value)

    assertEquals(QuickControlAction.None, cardState.quickControl)
  }

  @Test
  fun testColorTemperatureFormatting() {
    val ep =
      CommissionedEndpoint(
        endpointId = 1,
        deviceTypeId = MatterDeviceTypes.EXTENDED_COLOR_LIGHT,
        deviceTypeName = "Extended Color Light",
        serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL),
      )
    val desc = RoomDeviceHierarchyMapper.toEndpointDescriptor(ep, mapOf("1:768:7" to 370L))
    val colorCluster =
      desc.serverClusters.first { it.clusterId == MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL }
    val tempAttr = colorCluster.attributes.first { it.attributeId == 0x0007L }

    assertEquals(AttributeControlType.SLIDER_COLOR_TEMP, tempAttr.controlType)
    assertTrue(
      tempAttr.formattedValue.contains("2703 K") ||
        tempAttr.formattedValue.contains("2702 K") ||
        tempAttr.formattedValue.contains("2700 K")
    )
  }

  @Test
  fun testRoomGroupingAndOnlineCounts() {
    val nodes = registry.getAllNodes()
    val grouped = nodes.groupBy { it.roomName }
    assertTrue(grouped.containsKey("Living Room"))
    assertTrue(grouped.containsKey("Kitchen"))
    assertTrue(grouped.containsKey("Entryway"))

    val livingRoomNodes = checkNotNull(grouped["Living Room"])
    assertTrue(livingRoomNodes.isNotEmpty())
  }

  @Test
  fun testEmptyFabricAndRoomHierarchyHandling() {
    registry.clear()
    assertEquals(0, registry.count())
    assertTrue(registry.getAllNodes().isEmpty())
    assertTrue(registry.getAllRooms().isEmpty())

    val emptyTopology = HomeFabricTopology(fabricId = 1L, rooms = emptyList())
    assertEquals(0, emptyTopology.totalDevicesCount)
    assertEquals(0, emptyTopology.totalOnlineCount)
    assertTrue(emptyTopology.rooms.isEmpty())
  }

  @Test
  fun testFiveStateLinkBadgesMapping() {
    val lightNode = registry.getNode(0x1001L)!!

    // 1. LIVE
    val liveState = LinkState(phase = LinkPhase.LIVE)
    val cardLive = RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = liveState)
    assertEquals("Live", cardLive.linkStatusText)
    assertEquals("#137333", cardLive.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_live, cardLive.linkStatusIconRes)
    assertEquals(false, cardLive.showConnectButton)

    // 2. CONNECTING / PRIMING
    val connectingState = LinkState(phase = LinkPhase.CONNECTING)
    val cardConnecting =
      RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = connectingState)
    assertEquals("Connecting…", cardConnecting.linkStatusText)
    assertEquals("#5F6368", cardConnecting.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_connecting, cardConnecting.linkStatusIconRes)
    assertEquals(false, cardConnecting.showConnectButton)

    // 3. RECOVERING (Still trying…)
    val recoveringState = LinkState(phase = LinkPhase.RECOVERING)
    val cardRecovering =
      RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = recoveringState)
    assertEquals("Still trying…", cardRecovering.linkStatusText)
    assertEquals("#5F6368", cardRecovering.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_connecting, cardRecovering.linkStatusIconRes)
    assertEquals(false, cardRecovering.showConnectButton)

    // 4. ASLEEP
    val asleepState =
      LinkState(phase = LinkPhase.ASLEEP, lastHeardMs = System.currentTimeMillis() - 120_000L)
    val cardAsleep = RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = asleepState)
    assertTrue(cardAsleep.linkStatusText.startsWith("Asleep · heard 2m ago"))
    assertEquals("#3C4F76", cardAsleep.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_asleep, cardAsleep.linkStatusIconRes)
    assertEquals(false, cardAsleep.showConnectButton)

    // 5a. OFFLINE - descriptor verified
    val verifiedNode = lightNode.copy(descriptorVerified = true)
    val offlineState = LinkState(phase = LinkPhase.OFFLINE, reason = "Couldn't reach")
    val cardOfflineVerified =
      RoomDeviceHierarchyMapper.toDeviceCardState(verifiedNode, linkState = offlineState)
    assertEquals("Offline · Couldn't reach", cardOfflineVerified.linkStatusText)
    assertEquals("#B3261E", cardOfflineVerified.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_offline, cardOfflineVerified.linkStatusIconRes)
    assertEquals(true, cardOfflineVerified.showConnectButton)

    // 5b. OFFLINE - descriptor unverified (Neutral "Not reachable · may be asleep")
    val unverifiedNode = lightNode.copy(descriptorVerified = false)
    val cardOfflineUnverified =
      RoomDeviceHierarchyMapper.toDeviceCardState(unverifiedNode, linkState = offlineState)
    assertEquals("Not reachable · may be asleep", cardOfflineUnverified.linkStatusText)
    assertEquals("#3C4F76", cardOfflineUnverified.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_asleep, cardOfflineUnverified.linkStatusIconRes)
    assertEquals(true, cardOfflineUnverified.showConnectButton)

    // 6. NOT_CONNECTED
    val notConnectedState = LinkState(phase = LinkPhase.NOT_CONNECTED)
    val cardNotConnected =
      RoomDeviceHierarchyMapper.toDeviceCardState(lightNode, linkState = notConnectedState)
    assertEquals("Not connected", cardNotConnected.linkStatusText)
    assertEquals("#5F6368", cardNotConnected.linkStatusColorHex)
    assertEquals(R.drawable.ic_center_not_connected, cardNotConnected.linkStatusIconRes)
    assertEquals(true, cardNotConnected.showConnectButton)
  }

  @Test
  fun testHumidityFormattingAndDisambiguation() {
    val climateNode =
      CommissionedNode(
        nodeId = 0x9001L,
        nodeLabel = "W100 Climate Sensor",
        roomName = "Bedroom",
        productName = "Matter Temp & Humidity Sensor",
        batteryPercent = 100,
        endpoints =
          listOf(
            CommissionedEndpoint(
              endpointId = 1,
              deviceTypeId = MatterDeviceTypes.HUMIDITY_SENSOR,
              deviceTypeName = "Humidity Sensor",
              serverClusters =
                setOf(MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT),
            )
          ),
      )
    registry.registerNode(climateNode)

    HomeStateCache.put(
      nodeId = 0x9001L,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
      attributeId = 0x0000L,
      value = 5000,
      timestampMs = System.currentTimeMillis(),
      source = Source.UNICAST_READ,
    )

    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(climateNode)
    assertNotNull(cardState.sensorReadingText)
    assertTrue(
      "Expected '50% humidity' in reading text: ${cardState.sensorReadingText}",
      cardState.sensorReadingText!!.contains("50% humidity"),
    )
    assertEquals(100, cardState.batteryPercent)
  }

  @Test
  fun testEndpoint0MultiClusterMapping() {
    val lightNode = registry.getNode(0x1001L)!!
    val cardState = RoomDeviceHierarchyMapper.toDeviceCardState(lightNode)

    val ep0 = cardState.endpoints.firstOrNull { it.endpointId == 0 }
    assertNotNull("Endpoint 0 must be present in cardState endpoints", ep0)
    assertTrue(ep0!!.isRootEndpoint)
    assertEquals("Root Node", ep0.deviceTypeName)

    val serverClusterIds = ep0.serverClusters.map { it.clusterId }.toSet()
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_GENERAL_COMMISSIONING))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_NETWORK_COMMISSIONING))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_GENERAL_DIAGNOSTICS))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_ACCESS_CONTROL))
    assertTrue(
      serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_ADMINISTRATOR_COMMISSIONING)
    )
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_OPERATIONAL_CREDENTIALS))
    assertTrue(serverClusterIds.contains(MatterClusterMetaRegistry.CLUSTER_GROUP_KEY_MANAGEMENT))

    val basicCluster =
      ep0.serverClusters.first {
        it.clusterId == MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION
      }
    assertEquals("BasicInformation", basicCluster.clusterName)
    assertTrue(basicCluster.attributes.any { it.attributeName == "VendorName" })
    assertTrue(basicCluster.attributes.any { it.attributeName == "NodeLabel" && it.isWritable })
    assertEquals("General & System", basicCluster.category)
  }
}
