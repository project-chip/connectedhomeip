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
import chip.devicecontroller.ChipEventStructs
import chip.devicecontroller.ICDClientInfo
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.EventState
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.clusterclient.ICDCheckInCallback
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicLong
import kotlin.coroutines.resume
import kotlin.math.max
import kotlin.math.min
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.Semaphore
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.sync.withPermit
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader

/** Connection phase of one node, as shown by the Device Center. */
enum class LinkPhase {
  NOT_CONNECTED,
  CONNECTING,
  PRIMING,
  LIVE,
  RECOVERING,
  ASLEEP,
  OFFLINE,
  WAKING,
  UNREACHABLE,
}

/**
 * Link state of one node. [lastHeardMs] is the liveness watermark: the last time the node was
 * proven reachable (subscription established, report received, successful command or read, or the
 * moment a live subscription was torn down).
 *
 * [isIcd] is true for any node with the ICD Management cluster. [isLit] is true only for a LIT ICD
 * (OperatingMode is LIT, or this controller has a Check-In client registered for it). Only a LIT
 * ICD is parked ASLEEP to wait for its next Check-In; a SIT ICD is retried like a mains node.
 */
data class LinkState(
  val phase: LinkPhase = LinkPhase.NOT_CONNECTED,
  val generation: Long = 0L,
  val sinceMs: Long = 0L,
  val lastHeardMs: Long? = null,
  val reason: String? = null,
  val isIcd: Boolean = false,
  val subscriptionId: Long? = null,
  val checkInCounter: Long? = null,
  val isLit: Boolean = false,
)

/** Progress of one Connect All press. */
data class ConnectAllProgress(
  val total: Int,
  val settled: Int,
  val live: Int,
  val active: Boolean,
  val cancelled: Boolean = false,
)

/** One subscribe request handed to the transport. */
data class SubscribeRequest(
  val nodeId: Long,
  val devicePtr: Long,
  val attributePaths: List<ChipAttributePath>,
  val eventPaths: List<ChipEventPath>,
  val minInterval: Int,
  val maxInterval: Int,
  val keepSubscriptions: Boolean,
  val eventMin: Long?,
  val imTimeoutMs: Int,
  val established: SubscriptionEstablishedCallback,
  val resubscription: ResubscriptionAttemptCallback,
  val report: ReportCallback,
)

/** Seam between the link state machine and the Matter controller. Tests use a fake. */
interface CenterTransport {
  /** Opens (or reuses) a CASE session and returns the device pointer. Throws on failure. */
  suspend fun connect(nodeId: Long): Long

  /** Reads the Descriptor cluster once. Returns null if the read failed. */
  suspend fun readDescriptor(devicePtr: Long, nodeId: Long): NodeState?

  fun subscribe(request: SubscribeRequest)

  fun shutdown(nodeId: Long)
}

/**
 * Process-wide owner of the commissioned-node registry and of every node subscription.
 *
 * Subscriptions exist only while a lease is held (the Device Center is visible). Each node has a
 * link with a generation counter; every SDK callback carries the generation it was created with and
 * is dropped when the link has moved on. The app retries only CASE; once subscribeToPath has been
 * called, the SDK resubscribe logic owns retries until the link is torn down.
 */
object CenterConnectionManager {
  private const val TAG = "CenterLink"

  private const val PRIME_TIMEOUT_MAINS_MS = 10_000
  private const val PRIME_TIMEOUT_ICD_MS = 30_000
  private const val MAX_INTERVAL_MAINS_S = 60
  private const val MAX_INTERVAL_ICD_S = 300
  private const val MAX_PARALLEL_CASE = 4
  private const val MAINS_RESUBSCRIBE_ATTEMPTS_BEFORE_OFFLINE = 2
  private const val CASE_RETRY_BASE_MS = 2_000L
  private const val CASE_RETRY_MAX_MS = 60_000L
  private const val CLUSTER_ICD_MANAGEMENT = 0x0046L
  /** ICD Management OperatingMode (0x0007 is UserActiveModeTriggerInstruction, a string). */
  private const val ATTR_ICD_OPERATING_MODE = 0x0008L
  /** OperatingModeEnum: 0 = SIT, 1 = LIT. */
  private const val ICD_OPERATING_MODE_LIT = 1

  private val DEFAULT_SUBSCRIBED_CLUSTERS =
    listOf(
      0x0402L, // TempMeasurement
      0x0405L, // RelHumidity
      0x0006L, // OnOff
      0x0201L, // Thermostat
      0x0008L, // Level
      0x0045L, // BooleanState
      0x0406L, // Occupancy
      0x0400L, // Illuminance
      0x0101L, // DoorLock
      0x0102L, // WindowCovering
      0x0300L, // ColorControl
      0x0202L, // FanControl
    )

  private enum class Mode {
    /** Center became visible: keep links that are up or on their way up. */
    ENTER,
    /** Connect All: redo every link, joining links that are already connecting. */
    CONNECT_ALL,
    /** Per-device Connect: start a link unless it is up or on its way up. */
    SINGLE,
    /**
     * The descriptor was verified after the subscription was issued: rebuild the subscription so it
     * covers the newly discovered endpoints and clusters. A link still doing CASE is kept, because
     * runLink re-reads the node before it subscribes.
     */
    REFRESH_PATHS,
    /** check-in while visible: dial and go to WAKING. */
    CHECK_IN,
  }

  private class NodeLink {
    val mutex = Mutex()
    val gen = AtomicLong(0L)
    @Volatile var job: Job? = null
    @Volatile var resubscribeAttempts = 0
  }

  private val stateLock = Any()
  @Volatile private var appContext: Context? = null

  fun appContextOrNull(): Context? = appContext

  @Volatile private var sharedRegistry: CommissionedNodeRegistry? = null
  @Volatile private var transportOverride: CenterTransport? = null
  @Volatile private var chipTransport: CenterTransport? = null
  @Volatile private var scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)
  @Volatile private var caseSemaphore = Semaphore(MAX_PARALLEL_CASE)
  private val links = ConcurrentHashMap<Long, NodeLink>()
  private val descriptorInFlight = ConcurrentHashMap.newKeySet<Long>()
  /**
   * Nodes commissioned but not dialed yet. Their next link is dialed even if they are LIT ICDs: the
   * device is in its post-commissioning StayActive window and has never been subscribed.
   */
  private val pendingFreshNodes = ConcurrentHashMap.newKeySet<Long>()
  private val leases = mutableSetOf<String>()
  private var batchNodes: Set<Long> = emptySet()

  private val _linkStates = MutableStateFlow<Map<Long, LinkState>>(emptyMap())
  val linkStates: StateFlow<Map<Long, LinkState>> = _linkStates.asStateFlow()

  private val _connectAllProgress = MutableStateFlow<ConnectAllProgress?>(null)
  val connectAllProgress: StateFlow<ConnectAllProgress?> = _connectAllProgress.asStateFlow()

  /** Wall clock, replaceable in tests. */
  @Volatile var clock: () -> Long = { System.currentTimeMillis() }

  // ---------------------------------------------------------------------------------------------
  // Registry
  // ---------------------------------------------------------------------------------------------

  /** Returns the single registry shared by the Device Center and commissioning. */
  fun registry(context: Context?): CommissionedNodeRegistry {
    sharedRegistry?.let {
      return it
    }
    synchronized(stateLock) {
      sharedRegistry?.let {
        return it
      }
      val app = context?.applicationContext ?: context
      if (app != null) {
        appContext = app
        HomeStateCache.init(app)
      }
      val reg = CommissionedNodeRegistry(app)
      sharedRegistry = reg
      return reg
    }
  }

  private fun registryOrNull(): CommissionedNodeRegistry? = sharedRegistry

  // ---------------------------------------------------------------------------------------------
  // Public state
  // ---------------------------------------------------------------------------------------------

  fun linkState(nodeId: Long): LinkState = _linkStates.value[nodeId] ?: LinkState()

  fun isCenterActive(): Boolean = synchronized(stateLock) { leases.isNotEmpty() }

  /**
   * Age of a cached value, judged by the link's liveness watermark rather than by when the value
   * last changed. Returns 0 while the node is live (the subscription vouches for the value).
   */
  fun cacheAgeMs(nodeId: Long, updatedAtMs: Long): Long {
    val state = linkState(nodeId)
    if (state.phase == LinkPhase.LIVE) return 0L
    val verifiedAt = max(updatedAtMs, state.lastHeardMs ?: 0L)
    return max(0L, clock() - verifiedAt)
  }

  // ---------------------------------------------------------------------------------------------
  // Lease (Center visibility)
  // ---------------------------------------------------------------------------------------------

  /**
   * Called when the Device Center becomes visible. Leases are keyed by owner, so re-acquiring after
   * a configuration change is a no-op.
   */
  fun acquire(context: Context?, owner: String) {
    val reg = registry(context)
    if (context != null) ensureCheckInListener()
    val first =
      synchronized(stateLock) {
        val added = leases.add(owner)
        added && leases.size == 1
      }
    if (!first) {
      log("center acquire owner=$owner ignored (lease already held)")
      return
    }
    refreshRegisteredIcdPeers {
      val nodeIds = reg.getAllNodes().map { it.nodeId }.sorted()
      log("center acquire owner=$owner nodes=$nodeIds")
      for (nodeId in nodeIds) startLink(nodeId, Mode.ENTER)
    }
  }

  // ---------------------------------------------------------------------------------------------
  // ICD Check-In
  // ---------------------------------------------------------------------------------------------

  /**
   * Peers for which this controller has a registered ICD (Check-In) client. Read from the
   * controller only by [refreshRegisteredIcdPeers], never on a link path, so classifying a node as
   * LIT never takes the Matter stack lock.
   */
  @Volatile private var registeredIcdPeers: Set<Long> = emptySet()

  /**
   * Reads the registered ICD clients from the controller. This takes the Matter stack lock, so it
   * must never run on the CHIP thread (the lock is not recursive) or block the Android UI thread.
   * Replaceable in tests.
   */
  private val defaultIcdClientLookup: () -> Set<Long> = ::readRegisteredIcdPeers
  @Volatile internal var icdClientLookup: () -> Set<Long> = defaultIcdClientLookup
  @Volatile internal var forceAsyncIcdClientLookup: Boolean = false
  @Volatile
  internal var icdLookupDispatcher: kotlinx.coroutines.CoroutineDispatcher = Dispatchers.IO

  private fun readRegisteredIcdPeers(): Set<Long> {
    val ctx = appContextOrNull() ?: return emptySet()
    return ChipClient.getDeviceController(ctx).getICDClientInfo().map { it.peerNodeId }.toSet()
  }

  /**
   * Called from the main thread (Center entry, commissioning), never from a CHIP callback.
   * Production JNI lookup runs on [Dispatchers.IO] so `AcquireChipStackLock()` cannot block the UI
   * thread; test overrides run synchronously.
   */
  private fun refreshRegisteredIcdPeers(then: () -> Unit = {}) {
    val lookup = icdClientLookup
    val update = {
      try {
        registeredIcdPeers = lookup()
      } catch (e: Exception) {
        logE("reading registered ICD clients failed; keeping ${registeredIcdPeers.size} cached", e)
      }
    }
    if (lookup === defaultIcdClientLookup || forceAsyncIcdClientLookup) {
      scope.launch(icdLookupDispatcher) {
        update()
        then()
      }
    } else {
      update()
      then()
    }
  }

  @Volatile private var checkInListenerRegistered = false

  /**
   * Runs on the CHIP thread with the Matter stack lock held: AndroidCheckInDelegate calls into Java
   * without releasing it, and the lock is not recursive. Any controller call made synchronously
   * from here (getICDClientInfo, getConnectedDevicePointer, ...) would deadlock the Matter thread,
   * so the Check-In is handled on [scope] instead.
   */
  internal val checkInListener =
    object : ICDCheckInCallback {
      override fun notifyCheckInMessage(info: ICDClientInfo) {
        val nodeId = info.peerNodeId
        val counter = (info.startCounter + info.offset) and 0xFFFF_FFFFL
        scope.launch { onCheckIn(nodeId, counter) }
      }
    }

  /**
   * Listens for ICD Check-In messages for the lifetime of the app (they can arrive at any time).
   */
  private fun ensureCheckInListener() {
    if (checkInListenerRegistered) return
    synchronized(stateLock) {
      if (checkInListenerRegistered) return
      ChipClient.addICDCheckInCallback(checkInListener)
      checkInListenerRegistered = true
    }
  }

  /**
   * A LIT ICD sends a Check-In when it has no active subscription from this client. Record it for
   * the UI, mark the node as just heard, and resubscribe if Device Center is visible and the link
   * is ASLEEP or OFFLINE (noteReachable handles both). Never call this on the CHIP thread.
   */
  internal fun onCheckIn(nodeId: Long, counter: Long) {
    val isGreater = HomeStateCache.putCheckIn(nodeId, counter, clock())
    val currentPhaseBefore = linkState(nodeId).phase
    log("check-in node=$nodeId counter=$counter phase=$currentPhaseBefore")

    synchronized(stateLock) {
      // Only a registered ICD client can receive a Check-In, so remember the peer without asking
      // the controller.
      if (nodeId !in registeredIcdPeers) registeredIcdPeers = registeredIcdPeers + nodeId
      val current = _linkStates.value[nodeId] ?: LinkState()
      put(
        nodeId,
        current.copy(
          lastHeardMs = clock(),
          checkInCounter =
            if (isGreater && current.phase != LinkPhase.WAKING) counter else current.checkInCounter,
        ),
      )
    }

    if (!isGreater) return
    if (!isCenterActive()) {
      HomeStateCache.updateCheckInOutcome(nodeId, counter, CheckInOutcome.SKIPPED)
      return
    }

    if (
      currentPhaseBefore == LinkPhase.ASLEEP ||
        currentPhaseBefore == LinkPhase.NOT_CONNECTED ||
        currentPhaseBefore == LinkPhase.OFFLINE ||
        currentPhaseBefore == LinkPhase.UNREACHABLE
    ) {
      // Mark WAKING before the link starts: runLink may finish (and resolve the outcome in signal)
      // before startLink returns.
      HomeStateCache.updateCheckInOutcome(nodeId, counter, CheckInOutcome.WAKING)
      if (startLink(nodeId, Mode.CHECK_IN)) {
        log("check-in node=$nodeId counter=$counter -> WAKING")
      } else {
        HomeStateCache.updateCheckInOutcome(nodeId, counter, CheckInOutcome.SKIPPED)
      }
    } else {
      HomeStateCache.updateCheckInOutcome(nodeId, counter, CheckInOutcome.ALREADY_LIVE)
    }
  }

  /** Called when the Device Center stops being visible. Tears every subscription down at once. */
  fun release(owner: String) {
    val last = synchronized(stateLock) { leases.remove(owner) && leases.isEmpty() }
    if (last) {
      log("center release owner=$owner -> teardown")
      teardownAll("center released")
    }
  }

  // ---------------------------------------------------------------------------------------------
  // Connect All / Connect / Cancel
  // ---------------------------------------------------------------------------------------------

  /**
   * (Re)subscribes every commissioned node. Links that are already connecting are joined; every
   * other link is shut down and subscribed again. Returns false if ignored (no lease, or a Connect
   * All is already in flight).
   */
  fun connectAll(): Boolean {
    val reg = registryOrNull() ?: return false
    val nodeIds = reg.getAllNodes().map { it.nodeId }.sorted()
    synchronized(stateLock) {
      if (leases.isEmpty()) {
        log("connectAll ignored: center not visible")
        return false
      }
      if (_connectAllProgress.value?.active == true) {
        log("connectAll ignored: already in flight")
        return false
      }
      batchNodes = nodeIds.toSet()
      _connectAllProgress.value =
        ConnectAllProgress(nodeIds.size, 0, 0, active = nodeIds.isNotEmpty())
    }
    log("connectAll start nodes=$nodeIds")
    for (nodeId in nodeIds) startLink(nodeId, Mode.CONNECT_ALL)
    updateProgress()
    return true
  }

  /** Per-device Connect. Returns false if the link was already up or on its way up. */
  fun connect(nodeId: Long): Boolean = startLink(nodeId, Mode.SINGLE)

  /** Stops every node of the current Connect All that has not reached LIVE. */
  fun cancelConnectAll() {
    val nodes =
      synchronized(stateLock) {
        if (_connectAllProgress.value?.active != true) return
        batchNodes
      }
    log("connectAll cancel nodes=$nodes")
    for (nodeId in nodes) {
      if (linkState(nodeId).phase != LinkPhase.LIVE) teardownNode(nodeId, "cancelled")
    }
    synchronized(stateLock) {
      _connectAllProgress.value = _connectAllProgress.value?.copy(active = false, cancelled = true)
    }
  }

  /** A successful command or read proves the node is reachable right now. */
  fun noteReachable(nodeId: Long) {
    val phase =
      synchronized(stateLock) {
        val current = _linkStates.value[nodeId] ?: LinkState()
        put(nodeId, current.copy(lastHeardMs = clock()))
        current.phase
      }
    if (phase == LinkPhase.OFFLINE || phase == LinkPhase.ASLEEP || phase == LinkPhase.UNREACHABLE) {
      log("link node=$nodeId reachable via command while $phase -> reconnect")
      startLink(nodeId, Mode.SINGLE)
    }
  }

  /**
   * Called on the main thread when commissioning finishes. Usually no lease is held yet (the
   * commissioning screen replaced the Device Center), so the node is remembered and dialed by the
   * next Center entry, even if it is a LIT ICD.
   */
  fun onNodeCommissioned(nodeId: Long) {
    log("node commissioned node=$nodeId centerActive=${isCenterActive()}")
    pendingFreshNodes.add(nodeId)
    refreshRegisteredIcdPeers()
    startLink(nodeId, Mode.SINGLE)
  }

  fun onNodeRemoved(nodeId: Long) {
    teardownNode(nodeId, "node removed")
    synchronized(stateLock) {
      _linkStates.value = _linkStates.value - nodeId
      batchNodes = batchNodes - nodeId
    }
    links.remove(nodeId)
    descriptorInFlight.remove(nodeId)
    pendingFreshNodes.remove(nodeId)
  }

  fun ensureDescriptorDiscovered(nodeId: Long, devicePtr: Long? = null) {
    val reg = registryOrNull() ?: return
    val node = reg.getNode(nodeId) ?: return
    if (node.descriptorVerified) return
    if (!descriptorInFlight.add(nodeId)) return
    val link = links.getOrPut(nodeId) { NodeLink() }
    val myGen = link.gen.get()
    scope.launch {
      try {
        // Inside try: an early return must still clear descriptorInFlight in finally.
        if (link.gen.get() != myGen) return@launch
        val ptr =
          devicePtr
            ?: caseSemaphore.withPermit {
              if (link.gen.get() != myGen) return@launch
              transport().connect(nodeId)
            }
        val report =
          try {
            transport().readDescriptor(ptr, nodeId)
          } catch (e: CancellationException) {
            throw e
          } catch (e: Exception) {
            null
          }
        if (report != null && link.gen.get() == myGen) {
          val verified = reg.applyDescriptorReport(appContext, nodeId, report)
          log("descriptor node=$nodeId gen=$myGen verified=$verified")
          if (verified && reg.getNode(nodeId)?.isIcd() == true) {
            refreshSubscriptionPaths(nodeId)
          }
        }
      } catch (e: CancellationException) {
        throw e
      } catch (e: Exception) {
        log("descriptor discovery failed node=$nodeId gen=$myGen: ${e.message}")
      } finally {
        descriptorInFlight.remove(nodeId)
      }
    }
  }

  /**
   * Called when a node's descriptor is verified outside runLink. If the node was already subscribed
   * (with root-only paths, because the first Descriptor read failed), resubscribe so that the new
   * endpoints and clusters start reporting.
   */
  private fun refreshSubscriptionPaths(nodeId: Long) {
    val phase = linkState(nodeId).phase
    if (phase == LinkPhase.PRIMING || phase == LinkPhase.LIVE || phase == LinkPhase.RECOVERING) {
      log("descriptor verified after subscribe node=$nodeId phase=$phase -> resubscribe")
      startLink(nodeId, Mode.REFRESH_PATHS)
    }
  }

  // ---------------------------------------------------------------------------------------------
  // Link state machine
  // ---------------------------------------------------------------------------------------------

  private fun CommissionedNode.isIcd(): Boolean =
    getAllSupportedClusterIds().contains(CLUSTER_ICD_MANAGEMENT)

  /**
   * True if [this] node currently operates as a LIT ICD. Pure in-memory: it runs on the main thread
   * and on CHIP callbacks, so it must never call into the controller. The LITS FeatureMap bit only
   * means LIT is supported; a LITS-capable device with no registered client stays in SIT.
   */
  private fun CommissionedNode.isLitIcd(): Boolean {
    if (!isIcd()) return false
    val opMode =
      HomeStateCache.get(nodeId, 0, CLUSTER_ICD_MANAGEMENT, ATTR_ICD_OPERATING_MODE)?.let {
        (it.value as? Number)?.toInt()
      }
    if (opMode != null) return opMode == ICD_OPERATING_MODE_LIT
    return nodeId in registeredIcdPeers
  }

  private fun startLink(nodeId: Long, mode: Mode): Boolean {
    val reg = registryOrNull() ?: return false
    val node = reg.getNode(nodeId) ?: return false
    val link = links.getOrPut(nodeId) { NodeLink() }
    val isIcd = node.isIcd()
    val isLit = node.isLitIcd()
    var myGen = -1L
    var kept: LinkState? = null
    var before: LinkState? = null
    var nextState: LinkState? = null
    synchronized(stateLock) {
      if (leases.isEmpty()) return false
      val current = _linkStates.value[nodeId] ?: LinkState()
      if (
        mode == Mode.ENTER &&
          isLit &&
          nodeId !in pendingFreshNodes &&
          (current.phase == LinkPhase.NOT_CONNECTED ||
            current.phase == LinkPhase.ASLEEP ||
            current.phase == LinkPhase.OFFLINE)
      ) {
        put(nodeId, current.copy(phase = LinkPhase.ASLEEP, isIcd = true, isLit = true))
        return false
      }
      val inFlight =
        current.phase == LinkPhase.CONNECTING ||
          current.phase == LinkPhase.PRIMING ||
          current.phase == LinkPhase.WAKING
      val keep =
        when (mode) {
          Mode.ENTER ->
            inFlight || current.phase == LinkPhase.LIVE || current.phase == LinkPhase.RECOVERING
          Mode.CONNECT_ALL ->
            current.phase == LinkPhase.CONNECTING || current.phase == LinkPhase.PRIMING
          Mode.SINGLE -> inFlight || current.phase == LinkPhase.LIVE
          Mode.REFRESH_PATHS ->
            current.phase == LinkPhase.CONNECTING || current.phase == LinkPhase.WAKING
          Mode.CHECK_IN ->
            inFlight || current.phase == LinkPhase.LIVE || current.phase == LinkPhase.RECOVERING
        }
      if (keep) {
        kept = current
        return@synchronized
      }
      myGen = link.gen.incrementAndGet()
      link.job?.cancel()
      link.resubscribeAttempts = 0
      pendingFreshNodes.remove(nodeId)
      before = current
      val state =
        LinkState(
          phase = if (mode == Mode.CHECK_IN) LinkPhase.WAKING else LinkPhase.CONNECTING,
          generation = myGen,
          sinceMs = clock(),
          lastHeardMs = current.lastHeardMs,
          isIcd = isIcd,
          checkInCounter = if (mode == Mode.CHECK_IN) current.checkInCounter else null,
          isLit = isLit,
        )
      nextState = state
      put(nodeId, state)
      link.job = scope.launch { runLink(nodeId, link, myGen) }
    }
    kept?.let {
      log("link node=$nodeId keep ${it.phase} gen=${it.generation} mode=$mode")
      return false
    }
    val prev = before
    val next = nextState
    if (prev != null && next != null) {
      resolveCheckInOutcome(nodeId, prev, next, detail = "superseded by $mode")
    }
    val startPhase = if (mode == Mode.CHECK_IN) "WAKING" else "CONNECTING"
    log("link node=$nodeId $startPhase gen=$myGen mode=$mode")
    updateProgress()
    return true
  }

  private suspend fun runLink(nodeId: Long, link: NodeLink, myGen: Long) {
    // 1. Shut down whatever an older generation (or another screen) left behind, so that at most
    //    one ReadClient exists for this node.
    link.mutex.withLock {
      if (link.gen.get() != myGen) return
      transport().shutdown(nodeId)
      log("sub_shutdown node=$nodeId gen=$myGen")
    }

    // 2. CASE, retried by the app (no ReadClient exists yet).
    var attempt = 0
    var devicePtr: Long
    while (true) {
      registryOrNull()?.getNode(nodeId) ?: return
      try {
        devicePtr = caseSemaphore.withPermit { transport().connect(nodeId) }
        break
      } catch (e: CancellationException) {
        throw e
      } catch (e: Exception) {
        val backoff = min(CASE_RETRY_BASE_MS shl min(attempt, 5), CASE_RETRY_MAX_MS)
        var finalPhase: LinkPhase? = null
        var lit = false
        signal(nodeId, myGen, "caseFailure") {
          lit = it.isLit
          val next =
            if (it.phase == LinkPhase.WAKING) {
              it.copy(phase = LinkPhase.UNREACHABLE, reason = "Couldn't reach")
            } else if (it.phase == LinkPhase.UNREACHABLE) {
              it.copy(reason = "Couldn't reach")
            } else {
              it.copy(
                phase = if (it.isLit) LinkPhase.ASLEEP else LinkPhase.OFFLINE,
                reason = "Couldn't reach",
              )
            }
          finalPhase = next.phase
          next
        }
        log("link node=$nodeId CASE failed gen=$myGen attempt=$attempt retry_in_ms=$backoff")
        // Only a LIT ICD stops here: it sleeps by design and sends a Check-In when it wakes. A SIT
        // ICD is pollable and keeps retrying like a mains node.
        if (lit || finalPhase == LinkPhase.UNREACHABLE) {
          return
        }
        attempt++
        delay(backoff)
        if (link.gen.get() != myGen) return
      }
    }

    // 3. Descriptor first (a read, not a subscription) so that one subscribe carries full paths.
    var node = registryOrNull()?.getNode(nodeId) ?: return
    if (!node.descriptorVerified) {
      val report =
        try {
          transport().readDescriptor(devicePtr, nodeId)
        } catch (e: CancellationException) {
          throw e
        } catch (e: Exception) {
          null
        }
      if (link.gen.get() != myGen) return
      val verified =
        report != null &&
          registryOrNull()?.applyDescriptorReport(appContext, nodeId, report) == true
      log("descriptor node=$nodeId gen=$myGen verified=$verified")
      node = registryOrNull()?.getNode(nodeId) ?: return
    }

    // 4. One subscription, issued under the node mutex after re-checking the generation so that a
    //    teardown that raced with CASE can never leave an orphan ReadClient.
    link.mutex.withLock {
      if (link.gen.get() != myGen || !isCenterActive()) {
        log("drop stale device pointer node=$nodeId gen=$myGen current=${link.gen.get()}")
        return
      }
      // Re-read the node: the descriptor may have been verified by ensureDescriptorDiscovered while
      // this link was doing CASE, and the subscription must carry the full path set.
      node = registryOrNull()?.getNode(nodeId) ?: return
      val icd = node.isIcd()
      val (paths, eventPaths) = buildPaths(node)
      val maxInterval = if (icd) MAX_INTERVAL_ICD_S else MAX_INTERVAL_MAINS_S
      val eventMin = HomeStateCache.getEventMin(nodeId)?.plus(1)
      signal(nodeId, myGen, "caseSuccess") {
        val nextPhase =
          if (it.phase == LinkPhase.WAKING || it.phase == LinkPhase.UNREACHABLE) LinkPhase.WAKING
          else LinkPhase.PRIMING
        it.copy(phase = nextPhase, isIcd = icd, lastHeardMs = clock(), reason = null)
      }
      log(
        "link node=$nodeId PRIMING gen=$myGen paths=${paths.size} eventPaths=${eventPaths.size} " +
          "maxInterval=$maxInterval eventMin=$eventMin"
      )
      try {
        transport()
          .subscribe(
            SubscribeRequest(
              nodeId = nodeId,
              devicePtr = devicePtr,
              attributePaths = paths,
              eventPaths = eventPaths,
              minInterval = 0,
              maxInterval = maxInterval,
              keepSubscriptions = false,
              eventMin = eventMin,
              imTimeoutMs = if (icd) PRIME_TIMEOUT_ICD_MS else PRIME_TIMEOUT_MAINS_MS,
              established = establishedCallback(nodeId, myGen),
              resubscription = resubscriptionCallback(nodeId, myGen),
              report = reportCallback(nodeId, myGen),
            )
          )
      } catch (e: Exception) {
        logE("subscribe failed node=$nodeId gen=$myGen", e)
        signal(nodeId, myGen, "subscribeFailure") {
          if (it.phase == LinkPhase.WAKING || it.phase == LinkPhase.UNREACHABLE) {
            it.copy(phase = LinkPhase.UNREACHABLE, reason = "Subscribe failed")
          } else {
            it.copy(phase = LinkPhase.OFFLINE, reason = "Subscribe failed")
          }
        }
      }
    }
  }

  private fun establishedCallback(nodeId: Long, gen: Long) =
    SubscriptionEstablishedCallback { subscriptionId ->
      links[nodeId]?.let { if (it.gen.get() == gen) it.resubscribeAttempts = 0 }
      signal(nodeId, gen, "onSubscriptionEstablished") {
        it.copy(
          phase = LinkPhase.LIVE,
          lastHeardMs = clock(),
          reason = null,
          subscriptionId = subscriptionId,
        )
      }
    }

  private fun resubscriptionCallback(nodeId: Long, gen: Long) =
    ResubscriptionAttemptCallback { _, nextResubscribeIntervalMsec ->
      val link = links[nodeId]
      if (link == null || link.gen.get() != gen) {
        logDrop(nodeId, "onResubscriptionAttempt", gen)
        return@ResubscriptionAttemptCallback
      }
      link.resubscribeAttempts++
      val attempts = link.resubscribeAttempts
      signal(nodeId, gen, "onResubscriptionAttempt") {
        val phase =
          when {
            it.isLit -> LinkPhase.ASLEEP
            attempts >= MAINS_RESUBSCRIBE_ATTEMPTS_BEFORE_OFFLINE -> LinkPhase.OFFLINE
            else -> LinkPhase.RECOVERING
          }
        it.copy(phase = phase, reason = "Retrying in ${nextResubscribeIntervalMsec / 1000}s")
      }
    }

  private fun reportCallback(nodeId: Long, gen: Long) =
    object : ReportCallback {
      override fun onError(
        attributePath: ChipAttributePath?,
        eventPath: ChipEventPath?,
        ex: Exception,
      ) {
        if (attributePath != null || eventPath != null) {
          log("path error node=$nodeId gen=$gen path=${attributePath ?: eventPath}: ${ex.message}")
          return
        }
        // The SDK gave up resubscribing. Never shut down here: the ReadClient is already gone.
        signal(nodeId, gen, "onError") {
          val nextPhase =
            if (
              it.isLit && it.phase != LinkPhase.UNREACHABLE && it.phase != LinkPhase.NOT_CONNECTED
            ) {
              LinkPhase.ASLEEP
            } else {
              LinkPhase.OFFLINE
            }
          it.copy(phase = nextPhase, reason = ex.message?.take(80) ?: "Subscription error")
        }
      }

      override fun onReport(nodeState: NodeState) {
        if (!isCurrent(nodeId, gen)) {
          logDrop(nodeId, "onReport", gen)
          return
        }
        signal(nodeId, gen, "onReport") { it.copy(lastHeardMs = clock()) }
        processNodeStateReport(nodeId, nodeState)
      }

      override fun onDone() {
        // onDone only means the ReadClient is gone. It never means the node is online.
        signal(nodeId, gen, "onDone") {
          if (it.phase == LinkPhase.NOT_CONNECTED) {
            it
          } else {
            val nextPhase =
              if (it.isLit && it.phase != LinkPhase.UNREACHABLE) {
                LinkPhase.ASLEEP
              } else {
                LinkPhase.OFFLINE
              }
            it.copy(phase = nextPhase, reason = it.reason ?: "Subscription ended")
          }
        }
      }
    }

  private fun isCurrent(nodeId: Long, gen: Long): Boolean = links[nodeId]?.gen?.get() == gen

  /** Applies [transform] if [gen] is still current. Returns false if the callback was dropped. */
  private fun signal(
    nodeId: Long,
    gen: Long,
    kind: String,
    transform: (LinkState) -> LinkState,
  ): Boolean {
    var before = LinkState()
    var after = before
    var droppedAt: Long? = null
    synchronized(stateLock) {
      val link = links[nodeId]
      if (link == null || link.gen.get() != gen) {
        droppedAt = link?.gen?.get() ?: -1L
      } else {
        before = _linkStates.value[nodeId] ?: LinkState()
        var next = transform(before)
        if (next.phase != before.phase) next = next.copy(sinceMs = clock())
        put(nodeId, next)
        after = next
      }
    }
    droppedAt?.let {
      log("drop stale callback node=$nodeId kind=$kind gen=$gen current=$it")
      return false
    }
    resolveCheckInOutcome(nodeId, before, after)
    if (after.phase != before.phase) {
      val extra =
        when (after.phase) {
          LinkPhase.LIVE -> " subId=${after.subscriptionId}"
          else -> after.reason?.let { " reason=$it" } ?: ""
        }
      if (after.phase != LinkPhase.PRIMING) {
        log("link node=$nodeId ${after.phase} gen=$gen via=$kind$extra")
      }
      updateProgress()
    }
    return true
  }

  private fun teardownAll(reason: String) {
    val nodeIds =
      (links.keys + (registryOrNull()?.getAllNodes()?.map { it.nodeId } ?: emptyList()))
        .toSortedSet()
    for (nodeId in nodeIds) teardownNode(nodeId, reason)
    synchronized(stateLock) {
      batchNodes = emptySet()
      _connectAllProgress.value = null
    }
  }

  private fun teardownNode(nodeId: Long, reason: String) {
    val link = links.getOrPut(nodeId) { NodeLink() }
    val myGen: Long
    val before: LinkState
    val after: LinkState
    synchronized(stateLock) {
      myGen = link.gen.incrementAndGet()
      link.job?.cancel()
      link.job = null
      before = _linkStates.value[nodeId] ?: LinkState()
      val heard = if (before.phase == LinkPhase.LIVE) clock() else before.lastHeardMs
      after =
        LinkState(
          phase = LinkPhase.NOT_CONNECTED,
          generation = myGen,
          sinceMs = clock(),
          lastHeardMs = heard,
          reason = if (reason == "cancelled") "Cancelled" else null,
          isIcd = before.isIcd,
          isLit = before.isLit,
        )
      put(nodeId, after)
    }
    resolveCheckInOutcome(nodeId, before, after, detail = reason)
    log("link node=$nodeId NOT_CONNECTED gen=$myGen reason=$reason")
    val transport = transportOrNull() ?: return
    scope.launch {
      link.mutex.withLock {
        if (link.gen.get() != myGen) return@withLock
        transport.shutdown(nodeId)
        log("sub_shutdown node=$nodeId gen=$myGen")
      }
    }
  }

  /**
   * The one place a Check-In wake is resolved. Whatever moves a link out of WAKING (a callback or a
   * teardown) records how the wake ended, so a Check-In row never stays WAKING.
   */
  private fun resolveCheckInOutcome(
    nodeId: Long,
    before: LinkState,
    after: LinkState,
    detail: String? = null,
  ) {
    val counter = before.checkInCounter ?: return
    if (before.phase != LinkPhase.WAKING || after.phase == LinkPhase.WAKING) return
    val outcome =
      when (after.phase) {
        LinkPhase.LIVE -> CheckInOutcome.LIVE
        LinkPhase.NOT_CONNECTED,
        LinkPhase.CONNECTING -> CheckInOutcome.CANCELLED
        else -> CheckInOutcome.FAILED
      }
    val why = if (outcome == CheckInOutcome.LIVE) null else detail ?: after.reason
    HomeStateCache.updateCheckInOutcome(nodeId, counter, outcome, why)
  }

  private fun updateProgress() {
    var finished: ConnectAllProgress? = null
    synchronized(stateLock) {
      val progress = _connectAllProgress.value ?: return
      if (!progress.active) return
      val states = _linkStates.value
      val nodes = batchNodes
      val settled =
        nodes.count {
          val phase = states[it]?.phase ?: LinkPhase.NOT_CONNECTED
          phase != LinkPhase.CONNECTING && phase != LinkPhase.PRIMING && phase != LinkPhase.WAKING
        }
      val live = nodes.count { states[it]?.phase == LinkPhase.LIVE }
      val next =
        progress.copy(
          total = nodes.size,
          settled = settled,
          live = live,
          active = settled < nodes.size,
        )
      _connectAllProgress.value = next
      if (!next.active) finished = next
    }
    finished?.let { log("connectAll done live=${it.live}/${it.total}") }
  }

  private fun put(nodeId: Long, state: LinkState) {
    _linkStates.value = _linkStates.value + (nodeId to state)
  }

  // ---------------------------------------------------------------------------------------------
  // Paths and reports
  // ---------------------------------------------------------------------------------------------

  internal fun buildPaths(
    node: CommissionedNode
  ): Pair<List<ChipAttributePath>, List<ChipEventPath>> {
    val supportedClusters = node.getAllSupportedClusterIds()
    val paths = mutableListOf<ChipAttributePath>()
    val eventPaths = mutableListOf<ChipEventPath>()
    if (supportedClusters.isEmpty() || !node.isIcd()) {
      paths.add(
        ChipAttributePath.newInstance(
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
        )
      )
      eventPaths.add(
        ChipEventPath.newInstance(
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
          ChipPathId.forWildcard(),
          false,
        )
      )
      return paths to eventPaths
    }
    val extraClusters = mutableListOf<Long>()
    if (supportedClusters.contains(MatterClusterMetaRegistry.CLUSTER_POWER_SOURCE)) {
      extraClusters.add(MatterClusterMetaRegistry.CLUSTER_POWER_SOURCE)
    }
    if (supportedClusters.contains(MatterClusterMetaRegistry.CLUSTER_SWITCH)) {
      extraClusters.add(MatterClusterMetaRegistry.CLUSTER_SWITCH)
    }
    if (supportedClusters.contains(CLUSTER_ICD_MANAGEMENT)) {
      extraClusters.add(CLUSTER_ICD_MANAGEMENT)
    }
    val targetClusters =
      (DEFAULT_SUBSCRIBED_CLUSTERS.filter { it in supportedClusters } +
          MatterClusterMetaRegistry.CLUSTER_BASIC_INFORMATION +
          extraClusters)
        .distinct()
    for (clusterId in targetClusters) {
      paths.add(
        ChipAttributePath.newInstance(
          ChipPathId.forWildcard(),
          ChipPathId.forId(clusterId),
          ChipPathId.forWildcard(),
        )
      )
    }
    paths.add(
      ChipAttributePath.newInstance(
        ChipPathId.forWildcard(),
        ChipPathId.forId(0x001DL),
        ChipPathId.forWildcard(),
      )
    )
    eventPaths.add(
      ChipEventPath.newInstance(
        ChipPathId.forWildcard(),
        ChipPathId.forWildcard(),
        ChipPathId.forWildcard(),
        false,
      )
    )
    return paths to eventPaths
  }

  private fun decodeSwitchEvent(eventId: Long, eventState: EventState): Pair<Int, Int> {
    when (val obj = eventState.value) {
      is ChipEventStructs.SwitchClusterSwitchLatchedEvent -> return obj.newPosition to 1
      is ChipEventStructs.SwitchClusterInitialPressEvent -> return obj.newPosition to 1
      is ChipEventStructs.SwitchClusterLongPressEvent -> return obj.newPosition to 1
      is ChipEventStructs.SwitchClusterShortReleaseEvent -> return obj.previousPosition to 1
      is ChipEventStructs.SwitchClusterLongReleaseEvent -> return obj.previousPosition to 1
      is ChipEventStructs.SwitchClusterMultiPressOngoingEvent ->
        return obj.newPosition to obj.currentNumberOfPressesCounted
      is ChipEventStructs.SwitchClusterMultiPressCompleteEvent ->
        return obj.previousPosition to obj.totalNumberOfPressesCounted
    }
    val tlv = eventState.tlv?.takeIf { it.isNotEmpty() } ?: return 1 to 1
    return decodeSwitchEventFromTlv(eventId, tlv)
  }

  private fun decodeSwitchEventFromTlv(eventId: Long, tlv: ByteArray): Pair<Int, Int> {
    return try {
      val reader = TlvReader(tlv)
      reader.enterStructure(AnonymousTag)
      val pos = reader.getUByte(ContextSpecificTag(0)).toInt()
      val isMultiPress = eventId == 0x05L || eventId == 0x06L
      val count =
        if (isMultiPress && !reader.isEndOfContainer()) {
          reader.getUByte(ContextSpecificTag(1)).toInt()
        } else {
          1
        }
      reader.exitContainer()
      pos to count
    } catch (_: Throwable) {
      1 to 1
    }
  }

  /**
   * Stamps an event with the device's own epoch timestamp when it has one, so that events replayed
   * by a new subscription keep their real time instead of looking like they just happened.
   */
  internal fun eventTimestampMs(eventState: EventState, nowMs: Long): Long {
    if (eventState.timestampType != EventState.MILLIS_SINCE_EPOCH) return nowMs
    val ts = eventState.timestampValue
    val plausibleFloor = 1_577_836_800_000L // 2020-01-01
    return if (ts in plausibleFloor..(nowMs + 60_000L)) ts else nowMs
  }

  internal fun processNodeStateReport(nodeId: Long, nodeState: NodeState) {
    val now = clock()
    val registry = registryOrNull()
    val beforeNode = registry?.getNode(nodeId)
    if (registry != null && beforeNode == null) {
      return
    }

    if (registry != null) {
      val ep0 = nodeState.endpointStates[0]
      val partsListAttr = ep0?.clusterStates?.get(0x001DL)?.attributeStates?.get(0x0003L)
      var complete = false
      if (partsListAttr != null) {
        val partsListValue = extractAttributeValue(0x001DL, 0x0003L, partsListAttr)
        if (partsListValue is List<*>) {
          var allServersPresent = ep0.clusterStates[0x001DL]?.attributeStates?.get(0x0001L) != null
          for (ep in partsListValue) {
            val epNum = (ep as Number).toInt()
            if (
              nodeState.endpointStates[epNum]
                ?.clusterStates
                ?.get(0x001DL)
                ?.attributeStates
                ?.get(0x0001L) == null
            ) {
              allServersPresent = false
              break
            }
          }
          if (allServersPresent) {
            complete = true
          }
        }
      }

      if (complete) {
        val applied = registry.applyDescriptorReport(null, nodeId, nodeState)
        if (applied && beforeNode?.isIcd() == true) {
          val afterNode = registry.getNode(nodeId)
          if (
            afterNode != null &&
              afterNode.getAllSupportedClusterIds() != beforeNode.getAllSupportedClusterIds()
          ) {
            refreshSubscriptionPaths(nodeId)
          }
        }
      } else if (beforeNode?.descriptorVerified != true) {
        ensureDescriptorDiscovered(nodeId)
      }
    }
    for ((epId, endpointState) in nodeState.endpointStates) {
      for ((clusterId, clusterState) in endpointState.clusterStates) {
        recordClusterAttributes(nodeId, epId, clusterId, clusterState, now, registry)
        recordEvents(nodeId, epId, clusterId, clusterState, now)
      }
      for (clusterId in endpointState.clusterStates.keys) {
        registry?.recordDiscoveredEndpointCluster(null, nodeId, epId, clusterId, null, null)
      }
    }
    val afterNode = registry?.getNode(nodeId)
    if (registry != null && afterNode != null && afterNode != beforeNode) {
      appContext?.let { ctx -> scope.launch { registry.saveFabricToPreferences(ctx) } }
    }
  }

  private fun recordClusterAttributes(
    nodeId: Long,
    epId: Int,
    clusterId: Long,
    clusterState: chip.devicecontroller.model.ClusterState,
    now: Long,
    registry: CommissionedNodeRegistry?,
  ) {
    for ((attrId, attributeState) in clusterState.attributeStates) {
      val value = extractAttributeValue(clusterId, attrId, attributeState)
      HomeStateCache.put(nodeId, epId, clusterId, attrId, value, now, Source.SUBSCRIPTION_REPORT)
      registry?.recordDiscoveredEndpointCluster(null, nodeId, epId, clusterId, attrId, value)
    }
  }

  private fun extractAttributeValue(
    clusterId: Long,
    attrId: Long,
    attributeState: chip.devicecontroller.model.AttributeState,
  ): Any? {
    attributeState.value?.let {
      return it
    }
    val tlv = attributeState.tlv ?: return null
    val meta =
      MatterClusterMetaRegistry.getCluster(clusterId)?.attributes?.get(attrId) ?: return null
    return UniversalTlvEncoder.decodeAttributeReport(meta, tlv).rawValue
  }

  private fun recordEvents(
    nodeId: Long,
    epId: Int,
    clusterId: Long,
    clusterState: chip.devicecontroller.model.ClusterState,
    now: Long,
  ) {
    for ((eventId, eventList) in clusterState.eventStates) {
      for (eventState in eventList) {
        val eventName =
          MatterClusterMetaRegistry.getCluster(clusterId)?.events?.get(eventId)?.name
            ?: "Event(0x${eventId.toString(16)})"
        val timestamp = eventTimestampMs(eventState, now)

        if (clusterId == MatterClusterMetaRegistry.CLUSTER_SWITCH) {
          val (position, pressCount) = decodeSwitchEvent(eventId, eventState)
          HomeStateCache.putSwitchEvent(
            nodeId = nodeId,
            endpointId = epId,
            eventId = eventId,
            position = position,
            pressCount = pressCount,
            timestampMs = timestamp,
            eventNumber = eventState.eventNumber,
          )
          log(
            "switch_event node=$nodeId ep=$epId eventId=0x${eventId.toString(16)} " +
              "pos=$position count=$pressCount num=${eventState.eventNumber}"
          )
        }

        val priority = eventState.priorityLevel
        HomeStateCache.putEvent(
          nodeId = nodeId,
          endpointId = epId,
          clusterId = clusterId,
          eventId = eventId,
          eventName = eventName,
          eventNumber = eventState.eventNumber,
          priority = priority,
          timestampMs = timestamp,
          payload = eventState.value,
        )
      }
    }
  }

  // ---------------------------------------------------------------------------------------------
  // Transport
  // ---------------------------------------------------------------------------------------------

  private fun transport(): CenterTransport =
    transportOrNull() ?: throw IllegalStateException("CenterConnectionManager has no context")

  private fun transportOrNull(): CenterTransport? {
    transportOverride?.let {
      return it
    }
    chipTransport?.let {
      return it
    }
    val ctx = appContext ?: return null
    return ChipTransport(ctx).also { chipTransport = it }
  }

  private class ChipTransport(private val context: Context) : CenterTransport {
    override suspend fun connect(nodeId: Long): Long =
      ChipClient.getConnectedDevicePointer(context, nodeId)

    override suspend fun readDescriptor(devicePtr: Long, nodeId: Long): NodeState? =
      suspendCancellableCoroutine { cont ->
        ChipClient.getDeviceController(context)
          .readPath(
            descriptorCallback(cont),
            devicePtr,
            DESCRIPTOR_PATHS,
            emptyList<ChipEventPath>(),
            false,
            PRIME_TIMEOUT_MAINS_MS,
          )
      }

    private fun descriptorCallback(
      cont: kotlinx.coroutines.CancellableContinuation<NodeState?>
    ): ReportCallback =
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          ex: Exception,
        ) {
          if (attributePath == null && eventPath == null && cont.isActive) cont.resume(null)
        }

        override fun onReport(nodeState: NodeState) {
          if (cont.isActive) cont.resume(nodeState)
        }

        override fun onDone() {
          if (cont.isActive) cont.resume(null)
        }
      }

    override fun subscribe(request: SubscribeRequest) {
      ChipClient.getDeviceController(context)
        .subscribeToPath(
          request.established,
          request.resubscription,
          request.report,
          request.devicePtr,
          request.attributePaths,
          request.eventPaths,
          request.minInterval,
          request.maxInterval,
          request.keepSubscriptions,
          true,
          request.imTimeoutMs,
          request.eventMin,
        )
    }

    override fun shutdown(nodeId: Long) {
      try {
        val controller = ChipClient.getDeviceController(context)
        controller.shutdownSubscriptions(controller.fabricIndex, nodeId)
      } catch (e: Exception) {
        logE("shutdownSubscriptions failed node=$nodeId", e)
      }
    }

    companion object {
      private val DESCRIPTOR_PATHS =
        listOf(
          ChipAttributePath.newInstance(
            ChipPathId.forWildcard(),
            ChipPathId.forId(0x001DL),
            ChipPathId.forId(0x0000L), // DeviceTypeList
          ),
          ChipAttributePath.newInstance(
            ChipPathId.forWildcard(),
            ChipPathId.forId(0x001DL),
            ChipPathId.forId(0x0001L), // ServerList
          ),
          ChipAttributePath.newInstance(
            ChipPathId.forId(0L),
            ChipPathId.forId(0x001DL),
            ChipPathId.forId(0x0003L), // PartsList
          ),
        )
    }
  }

  // ---------------------------------------------------------------------------------------------
  // Logging (android.util.Log is not available in JVM unit tests)
  // ---------------------------------------------------------------------------------------------

  private fun logDrop(nodeId: Long, kind: String, gen: Long) {
    log("drop stale callback node=$nodeId kind=$kind gen=$gen current=${links[nodeId]?.gen?.get()}")
  }

  private fun log(msg: String) {
    try {
      android.util.Log.i(TAG, msg)
    } catch (e: Throwable) {
      println("INFO: $TAG: $msg")
    }
  }

  private fun logE(msg: String, t: Throwable? = null) {
    try {
      android.util.Log.e(TAG, msg, t)
    } catch (e: Throwable) {
      println("ERROR: $TAG: $msg $t")
    }
  }

  // ---------------------------------------------------------------------------------------------
  // Tests
  // ---------------------------------------------------------------------------------------------

  /** Resets all state and installs test doubles. */
  internal fun resetForTest(
    registry: CommissionedNodeRegistry?,
    transport: CenterTransport?,
    dispatcher: CoroutineDispatcher = Dispatchers.Default,
  ) {
    scope.cancel()
    scope = CoroutineScope(SupervisorJob() + dispatcher)
    caseSemaphore = Semaphore(MAX_PARALLEL_CASE)
    synchronized(stateLock) {
      leases.clear()
      batchNodes = emptySet()
      links.clear()
      descriptorInFlight.clear()
      pendingFreshNodes.clear()
      registeredIcdPeers = emptySet()
      icdClientLookup = defaultIcdClientLookup
      _linkStates.value = emptyMap()
      _connectAllProgress.value = null
      sharedRegistry = registry
      transportOverride = transport
      chipTransport = null
    }
    clock = { System.currentTimeMillis() }
  }

  /** Places [nodeId] in [state] directly (no transport involved). */
  internal fun setLinkStateForTest(nodeId: Long, state: LinkState) {
    synchronized(stateLock) { put(nodeId, state) }
  }
}
