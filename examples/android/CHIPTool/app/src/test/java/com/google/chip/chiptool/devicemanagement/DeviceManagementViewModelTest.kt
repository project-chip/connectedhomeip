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

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class DeviceManagementViewModelTest {

  private val viewModel =
    DeviceManagementViewModel(
      nodeRegistry = CommissionedNodeRegistry(),
      coroutineScope = CoroutineScope(Dispatchers.Unconfined),
    )

  private fun node(label: String, room: String, aliases: List<String> = emptyList()) =
    CommissionedNode(nodeId = 1L, nodeLabel = label, roomName = room, aliases = aliases)

  @Test
  fun emptyRoomOrLabelDoesNotMatchEveryQuery() {
    assertFalse(viewModel.matchesSearchQuery(node(label = "Desk Lamp", room = ""), "kitchen"))
    assertFalse(viewModel.matchesSearchQuery(node(label = "", room = "Study"), "kitchen"))
  }

  @Test
  fun forwardAndReverseContainmentStillMatch() {
    val lamp = node(label = "Lamp", room = "Living Room", aliases = listOf("reading light"))
    assertTrue(viewModel.matchesSearchQuery(lamp, "living"))
    assertTrue(viewModel.matchesSearchQuery(lamp, "living room lamp"))
    assertTrue(viewModel.matchesSearchQuery(lamp, "reading"))
    assertFalse(viewModel.matchesSearchQuery(lamp, "garage"))
  }
}
