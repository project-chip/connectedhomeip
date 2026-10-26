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
import chip.devicecontroller.ICDClientInfo
import chip.devicecontroller.model.NodeState
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.TestScope
import kotlinx.coroutines.test.advanceTimeBy
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.runCurrent
import kotlinx.coroutines.test.runTest
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class CenterConnectionManagerTest {
  private class FakeTransport : CenterTransport {
    val events = mutableListOf<String>()
    val requests = mutableListOf<SubscribeRequest>()
    /** Nodes whose CASE waits on a deferred; others connect at once. */
    val pendingCase = mutableMapOf<Long, CompletableDeferred<Long>>()
    var descriptorReport: NodeState? = null

    override suspend fun connect(nodeId: Long): Long {
      events += "connect:$nodeId"
      return pendingCase[nodeId]?.await() ?: (nodeId * 100)
    }

    override suspend fun readDescriptor(devicePtr: Long, nodeId: Long): NodeState? {
      events += "descriptor:$nodeId"
      return descriptorReport
    }

    override fun subscribe(request: SubscribeRequest) {
      events += "subscribe:${request.nodeId}"
      requests += request
    }

    override fun shutdown(nodeId: Long) {
      events += "shutdown:$nodeId"
    }

    fun lastRequest(nodeId: Long): SubscribeRequest = requests.last { it.nodeId == nodeId }

    fun count(event: String): Int = events.count { it == event }
  }

  private lateinit var registry: CommissionedNodeRegistry
  private lateinit var transport: FakeTransport
  private val dispatcher = StandardTestDispatcher()
  private val testScope = TestScope(dispatcher)

  private val light =
    CommissionedNode(
      nodeId = 3L,
      nodeLabel = "Light",
      roomName = "Living Room",
      endpoints =
        listOf(CommissionedEndpoint(1, 0x010DL, "Extended Color Light", setOf(0x0006L, 0x0008L))),
      descriptorVerified = true,
    )

  private val sensor =
    CommissionedNode(
      nodeId = 1L,
      nodeLabel = "W100",
      roomName = "Bedroom",
      endpoints =
        listOf(
          CommissionedEndpoint(0, 0x0016L, "Root Node", setOf(0x0028L, 0x0046L)),
          CommissionedEndpoint(1, 0x0302L, "Temperature Sensor", setOf(0x0402L, 0x0405L)),
          CommissionedEndpoint(2, 0x000FL, "Generic Switch", setOf(0x003BL)),
        ),
      descriptorVerified = true,
    )

  /** Peers the fake controller reports as having a registered ICD client. */
  private var registeredIcdPeers = emptySet<Long>()
  /** Number of controller ICD-client reads (each takes the Matter stack lock in production). */
  private var icdClientLookups = 0

  @Before
  fun setUp() {
    System.setProperty(
      "java.io.tmpdir",
      java.nio.file.Files.createTempDirectory("center_link").toFile().absolutePath,
    )
    HomeStateCache.clear()
    registry = CommissionedNodeRegistry()
    registry.registerNode(light)
    registry.registerNode(sensor)
    transport = FakeTransport()
    CenterConnectionManager.resetForTest(registry, transport, dispatcher)
    CenterConnectionManager.clock = { testScope.testScheduler.currentTime + 1_000_000L }
    CenterConnectionManager.icdClientLookup = {
      icdClientLookups++
      registeredIcdPeers
    }
  }

  @After
  fun tearDown() {
    CenterConnectionManager.resetForTest(null, null)
    HomeStateCache.clear()
  }

  private fun phase(nodeId: Long) = CenterConnectionManager.linkState(nodeId).phase

  /**
   * Seeds ICD Management OperatingMode: 0 = SIT, 1 = LIT. Uses the literal ID from
   * controller-clusters.matter (operatingMode = 8), not the production constant, so a wrong ID in
   * production fails these tests.
   */
  private fun seedOperatingMode(nodeId: Long, mode: Int) {
    HomeStateCache.put(nodeId, 0, 0x0046L, 0x0008L, mode, 0L, Source.UNICAST_READ)
  }

  private fun checkInRecord(nodeId: Long, counter: Long): CheckInRecord =
    HomeStateCache.getCheckInRecords(nodeId).first { it.counter == counter }

  @Test
  fun noSubscriptionWithoutLease() =
    testScope.runTest {
      assertFalse(CenterConnectionManager.connectAll())
      assertFalse(CenterConnectionManager.connect(3L))
      CenterConnectionManager.onNodeCommissioned(3L)
      advanceUntilIdle()
      assertTrue(
        "no transport traffic without a lease: ${transport.events}",
        transport.events.isEmpty(),
      )
      assertEquals(LinkPhase.NOT_CONNECTED, phase(3L))
    }

  @Test
  fun acquireOpensExactlyOneSubscriptionPerNode() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      for (nodeId in listOf(1L, 3L)) {
        assertEquals(1, transport.count("subscribe:$nodeId"))
        val shutdownAt = transport.events.indexOf("shutdown:$nodeId")
        val subscribeAt = transport.events.indexOf("subscribe:$nodeId")
        assertTrue("shutdown before subscribe for $nodeId", shutdownAt in 0 until subscribeAt)
        assertFalse(transport.lastRequest(nodeId).keepSubscriptions)
        assertEquals(LinkPhase.PRIMING, phase(nodeId))
      }
      assertEquals(60, transport.lastRequest(3L).maxInterval)
      assertEquals(300, transport.lastRequest(1L).maxInterval)
      assertEquals(1, transport.lastRequest(1L).eventPaths.size)

      transport.lastRequest(3L).established.onSubscriptionEstablished(42L)
      assertEquals(LinkPhase.LIVE, phase(3L))
      assertEquals(42L, CenterConnectionManager.linkState(3L).subscriptionId)
    }

  @Test
  fun onDoneNeverReportsLive() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val req = transport.lastRequest(3L)

      req.report.onDone()
      assertEquals(LinkPhase.OFFLINE, phase(3L))

      // Establishing again after onDone is still driven only by the established callback.
      req.established.onSubscriptionEstablished(1L)
      assertEquals(LinkPhase.LIVE, phase(3L))
      req.report.onDone()
      assertEquals(LinkPhase.OFFLINE, phase(3L))
    }

  @Test
  fun callbacksFromAnOlderGenerationAreDropped() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val oldRequest = transport.lastRequest(3L)
      oldRequest.established.onSubscriptionEstablished(1L)

      assertTrue(CenterConnectionManager.connectAll())
      advanceUntilIdle()
      val newRequest = transport.lastRequest(3L)
      assertTrue(oldRequest !== newRequest)
      assertEquals(LinkPhase.PRIMING, phase(3L))

      // The old ReadClient reports while being shut down: none of it may touch the new link.
      oldRequest.report.onDone()
      oldRequest.established.onSubscriptionEstablished(1L)
      oldRequest.report.onError(null, null, Exception("old"))
      oldRequest.resubscription.onResubscriptionAttempt(0L, 1000L)
      assertEquals(LinkPhase.PRIMING, phase(3L))

      newRequest.established.onSubscriptionEstablished(2L)
      assertEquals(LinkPhase.LIVE, phase(3L))
      assertEquals(2L, CenterConnectionManager.linkState(3L).subscriptionId)
    }

  @Test
  fun connectAllRedoesLiveSubscriptionShutdownFirstWithEventMin() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(3L).established.onSubscriptionEstablished(1L)
      transport.lastRequest(1L).established.onSubscriptionEstablished(2L)
      HomeStateCache.putEvent(
        nodeId = 1L,
        endpointId = 2,
        clusterId = 0x003BL,
        eventId = 0x06L,
        eventName = "MultiPressComplete",
        eventNumber = 41L,
        priority = 1,
        timestampMs = 0L,
      )
      transport.events.clear()

      assertTrue(CenterConnectionManager.connectAll())
      advanceUntilIdle()

      for (nodeId in listOf(1L, 3L)) {
        val order =
          transport.events.filter { it.endsWith(":$nodeId") }.map { it.substringBefore(":") }
        assertEquals(listOf("shutdown", "connect", "subscribe"), order)
      }
      assertEquals(42L, transport.lastRequest(1L).eventMin)
      assertNull(transport.lastRequest(3L).eventMin)
    }

  @Test
  fun duplicateConnectAllPressesAreIgnoredWhileInFlight() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(3L).established.onSubscriptionEstablished(1L)
      transport.lastRequest(1L).established.onSubscriptionEstablished(2L)
      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate

      assertTrue(CenterConnectionManager.connectAll())
      runCurrent()
      assertFalse("second press joins the batch", CenterConnectionManager.connectAll())
      assertTrue(CenterConnectionManager.connectAllProgress.value!!.active)

      gate.complete(100L)
      advanceUntilIdle()
      assertEquals(2, transport.count("subscribe:1"))
      assertEquals(2, transport.count("subscribe:3"))

      transport.lastRequest(1L).established.onSubscriptionEstablished(3L)
      transport.lastRequest(3L).established.onSubscriptionEstablished(4L)
      val progress = CenterConnectionManager.connectAllProgress.value!!
      assertFalse(progress.active)
      assertEquals(2, progress.live)
      assertEquals(2, progress.total)
    }

  @Test
  fun releaseTearsDownEveryNodeAndStopsRetries() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val req = transport.lastRequest(3L)
      req.established.onSubscriptionEstablished(1L)
      transport.events.clear()

      CenterConnectionManager.release("center")
      advanceUntilIdle()

      assertEquals(listOf("shutdown:1", "shutdown:3"), transport.events.sorted())
      assertEquals(LinkPhase.NOT_CONNECTED, phase(1L))
      assertEquals(LinkPhase.NOT_CONNECTED, phase(3L))
      assertNotNull(CenterConnectionManager.linkState(3L).lastHeardMs)

      // Late callbacks from the torn-down ReadClient change nothing and start nothing.
      req.resubscription.onResubscriptionAttempt(0L, 1000L)
      req.report.onDone()
      advanceTimeBy(120_000L)
      assertEquals(LinkPhase.NOT_CONNECTED, phase(3L))
      assertEquals(listOf("shutdown:1", "shutdown:3"), transport.events.sorted())
      assertFalse(CenterConnectionManager.connect(3L))
    }

  @Test
  fun reacquireBySameOwnerIsANoOp() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertEquals(1, transport.count("subscribe:3"))
      assertEquals(1, transport.count("subscribe:1"))
    }

  @Test
  fun teardownDuringCaseNeverSubscribes() =
    testScope.runTest {
      val gate = CompletableDeferred<Long>()
      transport.pendingCase[3L] = gate
      CenterConnectionManager.acquire(null, "center")
      runCurrent()
      assertEquals(LinkPhase.CONNECTING, phase(3L))

      CenterConnectionManager.release("center")
      gate.complete(300L)
      advanceUntilIdle()

      assertEquals(0, transport.count("subscribe:3"))
      assertEquals(LinkPhase.NOT_CONNECTED, phase(3L))
    }

  @Test
  fun hardCaseFailureMarksLitIcdAsleepWithoutRetry() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate
      assertTrue(CenterConnectionManager.connect(1L))
      runCurrent()
      assertEquals(LinkPhase.CONNECTING, phase(1L))
      gate.completeExceptionally(IllegalStateException("no route"))
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals("Couldn't reach", CenterConnectionManager.linkState(1L).reason)
      advanceTimeBy(120_000L)
      assertEquals("a LIT ICD waits for its Check-In", 1, transport.count("connect:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun hardCaseFailureKeepsRetryingSitIcd() =
    testScope.runTest {
      // SIT control: the sensor has the ICD Management cluster but reports OperatingMode SIT.
      seedOperatingMode(1L, 0)
      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate
      CenterConnectionManager.acquire(null, "center")
      runCurrent()
      assertEquals(LinkPhase.CONNECTING, phase(1L))
      gate.completeExceptionally(IllegalStateException("no route"))
      runCurrent()
      assertEquals("SIT shows Connect, not Asleep", LinkPhase.OFFLINE, phase(1L))
      assertFalse(CenterConnectionManager.linkState(1L).isLit)
      assertTrue(CenterConnectionManager.linkState(1L).isIcd)
      transport.pendingCase.remove(1L)
      advanceTimeBy(2_001L)
      runCurrent()
      assertEquals("SIT keeps the CASE backoff loop", 2, transport.count("connect:1"))
      assertEquals(1, transport.count("subscribe:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun hardCaseFailureMarksMainsOffline() =
    testScope.runTest {
      val gate = CompletableDeferred<Long>()
      transport.pendingCase[3L] = gate
      CenterConnectionManager.acquire(null, "center")
      runCurrent()
      gate.completeExceptionally(IllegalStateException("Unable to get connected device"))
      runCurrent()
      assertEquals(LinkPhase.OFFLINE, phase(3L))
      assertEquals("Couldn't reach", CenterConnectionManager.linkState(3L).reason)
      CenterConnectionManager.release("center")
    }

  @Test
  fun resubscribeAttemptsMapToRecoveringOrOfflineForMainsAndSit() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val lightReq = transport.lastRequest(3L)
      lightReq.established.onSubscriptionEstablished(1L)
      lightReq.resubscription.onResubscriptionAttempt(0L, 2000L)
      assertEquals(LinkPhase.RECOVERING, phase(3L))
      lightReq.resubscription.onResubscriptionAttempt(0L, 4000L)
      assertEquals(LinkPhase.OFFLINE, phase(3L))
      lightReq.established.onSubscriptionEstablished(2L)
      assertEquals(LinkPhase.LIVE, phase(3L))

      // The sensor is an ICD with no OperatingMode and no registered client: SIT, so not ASLEEP.
      val sensorReq = transport.lastRequest(1L)
      sensorReq.established.onSubscriptionEstablished(3L)
      sensorReq.resubscription.onResubscriptionAttempt(0L, 2000L)
      assertEquals(LinkPhase.RECOVERING, phase(1L))
    }

  @Test
  fun subscriptionErrorDoesNotShutDownAndMarksOffline() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val req = transport.lastRequest(3L)
      req.established.onSubscriptionEstablished(1L)
      transport.events.clear()

      req.report.onError(
        chip.devicecontroller.model.ChipAttributePath.newInstance(
          chip.devicecontroller.model.ChipPathId.forId(1L),
          chip.devicecontroller.model.ChipPathId.forId(6L),
          chip.devicecontroller.model.ChipPathId.forId(0L),
        ),
        null,
        Exception("path"),
      )
      assertEquals(LinkPhase.LIVE, phase(3L))

      req.report.onError(null, null, Exception("gave up"))
      advanceUntilIdle()
      assertEquals(LinkPhase.OFFLINE, phase(3L))
      assertTrue("no shutdown on error: ${transport.events}", transport.events.isEmpty())
    }

  @Test
  fun descriptorIsReadBeforeTheSingleSubscribe() =
    testScope.runTest {
      registry.registerNode(light.copy(descriptorVerified = false))
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val order = transport.events.filter { it.endsWith(":3") }.map { it.substringBefore(":") }
      assertEquals(listOf("shutdown", "connect", "descriptor", "subscribe"), order)
    }

  @Test
  fun lateDescriptorDiscoveryRebuildsSubscription() =
    testScope.runTest {
      // Node 3 is known only by its root endpoint, and its first Descriptor read fails.
      registry.unregisterNode(3L)
      registry.autoRegisterCommissionedNode(3L)
      transport.descriptorReport = null
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val first = transport.lastRequest(3L)
      first.established.onSubscriptionEstablished(1L)
      assertEquals(LinkPhase.LIVE, phase(3L))
      assertEquals(
        "a mains node subscribes with one wildcard path",
        chip.devicecontroller.model.ChipPathId.forWildcard(),
        first.attributePaths.single().clusterId,
      )

      // A later Descriptor read succeeds: the subscription is NOT rebuilt because MAINS nodes use
      // wildcard paths.
      transport.descriptorReport = bulbDescriptorReport()
      CenterConnectionManager.ensureDescriptorDiscovered(3L)
      advanceUntilIdle()

      assertTrue(registry.getNode(3L)!!.descriptorVerified)
      assertEquals(1, transport.count("subscribe:3")) // Still 1
    }

  @Test
  fun processNodeStateReportReplacesEndpointsAndResubscribesForICD() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(1L).established.onSubscriptionEstablished(1L)

      // Node 1 is an ICD. The descriptor changes.
      val report = bulbDescriptorReport()
      CenterConnectionManager.processNodeStateReport(1L, report)
      advanceUntilIdle()

      assertTrue(registry.getNode(1L)!!.getAllSupportedClusterIds().contains(0x0300L))
      assertEquals(2, transport.count("subscribe:1"))
    }

  @Test
  fun buildPathsSetsWildcardsForMainsAndTargetedForICD() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      // Node 3 is Mains (no ICD Management cluster) -> wildcard attribute and wildcard event
      val mainsReq = transport.lastRequest(3L)
      assertEquals(1, mainsReq.attributePaths.size)
      assertEquals(
        chip.devicecontroller.model.ChipPathId.forWildcard(),
        mainsReq.attributePaths[0].clusterId,
      )
      assertEquals(1, mainsReq.eventPaths.size)
      assertEquals(
        chip.devicecontroller.model.ChipPathId.forWildcard(),
        mainsReq.eventPaths[0].clusterId,
      )

      // Node 1 is ICD -> targeted attribute (including Descriptor 0x001D and ICDManagement 0x0046),
      // wildcard event
      val icdReq = transport.lastRequest(1L)
      assertTrue(icdReq.attributePaths.size > 1)
      assertTrue(icdReq.attributePaths.any { it.clusterId.id == 0x001DL })
      assertTrue(icdReq.attributePaths.any { it.clusterId.id == 0x0046L })
      assertEquals(1, icdReq.eventPaths.size)
      assertEquals(
        chip.devicecontroller.model.ChipPathId.forWildcard(),
        icdReq.eventPaths[0].clusterId,
      )
    }

  @Test
  fun checkInWhileOfflineRecordsItAndResubscribes() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(1L).report.onError(null, null, Exception("gone"))
      advanceUntilIdle()
      val subscribesBefore = transport.count("subscribe:1")

      CenterConnectionManager.onCheckIn(1L, counter = 7L)
      advanceUntilIdle()

      assertNotNull(HomeStateCache.lastCheckInMs(1L))
      assertEquals(7L, HomeStateCache.getRecentNodeEvents(1L).first { it.isCheckIn }.checkInCounter)
      assertEquals(subscribesBefore + 1, transport.count("subscribe:1"))
    }

  @Test
  fun partialDescriptorReportNeverDropsEndpoints() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(1L).established.onSubscriptionEstablished(1L)
      val before = registry.getNode(1L)!!.endpoints.map { it.endpointId }.toSet()

      // A later report carries only EP1's ServerList (no EP0 PartsList): not a complete Descriptor.
      val partial = NodeState()
      addAttr(partial, 1, 0x001DL, 0x0001L, arrayListOf(0x0402L))
      CenterConnectionManager.processNodeStateReport(1L, partial)
      advanceUntilIdle()

      assertEquals(before, registry.getNode(1L)!!.endpoints.map { it.endpointId }.toSet())
      assertEquals(1, transport.count("subscribe:1"))
    }

  @Test
  fun eventMinComesFromHighestEventNumberOfAnyCluster() =
    testScope.runTest {
      // A non-Switch event (BasicInformation StartUp) was the newest event seen from node 3.
      HomeStateCache.putEvent(3L, 0, 0x0028L, 0x00L, "StartUp", 41L, 2, 1000L)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertEquals(42L, transport.lastRequest(3L).eventMin)
    }

  private fun bulbDescriptorReport(): NodeState {
    val report = NodeState()
    addAttr(report, 0, 0x001DL, 0x0003L, arrayListOf(1L))
    addAttr(report, 0, 0x001DL, 0x0001L, arrayListOf(0x001DL, 0x0028L))
    addAttr(report, 1, 0x001DL, 0x0001L, arrayListOf(0x0006L, 0x0008L, 0x0300L))
    addAttr(
      report,
      1,
      0x001DL,
      0x0000L,
      arrayListOf(ChipStructs.DescriptorClusterDeviceTypeStruct(0x010DL, 2)),
    )
    return report
  }

  private fun addAttr(
    nodeState: NodeState,
    endpointId: Int,
    clusterId: Long,
    attributeId: Long,
    value: Any?,
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
    m.invoke(nodeState, endpointId, clusterId, attributeId, value, ByteArray(0), null)
  }

  @Test
  fun cancelStopsPendingNodesAndKeepsLiveOnes() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.pendingCase[1L] = CompletableDeferred()
      assertTrue(CenterConnectionManager.connectAll())
      runCurrent()
      transport.lastRequest(3L).established.onSubscriptionEstablished(5L)
      assertEquals(LinkPhase.LIVE, phase(3L))
      assertTrue(CenterConnectionManager.connectAllProgress.value!!.active)

      CenterConnectionManager.cancelConnectAll()
      runCurrent()
      assertEquals(LinkPhase.LIVE, phase(3L))
      assertEquals(LinkPhase.NOT_CONNECTED, phase(1L))
      assertEquals("Cancelled", CenterConnectionManager.linkState(1L).reason)
      val progress = CenterConnectionManager.connectAllProgress.value!!
      assertFalse(progress.active)
      assertTrue(progress.cancelled)
      CenterConnectionManager.release("center")
    }

  @Test
  fun perDeviceConnectOnlyStartsWhenNotUp() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val req = transport.lastRequest(3L)
      req.established.onSubscriptionEstablished(1L)
      assertFalse(CenterConnectionManager.connect(3L))

      req.report.onError(null, null, Exception("gone"))
      assertEquals(LinkPhase.OFFLINE, phase(3L))
      assertTrue(CenterConnectionManager.connect(3L))
      advanceUntilIdle()
      assertEquals(2, transport.count("subscribe:3"))
    }

  @Test
  fun successfulCommandMarksReachableAndReconnectsOfflineNode() =
    testScope.runTest {
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(3L).report.onError(null, null, Exception("gone"))
      assertEquals(LinkPhase.OFFLINE, phase(3L))

      CenterConnectionManager.noteReachable(3L)
      advanceUntilIdle()
      assertEquals(2, transport.count("subscribe:3"))
      assertNotNull(CenterConnectionManager.linkState(3L).lastHeardMs)
    }

  @Test
  fun cacheAgeIsZeroWhileLive() =
    testScope.runTest {
      val now = CenterConnectionManager.clock()
      assertEquals(600_000L, CenterConnectionManager.cacheAgeMs(3L, now - 600_000L))
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(3L).established.onSubscriptionEstablished(1L)
      assertEquals(0L, CenterConnectionManager.cacheAgeMs(3L, now - 600_000L))
    }

  @Test
  fun litIcdDefaultsAsleepAndIsNotDialedOnAcquire() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertTrue(CenterConnectionManager.linkState(1L).isLit)
      assertEquals(0, transport.count("subscribe:1")) // Not dialed
      assertEquals(LinkPhase.PRIMING, phase(3L))
      assertEquals(1, transport.count("subscribe:3")) // Light is dialed
      CenterConnectionManager.release("center")
    }

  @Test
  fun sitOperatingModeIsDialedOnAcquireEvenWithRegisteredClient() =
    testScope.runTest {
      // OperatingMode is authoritative: a device that reports SIT is pollable and is dialed.
      seedOperatingMode(1L, 0)
      registeredIcdPeers = setOf(1L)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      assertEquals(LinkPhase.PRIMING, phase(1L))
      assertEquals(1, transport.count("subscribe:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun userActiveModeTriggerInstructionIsNotOperatingMode() =
    testScope.runTest {
      // 0x0007 is UserActiveModeTriggerInstruction. A value there must not classify the node.
      HomeStateCache.put(1L, 0, 0x0046L, 0x0007L, 1, 0L, Source.UNICAST_READ)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      assertEquals(LinkPhase.PRIMING, phase(1L))
      assertEquals(1, transport.count("subscribe:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun litsFeatureBitAloneIsNotLitAndNonIcdNeverLit() =
    testScope.runTest {
      // FeatureMap LITS (0x04) only means LIT is supported. With no registered client the device
      // runs in SIT and must be dialed. A registered peer that is not an ICD is never LIT.
      HomeStateCache.put(1L, 0, 0x0046L, 0xFFFCL, 0x04, 0L, Source.UNICAST_READ)
      registeredIcdPeers = setOf(3L)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      assertEquals(LinkPhase.PRIMING, phase(1L))
      assertEquals(LinkPhase.PRIMING, phase(3L))
      assertFalse(CenterConnectionManager.linkState(3L).isLit)
      assertFalse(CenterConnectionManager.linkState(3L).isIcd)
      assertEquals("one controller read per Center entry, not one per node", 1, icdClientLookups)
      CenterConnectionManager.release("center")
    }

  @Test
  fun registeredIcdClientWithoutOperatingModeIsLit() =
    testScope.runTest {
      registeredIcdPeers = setOf(1L)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(0, transport.count("subscribe:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun checkInListenerNeverRunsOnTheCallingThreadAndNeverReadsIcdClients() =
    testScope.runTest {
      // The SDK calls the listener on the CHIP thread holding the non-recursive stack lock. Nothing
      // may happen synchronously, and the controller's ICD client list must not be read.
      registeredIcdPeers = setOf(1L)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      val lookupsBefore = icdClientLookups
      val eventsBefore = transport.events.toList()

      CenterConnectionManager.checkInListener.notifyCheckInMessage(
        ICDClientInfo(1L, 112233L, 10L, 0L, 112233L, ByteArray(16), ByteArray(16))
      )

      assertTrue("no record yet", HomeStateCache.getCheckInRecords(1L).isEmpty())
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(eventsBefore, transport.events)

      runCurrent()
      assertEquals(CheckInOutcome.WAKING, checkInRecord(1L, 10L).outcome)
      assertEquals(LinkPhase.WAKING, phase(1L))
      assertEquals(1, transport.count("connect:1"))
      assertEquals("check-in path must not read ICD clients", lookupsBefore, icdClientLookups)
      CenterConnectionManager.release("center")
    }

  @Test
  fun checkInFromUnknownOperatingModeMarksPeerAsLitWithoutControllerRead() =
    testScope.runTest {
      // The sensor is not known to be LIT yet; a received Check-In proves a registered client.
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      transport.lastRequest(1L).report.onError(null, null, Exception("gone"))
      assertEquals(LinkPhase.OFFLINE, phase(1L))
      val lookupsBefore = icdClientLookups

      CenterConnectionManager.onCheckIn(1L, 5L)
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))
      assertTrue(CenterConnectionManager.linkState(1L).isLit)
      assertEquals(lookupsBefore, icdClientLookups)
      CenterConnectionManager.release("center")
    }

  @Test
  fun freshlyCommissionedLitIcdIsDialedOnTheNextEnterOnly() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      // Commissioning finishes while the Device Center is not visible (no lease).
      CenterConnectionManager.onNodeCommissioned(1L)
      advanceUntilIdle()
      assertTrue(transport.events.isEmpty())

      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertEquals(1, transport.count("subscribe:1"))
      assertEquals(LinkPhase.PRIMING, phase(1L))

      // The bypass is used once: the next Center entry parks the LIT node again.
      CenterConnectionManager.release("center")
      advanceUntilIdle()
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(1, transport.count("subscribe:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun litIcdCheckInTriggersWakingThenLive() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertEquals(0, transport.count("subscribe:1"))

      CenterConnectionManager.onCheckIn(1L, 10L)
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))
      assertEquals(1, transport.count("subscribe:1"))
      assertEquals(CheckInOutcome.WAKING, checkInRecord(1L, 10L).outcome)

      transport.lastRequest(1L).established.onSubscriptionEstablished(1L)
      runCurrent()
      assertEquals(LinkPhase.LIVE, phase(1L))
      assertEquals(CheckInOutcome.LIVE, checkInRecord(1L, 10L).outcome)
      CenterConnectionManager.release("center")
    }

  @Test
  fun duplicateOrOlderCheckInCounterIgnored() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      CenterConnectionManager.onCheckIn(1L, 10L)
      advanceUntilIdle()
      val req1 = transport.lastRequest(1L)
      req1.established.onSubscriptionEstablished(1L)
      req1.report.onDone()
      advanceUntilIdle()
      assertEquals(LinkPhase.ASLEEP, phase(1L))

      val dials = transport.count("connect:1")
      CenterConnectionManager.onCheckIn(1L, 10L) // same counter
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(dials, transport.count("connect:1"))

      CenterConnectionManager.onCheckIn(1L, 9L) // older counter
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(dials, transport.count("connect:1"))
      CenterConnectionManager.release("center")
    }

  @Test
  fun checkInDuringInFlightSubscribeDoesNotRestart() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate

      CenterConnectionManager.onCheckIn(1L, 10L)
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))
      val generation = CenterConnectionManager.linkState(1L).generation

      CenterConnectionManager.onCheckIn(1L, 11L) // another checkin during WAKING
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))
      assertEquals("not restarted", generation, CenterConnectionManager.linkState(1L).generation)
      assertEquals(10L, CenterConnectionManager.linkState(1L).checkInCounter)
      assertEquals(CheckInOutcome.ALREADY_LIVE, checkInRecord(1L, 11L).outcome)
      gate.complete(100L)
      advanceUntilIdle()
      assertEquals(1, transport.count("connect:1"))
      assertEquals(1, transport.count("subscribe:1"))
      transport.lastRequest(1L).established.onSubscriptionEstablished(1L)
      runCurrent()
      assertEquals(CheckInOutcome.LIVE, checkInRecord(1L, 10L).outcome)
      assertTrue(
        "no Check-In row may stay WAKING",
        HomeStateCache.getCheckInRecords(1L).none { it.outcome == CheckInOutcome.WAKING },
      )
      CenterConnectionManager.release("center")
    }

  @Test
  fun caseFailureAfterCheckInMarksUnreachable() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate
      CenterConnectionManager.onCheckIn(1L, 10L)
      runCurrent()

      gate.completeExceptionally(IllegalStateException("No route"))
      runCurrent()
      assertEquals(LinkPhase.UNREACHABLE, phase(1L))
      val record = checkInRecord(1L, 10L)
      assertEquals(CheckInOutcome.FAILED, record.outcome)
      assertEquals("FAILED: Couldn't reach", record.outcomeText)
      CenterConnectionManager.release("center")
    }

  @Test
  fun wakingCheckInIsResolvedWhenTheSubscriptionEndsOrIsTornDown() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      // onDone before the subscription is established.
      CenterConnectionManager.onCheckIn(1L, 10L)
      runCurrent()
      transport.lastRequest(1L).report.onDone()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      assertEquals(CheckInOutcome.FAILED, checkInRecord(1L, 10L).outcome)

      // onError (SDK gave up) before the subscription is established.
      CenterConnectionManager.onCheckIn(1L, 11L)
      runCurrent()
      transport.lastRequest(1L).report.onError(null, null, Exception("gave up"))
      assertEquals(CheckInOutcome.FAILED, checkInRecord(1L, 11L).outcome)
      assertEquals("FAILED: gave up", checkInRecord(1L, 11L).outcomeText)

      // A resubscribe attempt before the subscription is established.
      CenterConnectionManager.onCheckIn(1L, 12L)
      runCurrent()
      transport.lastRequest(1L).resubscription.onResubscriptionAttempt(0L, 5000L)
      assertEquals(CheckInOutcome.FAILED, checkInRecord(1L, 12L).outcome)

      // Teardown while waking.
      transport.pendingCase[1L] = CompletableDeferred()
      CenterConnectionManager.onCheckIn(1L, 13L)
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))
      CenterConnectionManager.release("center")
      assertEquals(CheckInOutcome.CANCELLED, checkInRecord(1L, 13L).outcome)

      assertTrue(
        "no Check-In row may stay WAKING",
        HomeStateCache.getCheckInRecords(1L).none { it.outcome == CheckInOutcome.WAKING },
      )
    }

  @Test
  fun connectAllSupersedesWakingAndUpdatesProgress() =
    testScope.runTest {
      registry.unregisterNode(3L)
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      val gate = CompletableDeferred<Long>()
      transport.pendingCase[1L] = gate
      CenterConnectionManager.onCheckIn(1L, 10L)
      runCurrent()
      assertEquals(LinkPhase.WAKING, phase(1L))

      CenterConnectionManager.connectAll()
      runCurrent()
      assertEquals(LinkPhase.CONNECTING, phase(1L))
      assertEquals(CheckInOutcome.CANCELLED, checkInRecord(1L, 10L).outcome)
      assertEquals("CANCELLED: superseded by CONNECT_ALL", checkInRecord(1L, 10L).outcomeText)
      val progress = CenterConnectionManager.connectAllProgress.value
      assertNotNull(progress)
      assertTrue(progress!!.active)

      gate.complete(100L)
      advanceUntilIdle()
      transport.lastRequest(1L).established.onSubscriptionEstablished(1L)
      runCurrent()
      assertEquals(LinkPhase.LIVE, phase(1L))
      assertFalse(CenterConnectionManager.connectAllProgress.value!!.active)
      CenterConnectionManager.release("center")
    }

  @Test
  fun checkInCounterWrapsUint32() =
    testScope.runTest {
      registeredIcdPeers = setOf(1L)
      CenterConnectionManager.acquire(null, "center")
      runCurrent()

      CenterConnectionManager.checkInListener.notifyCheckInMessage(
        ICDClientInfo(1L, 112233L, 0xFFFF_FFFEL, 5L, 112233L, ByteArray(16), ByteArray(16))
      )
      runCurrent()
      assertEquals(3L, HomeStateCache.getCheckInRecords(1L).first().counter)
      CenterConnectionManager.release("center")
    }

  @Test
  fun litIcdSubscriptionLapseMarksAsleep() =
    testScope.runTest {
      seedOperatingMode(1L, 1)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()

      CenterConnectionManager.onCheckIn(1L, 10L)
      advanceUntilIdle()
      val req = transport.lastRequest(1L)
      req.established.onSubscriptionEstablished(1L)
      assertEquals(LinkPhase.LIVE, phase(1L))

      req.resubscription.onResubscriptionAttempt(0L, 1000L)
      runCurrent()
      assertEquals(LinkPhase.ASLEEP, phase(1L))
      CenterConnectionManager.release("center")
    }

  @Test
  fun sitIcdSubscriptionLapseIsRecoveringNotAsleep() =
    testScope.runTest {
      seedOperatingMode(1L, 0)
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      val req = transport.lastRequest(1L)
      req.established.onSubscriptionEstablished(1L)

      req.resubscription.onResubscriptionAttempt(0L, 1000L)
      assertEquals(LinkPhase.RECOVERING, phase(1L))
      req.report.onError(null, null, Exception("gone"))
      assertEquals("SIT shows Connect", LinkPhase.OFFLINE, phase(1L))
      CenterConnectionManager.release("center")
    }

  @Test
  fun descriptorDiscoveryDroppedByNewGenerationDoesNotLeakInFlight() =
    testScope.runTest {
      // Node 3 is known only by its root endpoint and its first Descriptor read fails.
      registry.unregisterNode(3L)
      registry.autoRegisterCommissionedNode(3L)
      transport.descriptorReport = null
      CenterConnectionManager.acquire(null, "center")
      advanceUntilIdle()
      assertFalse(registry.getNode(3L)!!.descriptorVerified)

      // Discovery is launched, then the link generation moves on before it runs.
      CenterConnectionManager.ensureDescriptorDiscovered(3L, devicePtr = 300L)
      CenterConnectionManager.release("center")
      advanceUntilIdle()
      val readsBefore = transport.count("descriptor:3")

      // A later discovery must still run: the dropped one released its in-flight slot.
      transport.descriptorReport = bulbDescriptorReport()
      CenterConnectionManager.ensureDescriptorDiscovered(3L, devicePtr = 300L)
      advanceUntilIdle()
      assertEquals(readsBefore + 1, transport.count("descriptor:3"))
      assertTrue(registry.getNode(3L)!!.descriptorVerified)
    }
}
