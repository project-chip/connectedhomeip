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

class EndpointInspectorViewModelTest {

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var synchronizer: MatterStateSynchronizer
  private lateinit var dispatcher: UniversalMatterDispatcher
  private lateinit var viewModel: EndpointInspectorViewModel

  @Before
  fun setUp() {
    registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
    synchronizer = MatterStateSynchronizer(registry)
    dispatcher = UniversalMatterDispatcher(registry)

    viewModel = EndpointInspectorViewModel(0x1001L, registry, synchronizer, dispatcher)
  }

  @Test
  fun testEndpointDiscovery() {
    val endpoints = viewModel.endpoints.value
    assertTrue(endpoints.isNotEmpty())

    val ep1 = endpoints.firstOrNull { it.endpointId == 1 }
    assertNotNull(ep1)
    assertEquals("Extended Color Light", ep1?.deviceTypeName)
  }

  @Test
  fun testClusterInspection() {
    val ep1 = viewModel.endpoints.value.first { it.endpointId == 1 }
    val clusterIds = ep1.serverClusters.map { it.clusterId }

    assertTrue(clusterIds.contains(MatterClusterMetaRegistry.CLUSTER_ON_OFF))
    assertTrue(clusterIds.contains(MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL))
    assertTrue(clusterIds.contains(MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL))
  }

  @Test
  fun testAttributeControlsInCluster() {
    val ep1 = viewModel.endpoints.value.first { it.endpointId == 1 }
    val onOffCluster =
      ep1.serverClusters.first { it.clusterId == MatterClusterMetaRegistry.CLUSTER_ON_OFF }
    val onOffAttr = onOffCluster.attributes.first { it.attributeId == 0x0000L }

    // OnOff (0x0006/0x0000) is read-only per onoff-cluster.xml; toggling goes through commands.
    assertEquals(AttributeControlType.READONLY_TEXT, onOffAttr.controlType)
    assertFalse(onOffAttr.isWritable)
    assertTrue(onOffAttr.isReportable)

    val colorCluster =
      ep1.serverClusters.first { it.clusterId == MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL }
    val kelvinAttr = colorCluster.attributes.first { it.attributeId == 0x0007L }
    assertEquals(AttributeControlType.SLIDER_COLOR_TEMP, kelvinAttr.controlType)
  }

  @Test
  fun testSelectEndpointIndex() {
    viewModel.selectEndpoint(0)
    assertEquals(0, viewModel.selectedEndpointIndex.value)
    assertNotNull(viewModel.currentEndpoint.value)
  }
}
