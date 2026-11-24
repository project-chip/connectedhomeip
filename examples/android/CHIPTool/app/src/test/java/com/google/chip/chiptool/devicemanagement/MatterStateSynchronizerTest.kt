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
import org.junit.Assert.assertNull
import org.junit.Assert.fail
import org.junit.Before
import org.junit.Test

class MatterStateSynchronizerTest {

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var synchronizer: MatterStateSynchronizer

  @Before
  fun setUp() {
    HomeStateCache.clear()
    registry = CommissionedNodeRegistry()
    registry.loadTestSmartHomeFabric()
    synchronizer = MatterStateSynchronizer(registry)
  }

  @Test
  fun testOptimisticUpdateAndRollback() {
    val rollback =
      synchronizer.applyOptimisticUpdate(
        nodeId = 0x1001L,
        endpointId = 1,
        clusterId = 6L,
        attributeId = 0L,
        optimisticValue = true,
      )

    assertEquals(true, synchronizer.getCachedAttributeValue(0x1001L, 1, 6L, 0L))

    // Revert via rollback
    Thread.sleep(2) // ensure timestamp increments
    rollback.invoke()
    assertNull(synchronizer.getCachedAttributeValue(0x1001L, 1, 6L, 0L))
  }

  @Test
  fun testRecordConfirmedAttribute() {
    synchronizer.recordConfirmedAttribute(
      nodeId = 0x1001L,
      endpointId = 1,
      clusterId = 8L,
      attributeId = 0L,
      confirmedValue = 200,
    )

    assertEquals(200, synchronizer.getCachedAttributeValue(0x1001L, 1, 8L, 0L))
    val node = registry.getNode(0x1001L)
    assertEquals(200, node?.liveStates?.get("currentLevel"))
  }

  @Test
  fun testAllStatesFollowsCacheAndDropsClearedKeys() {
    awaitCollectorActive()
    val onOffKey = synchronizer.buildAttributeKey(0x1001L, 1, 6L, 0L)
    putAttr(0x1001L, 1, 6L, 0L, true)
    awaitUntil { synchronizer.allStates.value[onOffKey] == true }

    // A key removed from the cache must not be resurrected by the previous aggregate map.
    HomeStateCache.clear()
    val levelKey = synchronizer.buildAttributeKey(0x1001L, 1, 8L, 0L)
    putAttr(0x1001L, 1, 8L, 0L, 128)
    awaitUntil { synchronizer.allStates.value[levelKey] == 128 }
    assertFalse(synchronizer.allStates.value.containsKey(onOffKey))
    assertFalse(synchronizer.getAllCachedStates().containsKey(onOffKey))
  }

  @Test
  fun testAllStatesNeverOverridesFresherCacheValue() {
    awaitCollectorActive()
    val key = synchronizer.buildAttributeKey(0x1001L, 1, 6L, 0L)
    putAttr(0x1001L, 1, 6L, 0L, false)
    awaitUntil { synchronizer.allStates.value[key] == false }

    putAttr(0x1001L, 1, 6L, 0L, true)
    assertEquals(true, synchronizer.getAllCachedStates()[key])
    awaitUntil { synchronizer.allStates.value[key] == true }
  }

  private var tick = 0L

  private fun putAttr(nodeId: Long, ep: Int, clusterId: Long, attrId: Long, value: Any?) {
    HomeStateCache.put(
      nodeId,
      ep,
      clusterId,
      attrId,
      value,
      System.currentTimeMillis() + ++tick,
      Source.SUBSCRIPTION_REPORT,
    )
  }

  /** The synchronizer subscribes asynchronously; emit a probe attribute until it is observed. */
  private fun awaitCollectorActive() {
    val probeKey = synchronizer.buildAttributeKey(0x7FFFL, 9, 0x0028L, 0x0001L)
    awaitUntil {
      putAttr(0x7FFFL, 9, 0x0028L, 0x0001L, tick)
      synchronizer.allStates.value.containsKey(probeKey)
    }
    HomeStateCache.clear()
  }

  private fun awaitUntil(condition: () -> Boolean) {
    val deadline = System.currentTimeMillis() + 5_000L
    while (!condition()) {
      if (System.currentTimeMillis() > deadline) fail("condition not met within 5s")
      Thread.sleep(10)
    }
  }
}
