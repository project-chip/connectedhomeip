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
import kotlinx.coroutines.runBlocking
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.mockito.Mockito.mock

class UniversalMatterDispatcherTest {

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var dispatcher: UniversalMatterDispatcher
  private lateinit var mockContext: Context

  @Before
  fun setUp() {
    HomeStateCache.clear()
    registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
    dispatcher = UniversalMatterDispatcher(registry)
    mockContext = mock(Context::class.java)
  }

  @After
  fun tearDown() {
    HomeStateCache.clear()
  }

  /** Two-gang switch on a synthetic vendor, so uncached reads never touch the network. */
  private fun registerTwoGangSwitch(): Long {
    val nodeId = 0x9001L
    registry.registerNode(
      CommissionedNode(
        nodeId = nodeId,
        nodeLabel = "Two Gang Switch",
        roomName = "Hall",
        vendorName = "SyntheticVendor",
        endpoints =
          listOf(
            CommissionedEndpoint(1, 0x0100L, "On/Off Light", setOf(0x0006L)),
            CommissionedEndpoint(2, 0x0100L, "On/Off Light", setOf(0x0006L)),
          ),
        descriptorVerified = true,
      )
    )
    return nodeId
  }

  @Test
  fun testPinnedReadNeverReturnsAnotherEndpointsCachedValue() = runBlocking {
    val nodeId = registerTwoGangSwitch()
    HomeStateCache.put(
      nodeId = nodeId,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
      attributeId = 0x0000L,
      value = true,
      timestampMs = System.currentTimeMillis(),
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = nodeId,
        endpointId = 2,
        clusterId = MatterClusterMetaRegistry.CLUSTER_ON_OFF,
        attributeId = 0x0000L,
      )

    // EP2 is not cached, so the value must come from EP2 (the synthetic default), not EP1's `true`.
    assertTrue(result.isSuccess)
    assertEquals(false, result.decodedAttribute?.rawValue)
    assertEquals(false, HomeStateCache.get(nodeId, 2, 0x0006L, 0x0000L)?.value)
  }

  @Test
  fun testEndpointZeroReadDoesNotUseChildEndpointCache() = runBlocking {
    val nodeId = registerTwoGangSwitch()
    HomeStateCache.put(
      nodeId = nodeId,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR,
      attributeId = 0x0001L,
      value = listOf(0x0006L, 0x001DL),
      timestampMs = System.currentTimeMillis(),
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = nodeId,
        endpointId = 0,
        clusterId = MatterClusterMetaRegistry.CLUSTER_DESCRIPTOR,
        attributeId = 0x0001L,
      )

    assertNotEquals(listOf(0x0006L, 0x001DL), result.decodedAttribute?.rawValue)
  }

  @Test
  fun testDecodedTemperatureStatusFeedback() = runBlocking {
    val now = System.currentTimeMillis()
    HomeStateCache.put(
      nodeId = 0x5001L,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT,
      attributeId = 0x0000L,
      value = 2240L,
      timestampMs = now,
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = 0x5001L,
        endpointId = 1,
        clusterId = MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT,
        attributeId = 0x0000L,
        targetDescription = "Living Room Climate Sensor",
      )

    assertTrue(result.isSuccess)
    assertEquals("22.4°C", result.decodedAttribute?.formattedValue)
    assertTrue(
      result.feedbackMessage.startsWith("The temperature at Living Room Climate Sensor is 22.4°C.")
    )
  }

  @Test
  fun testDecodedHumidityStatusFeedback() = runBlocking {
    val now = System.currentTimeMillis()
    HomeStateCache.put(
      nodeId = 0x5001L,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
      attributeId = 0x0000L,
      value = 4800L,
      timestampMs = now,
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = 0x5001L,
        endpointId = 1,
        clusterId = MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
        attributeId = 0x0000L,
        targetDescription = "Living Room Climate Sensor",
      )

    assertTrue(result.isSuccess)
    assertEquals("48.0%", result.decodedAttribute?.formattedValue)
    assertTrue(
      result.feedbackMessage.startsWith(
        "The relative humidity at Living Room Climate Sensor is 48.0%."
      )
    )
  }

  @Test
  fun testDecodedLockStateFeedback() = runBlocking {
    val now = System.currentTimeMillis()
    HomeStateCache.put(
      nodeId = 0x2001L,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
      attributeId = 0x0000L,
      value = 1L,
      timestampMs = now,
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = 0x2001L,
        endpointId = 1,
        clusterId = MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK,
        attributeId = 0x0000L,
        targetDescription = "Front Door Deadbolt",
      )

    assertTrue(result.isSuccess)
    assertEquals("Locked", result.decodedAttribute?.formattedValue)
    assertTrue(result.feedbackMessage.startsWith("Front Door Deadbolt is currently Locked."))
  }

  @Test
  fun testDecodedSmokeAlarmStateFeedback() = runBlocking {
    val now = System.currentTimeMillis()
    HomeStateCache.put(
      nodeId = 0x5003L,
      endpointId = 1,
      clusterId = MatterClusterMetaRegistry.CLUSTER_SMOKE_CO_ALARM,
      attributeId = 0x0001L,
      value = 0L,
      timestampMs = now,
      source = Source.SUBSCRIPTION_REPORT,
    )

    val result =
      dispatcher.readAttribute(
        context = mockContext,
        nodeId = 0x5003L,
        endpointId = 1,
        clusterId = MatterClusterMetaRegistry.CLUSTER_SMOKE_CO_ALARM,
        attributeId = 0x0001L,
        targetDescription = "Kitchen Smoke Detector",
      )

    assertTrue(result.isSuccess)
    assertEquals("OK", result.decodedAttribute?.formattedValue)
    assertTrue(result.feedbackMessage.startsWith("Smoke sensor on Kitchen Smoke Detector is OK."))
  }

  @Test
  fun testDecodeReportedAttributeFallsBackToTlvWhenValueNull() {
    val writer = matter.tlv.TlvWriter()
    writer.put(matter.tlv.AnonymousTag, 2150.toShort())
    val tlvBytes = writer.validateTlv().getEncoded()
    val attrState = chip.devicecontroller.model.AttributeState(null, tlvBytes, null)
    val tempMeta =
      MatterClusterMetaRegistry.getCluster(
          MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT
        )
        ?.attributes
        ?.get(0x0000L)
    val method =
      UniversalMatterDispatcher::class
        .java
        .getDeclaredMethod(
          "decodeReportedAttribute",
          chip.devicecontroller.model.AttributeState::class.java,
          MatterAttributeMeta::class.java,
        )
    method.isAccessible = true
    val decoded = method.invoke(dispatcher, attrState, tempMeta) as? DecodedAttributeValue
    assertEquals("21.5°C", decoded?.formattedValue)
  }
}
