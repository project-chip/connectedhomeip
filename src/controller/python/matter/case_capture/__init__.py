#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Observe CASE handshake messages.

Header capture: inbound Sigma2, Sigma2_Resume and StatusReport. Call Reset() before
triggering the handshake; check `.present` on each slot to see whether the message was
observed.

Handshake metrics: register a function and it is handed the timings for every CASE handshake
that completes, until you remove it. There is nothing to start and nothing to poll:

    def report(metrics):
        print(metrics.peer_node_id,
              metrics.device_discovery_duration_ms,
              metrics.sigma1_sigma2_exchange_duration_ms,
              metrics.sigma3_exchange_duration_ms)

    listener_id = case_capture.AddCASEHandshakeListener(report)
    await dev_ctrl.GetConnectedDevice(node_id, allowPASE=False)
    case_capture.RemoveCASEHandshakeListener(listener_id)

Pass own_thread=True for a listener that does something slow, such as writing to a database or a
spreadsheet, so it cannot hold up the others.
See the CASEHandshakeMetrics class for the phases reported."""

import ctypes
import logging
import queue
import threading
from dataclasses import dataclass

from ..native import GetLibraryHandle, HandleFlags, NativeLibraryHandleMethodArguments, PyChipError


# Mirror of the C struct PychipCaseCapturedHeaders defined in CASECapture.h.
class PyCaseCapturedHeaders(ctypes.Structure):
    _fields_ = [
        ("sessionType", ctypes.c_uint8),
        ("sFlag", ctypes.c_uint8),
        ("dsiz", ctypes.c_uint8),
        ("protocolId", ctypes.c_uint16),
        ("opcode", ctypes.c_uint8),
        ("isInitiator", ctypes.c_uint8),
        ("present", ctypes.c_uint8),
    ]


# Mirror of the C struct PychipCaseCaptureSnapshot defined in CASECapture.h.
class PyCaseCaptureSnapshot(ctypes.Structure):
    _fields_ = [
        ("sigma2", PyCaseCapturedHeaders),
        ("sigma2Resume", PyCaseCapturedHeaders),
        ("statusReport", PyCaseCapturedHeaders),
        ("statusReportParsed", ctypes.c_uint8),
        ("statusReportGeneralCode", ctypes.c_uint16),
        ("statusReportProtocolId", ctypes.c_uint32),
        ("statusReportProtocolCode", ctypes.c_uint16),
    ]


def _GetLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_case_capture_set_observer.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_case_capture_set_observer', PyChipError, [ctypes.c_void_p])
        setter.Set('pychip_case_capture_reset', PyChipError, [])
        setter.Set('pychip_case_capture_get_snapshot', PyChipError, [ctypes.POINTER(PyCaseCaptureSnapshot)])
    return handle


def SetObserver(controller) -> None:
    """Set the CASE capture observer on the given controller's ExchangeManager."""
    handle = _GetLibraryHandle()
    handle.pychip_case_capture_set_observer(controller.devCtrl).raise_on_error()


def Reset() -> None:
    """Clear all captured slots. Call before triggering a CASE handshake whose
    values you intend to read."""
    handle = _GetLibraryHandle()
    handle.pychip_case_capture_reset().raise_on_error()


def GetSnapshot() -> PyCaseCaptureSnapshot:
    """Read the entire current capture state in one call."""
    handle = _GetLibraryHandle()
    snapshot = PyCaseCaptureSnapshot()
    handle.pychip_case_capture_get_snapshot(ctypes.pointer(snapshot)).raise_on_error()
    return snapshot


# --- CASE handshake metrics -------------------------------------------------------------
#
# Timings come from a tracing backend in the native library that timestamps each Sigma
# message as SessionManager sends or receives it, so nothing here depends on log text or
# trace files.

# Must match PYCHIP_CASE_HANDSHAKE_METRICS_PEER_ADDRESS_MAX_LENGTH in CASECapture.h. Records cross
# the boundary one at a time, so this is the only size that has to agree between the two sides.
PEER_TRANSPORT_ADDRESS_MAX_LENGTH = 80

# The native layer records microseconds, because the Sigma phases are around a millisecond and
# millisecond resolution would round most of the detail away. Durations are reported in
# milliseconds, so every raw difference passes through this divisor.
_MICROSECONDS_PER_MILLISECOND = 1000.0

# A StatusReport says success only when both of its codes are zero; see GeneralStatusCode
# and kProtocolCodeSuccess in src/protocols/secure_channel/Constants.h.
STATUS_REPORT_GENERAL_CODE_SUCCESS = 0
STATUS_REPORT_PROTOCOL_CODE_SUCCESS = 0

# Must match the PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_* bits in CASECapture.h.
RECORDED_SIGMA1_SENT = 0x01
RECORDED_SIGMA2_RECEIVED = 0x02
RECORDED_SIGMA3_SENT = 0x04
RECORDED_STATUS_REPORT_RECEIVED = 0x08
RECORDED_SIGMA2_RESUME_RECEIVED = 0x10
RECORDED_DEVICE_DISCOVERY = 0x20
RECORDED_STATUS_REPORT_CODES = 0x40
RECORDED_THIS_NODE_REJECTED_PEER = 0x80


# Mirror of the C struct PychipCASEHandshakeMetricsRecord defined in CASECapture.h.
class PyCASEHandshakeMetricsRecord(ctypes.Structure):
    _fields_ = [
        ("sigma1SentTimestampUs", ctypes.c_uint64),
        ("sigma2ReceivedTimestampUs", ctypes.c_uint64),
        ("sigma3SentTimestampUs", ctypes.c_uint64),
        ("statusReportReceivedTimestampUs", ctypes.c_uint64),
        ("sigma2ResumeReceivedTimestampUs", ctypes.c_uint64),
        ("discoveryStartedTimestampUs", ctypes.c_uint64),
        ("discoveryCompletedTimestampUs", ctypes.c_uint64),
        ("localEphemeralNodeId", ctypes.c_uint64),
        ("peerNodeId", ctypes.c_uint64),
        ("statusReportGeneralCode", ctypes.c_uint16),
        ("statusReportProtocolCode", ctypes.c_uint16),
        ("exchangeId", ctypes.c_uint16),
        ("recordedFields", ctypes.c_uint8),
        ("peerTransportAddress", ctypes.c_char * PEER_TRANSPORT_ADDRESS_MAX_LENGTH),
    ]


@dataclass(frozen=True)
class CASEHandshakeMetrics:
    """One initiator-side CASE handshake.

    Three durations, all in milliseconds and all None when the handshake did not reach
    that point:

        device_discovery_duration_ms          operational discovery, before any Sigma message
        sigma1_sigma2_exchange_duration_ms    Sigma1 out -> Sigma2 in
        sigma3_exchange_duration_ms           Sigma3 out -> StatusReport in
        total_case_handshake_duration_ms      discovery start -> StatusReport in

    The raw microsecond timestamps are kept as fields, so anything else (the local turnaround
    between the two exchanges, an end-to-end total) can be derived from them.

    With several handshakes running at once, peer_node_id says which device each one is
    with, falling back to peer_transport_address when the address came from cache rather than a
    fresh lookup. Both are learned from the peer's first reply, so a handshake that never
    got one reports neither."""

    sigma1_sent_timestamp_us: int | None
    sigma2_received_timestamp_us: int | None
    sigma3_sent_timestamp_us: int | None
    status_report_received_timestamp_us: int | None
    sigma2_resume_received_timestamp_us: int | None
    discovery_started_timestamp_us: int | None
    discovery_completed_timestamp_us: int | None
    status_report_general_code: int | None
    status_report_protocol_code: int | None
    this_node_rejected_peer: bool
    # Identity of the handshake, so concurrent handshakes can be told apart. local_ephemeral_node_id
    # is this node's ephemeral initiator id, which differs per controller.
    exchange_id: int
    local_ephemeral_node_id: int
    # Which peer the handshake is with. See peer_node_id and peer_transport_address below.
    peer_node_id: int
    peer_transport_address: str

    @staticmethod
    def _from_native_record(record: PyCASEHandshakeMetricsRecord) -> "CASEHandshakeMetrics":
        def value(field: str, bit: int) -> int | None:
            return getattr(record, field) if record.recordedFields & bit else None

        return CASEHandshakeMetrics(
            sigma1_sent_timestamp_us=value("sigma1SentTimestampUs", RECORDED_SIGMA1_SENT),
            sigma2_received_timestamp_us=value("sigma2ReceivedTimestampUs", RECORDED_SIGMA2_RECEIVED),
            sigma3_sent_timestamp_us=value("sigma3SentTimestampUs", RECORDED_SIGMA3_SENT),
            status_report_received_timestamp_us=value("statusReportReceivedTimestampUs", RECORDED_STATUS_REPORT_RECEIVED),
            sigma2_resume_received_timestamp_us=value("sigma2ResumeReceivedTimestampUs", RECORDED_SIGMA2_RESUME_RECEIVED),
            discovery_started_timestamp_us=value("discoveryStartedTimestampUs", RECORDED_DEVICE_DISCOVERY),
            discovery_completed_timestamp_us=value("discoveryCompletedTimestampUs", RECORDED_DEVICE_DISCOVERY),
            status_report_general_code=value("statusReportGeneralCode", RECORDED_STATUS_REPORT_CODES),
            status_report_protocol_code=value("statusReportProtocolCode", RECORDED_STATUS_REPORT_CODES),
            this_node_rejected_peer=bool(record.recordedFields & RECORDED_THIS_NODE_REJECTED_PEER),
            exchange_id=record.exchangeId,
            local_ephemeral_node_id=record.localEphemeralNodeId,
            peer_node_id=record.peerNodeId,
            peer_transport_address=record.peerTransportAddress.decode("utf-8", errors="replace"),
        )

    @staticmethod
    def _duration_between_ms(start: int | None, end: int | None) -> float | None:
        if start is None or end is None:
            return None
        return (end - start) / _MICROSECONDS_PER_MILLISECOND

    @property
    def device_discovery_duration_ms(self) -> float | None:
        """Operational discovery: address lookup started -> resolution done.

        Matched to this handshake by the peer's address, so it stays correct when several
        handshakes are in flight. None when the connect reused a cached address and no lookup
        ran, or when the peer never replied and so was never identified."""
        return self._duration_between_ms(self.discovery_started_timestamp_us, self.discovery_completed_timestamp_us)

    @property
    def sigma1_sigma2_exchange_duration_ms(self) -> float | None:
        """The Sigma1/Sigma2 exchange: Sigma1 out -> Sigma2 in.

        A full round trip, so it includes the peer's Sigma1 verification and Sigma2
        generation as well as the network. It does not include this node's verification
        of Sigma2, which happens after Sigma2 arrives.

        The clock starts when Sigma1 is first sent, so if MRP had to retransmit, the retry
        wait is part of this figure."""
        return self._duration_between_ms(self.sigma1_sent_timestamp_us, self.sigma2_received_timestamp_us)

    @property
    def sigma3_exchange_duration_ms(self) -> float | None:
        """The Sigma3/StatusReport exchange: Sigma3 out -> StatusReport in.

        A full round trip, so it includes the peer's Sigma3 verification. The
        StatusReport is a generic Secure Channel message rather than a Sigma one, but it
        is the only signal that Sigma3 was accepted.

        As with the Sigma1/Sigma2 exchange, any MRP retry wait is included."""
        return self._duration_between_ms(self.sigma3_sent_timestamp_us, self.status_report_received_timestamp_us)

    @property
    def total_case_handshake_duration_ms(self) -> float | None:
        """Everything observed, from the first mark to the last: discovery start through to
        the StatusReport closing Sigma3.

        Wider than the three phases summed, because it also covers the gap between
        discovery finishing and Sigma1 going out, and this node's own Sigma2 verification
        between the two exchanges.

        Falls back to starting at Sigma1 when no discovery ran, so it always reports
        whatever was actually measured. Runs a few milliseconds short of the wall clock
        around GetConnectedDevice, which additionally covers session setup before the
        lookup and session activation after the handshake."""
        start = self.discovery_started_timestamp_us if self.discovery_started_timestamp_us is not None else self.sigma1_sent_timestamp_us
        return self._duration_between_ms(start, self.status_report_received_timestamp_us)

    @property
    def used_session_resumption(self) -> bool:
        """True if the peer answered Sigma1 with Sigma2_Resume, in which case no Sigma2
        or Sigma3 was exchanged and neither exchange duration is available."""
        return self.sigma2_resume_received_timestamp_us is not None

    @property
    def all_messages_exchanged(self) -> bool:
        """True if a full Sigma1/Sigma2/Sigma3 handshake ran to its StatusReport.

        Structural only: it says the four messages were exchanged, not that the peer
        accepted Sigma3. See success for that."""
        return all(value is not None for value in (
            self.sigma1_sent_timestamp_us, self.sigma2_received_timestamp_us,
            self.sigma3_sent_timestamp_us, self.status_report_received_timestamp_us))

    @property
    def _status_report_indicates_success(self) -> bool | None:
        """Whether the closing StatusReport reported success, or None if it never arrived."""
        if self.status_report_general_code is None or self.status_report_protocol_code is None:
            return None
        return (self.status_report_general_code == STATUS_REPORT_GENERAL_CODE_SUCCESS
                and self.status_report_protocol_code == STATUS_REPORT_PROTOCOL_CODE_SUCCESS)

    @property
    def case_handshake_succeeded(self) -> bool:
        """True only when the StatusReport closing Sigma3 carried a success code.

        A resumed handshake reports False because it has no Sigma3 and so no closing
        report; check resumed to tell that apart from a real rejection."""
        return self.all_messages_exchanged and self._status_report_indicates_success is True

    @property
    def sigma3_rejection_reason(self) -> str | None:
        """The Sigma3 rejection reason, or None when Sigma3 was accepted.

        Reads as 'general=<code> protocol=<code>'. General code 0 is success and 1 is a
        generic failure, with the protocol code carrying the CASE-specific detail."""
        if self._status_report_indicates_success is not False:
            return None
        return f"general={self.status_report_general_code} protocol={self.status_report_protocol_code}"

    @property
    def error_encountered(self) -> bool:
        """True when something went wrong during the exchange.

        Covers all three observable failures: this node rejected the peer, the peer
        rejected this node, or the handshake started and never finished. A resumed
        handshake is a legitimate outcome and does not count."""
        if self.used_session_resumption:
            return False
        if self.this_node_rejected_peer:
            return True
        if not self.all_messages_exchanged:
            return True
        # Completed, so the remaining question is whether the peer accepted Sigma3. Anything
        # other than a decoded success counts, including a report that could not be decoded:
        # there is then no evidence the handshake was accepted.
        return self._status_report_indicates_success is not True


# --- Streaming completed handshakes out of the native layer -------------------------------
# Passed as notification_queue_depth to take the native default depth.
NOTIFICATION_QUEUE_DEPTH_NATIVE_DEFAULT = 0

# Updated by the delivery thread each time it waits, so a consumer that cannot keep up with the
# notification queue is visible without consuming a record to find out.
_dropped_notification_count = 0


def _GetNotificationLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_case_handshake_metrics_wait_for_notification.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_case_handshake_metrics_start_notifications', PyChipError, [ctypes.c_uint32])
        setter.Set('pychip_case_handshake_metrics_stop_notifications', PyChipError, [])
        setter.Set('pychip_case_handshake_metrics_wait_for_notification', PyChipError,
                   [ctypes.POINTER(PyCASEHandshakeMetricsRecord), ctypes.c_uint32,
                    ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint32)])
        setter.Set('pychip_case_handshake_metrics_get_abandoned_count', PyChipError,
                   [ctypes.POINTER(ctypes.c_uint32)])
    return handle


def StartCASEHandshakeNotifications(notification_queue_depth: int = NOTIFICATION_QUEUE_DEPTH_NATIVE_DEFAULT) -> None:
    """Start timing handshakes and queueing the completed ones, discarding anything left from
    before.

    Called for you when the first listener is registered, so there is rarely a reason to call
    this directly. notification_queue_depth bounds how many completed handshakes may wait for the
    delivery thread; NOTIFICATION_QUEUE_DEPTH_NATIVE_DEFAULT takes the native default."""
    _GetNotificationLibraryHandle().pychip_case_handshake_metrics_start_notifications(notification_queue_depth).raise_on_error()


def StopCASEHandshakeNotifications() -> None:
    """Stop queueing completed handshakes and release any waiting consumer."""
    _GetNotificationLibraryHandle().pychip_case_handshake_metrics_stop_notifications().raise_on_error()


def _WaitForCompletedCASEHandshake(timeout_ms: int) -> "CASEHandshakeMetrics | None":
    """Block until a handshake completes, or the timeout expires, or notifications are stopped.

    The wait happens inside the native call, which releases the interpreter lock, so other
    Python threads keep running while this one is parked."""
    global _dropped_notification_count
    record = PyCASEHandshakeMetricsRecord()
    received = ctypes.c_uint8(0)
    dropped = ctypes.c_uint32(0)
    _GetNotificationLibraryHandle().pychip_case_handshake_metrics_wait_for_notification(
        ctypes.byref(record), timeout_ms, ctypes.byref(received),
        ctypes.byref(dropped)).raise_on_error()
    _dropped_notification_count = dropped.value
    if not received.value:
        return None
    return CASEHandshakeMetrics._from_native_record(record)


def GetAbandonedCASEHandshakeCount() -> int:
    """Handshakes that began but never reached a conclusion, so no listener heard about them.

    A handshake that times out with no reply is the usual cause. This is what explains a run
    seeing fewer notifications than it ran establishments."""
    abandoned = ctypes.c_uint32(0)
    _GetNotificationLibraryHandle().pychip_case_handshake_metrics_get_abandoned_count(
        ctypes.byref(abandoned)).raise_on_error()
    return abandoned.value


def GetDroppedCASEHandshakeNotificationCount() -> int:
    """Completed handshakes the notification queue had no room for, because the delivery thread was
    still busy. Non-zero means notifications were lost before any listener saw them."""
    return _dropped_notification_count

# --- Listeners: being told about each handshake as it completes ---------------------------
#
# Register a function once and it is handed the metrics for every handshake that completes,
# until you remove it. This is the only way metrics are reported; there is nothing to poll.
#
# Delivery is arranged so that no listener can affect the measurements or any other listener:
#
#   CHIP event loop       notification queue     delivery thread        listener queues
#   ───────────────       ──────────────────     ───────────────        ───────────────
#   handshake ends,  ──►  queued without    ──►  builds one metrics ──► one reference
#   record copied,        entering Python        object, then hands     queued per
#   thread returns        or waiting             it to every listener   listener, each
#                                                without waiting        drained at its
#                                                                       own pace
#
# The event loop never enters Python, so notification costs the handshake nothing however slow
# or numerous the listeners are. Each listener owns its queue, so a slow one delays only
# itself.


# How long the delivery thread parks in native code before looping. It wakes on its own to
# check whether it has been asked to stop, so this only bounds shutdown latency.
_DELIVERY_WAIT_MS = 200

# Messages a listener may fall behind by before its oldest are discarded. Bounded on purpose: a
# listener that never keeps up would otherwise grow memory until the process died.
LISTENER_NOTIFICATION_QUEUE_DEPTH = 256


logger = logging.getLogger(__name__)


class _RegisteredListener:
    """One registered function, its notification queue, and the thread that delivers to it."""

    def __init__(self, listener_id: int, listener, shared: bool):
        self.listener_id = listener_id
        self.listener = listener
        self.shared = shared
        self.pending_notifications: queue.Queue = queue.Queue(maxsize=LISTENER_NOTIFICATION_QUEUE_DEPTH)
        self.delivered = 0
        self.dropped = 0
        self.worker: threading.Thread | None = None
        self.active = True

    def enqueue_notification(self, metrics: "CASEHandshakeMetrics") -> None:
        """Hand over a completed handshake without ever blocking the caller."""
        try:
            self.pending_notifications.put_nowait(metrics)
        except queue.Full:
            # Discard this listener's oldest so the newest still gets through, and count it so
            # falling behind is visible rather than silent.
            try:
                self.pending_notifications.get_nowait()
                self.dropped += 1
                self.pending_notifications.put_nowait(metrics)
            except (queue.Empty, queue.Full):
                self.dropped += 1

    def invoke_listener(self, metrics: "CASEHandshakeMetrics") -> None:
        """Invoke the registered function, absorbing anything it raises."""
        try:
            self.listener(metrics)
            self.delivered += 1
        except Exception:
            logger.exception("CASE handshake listener %d raised; continuing", self.listener_id)


class _ListenerRegistry:
    """Owns the registered listeners, the delivery thread, and the worker threads."""

    def __init__(self):
        self._lock = threading.Lock()
        self._listeners: dict[int, _RegisteredListener] = {}
        self._next_id = 1
        self._delivery_thread: threading.Thread | None = None
        self._shared_worker: threading.Thread | None = None
        self._shared_delivery_queue: queue.Queue = queue.Queue()
        self._running = False

    # -- registration ----------------------------------------------------------------

    def add(self, listener, own_thread: bool) -> int:
        if not callable(listener):
            raise TypeError("listener must be callable")
        with self._lock:
            listener_id = self._next_id
            self._next_id += 1
            registered = _RegisteredListener(listener_id, listener, shared=not own_thread)
            self._listeners[listener_id] = registered
            if own_thread:
                registered.worker = threading.Thread(
                    target=self._deliver_on_dedicated_thread, args=(registered,),
                    name=f"case-listener-{listener_id}", daemon=True)
                registered.worker.start()
            self._ensure_running_locked()
            return listener_id

    def remove(self, listener_id: int) -> None:
        with self._lock:
            registered = self._listeners.pop(listener_id, None)
            if registered is None:
                raise KeyError(f"no CASE handshake listener with id {listener_id}")
            # Marked inactive under the lock, so no further delivery can pick it up. A call
            # already running finishes on its own; we do not wait for it, or a blocked listener
            # would hang the caller.
            registered.active = False
            should_stop = not self._listeners
        registered.pending_notifications.put_nowait(None)  # release its worker, if it has one
        if should_stop:
            self._stop()

    def remove_all(self) -> None:
        with self._lock:
            registered_all = list(self._listeners.values())
            self._listeners.clear()
            for registered in registered_all:
                registered.active = False
        for registered in registered_all:
            registered.pending_notifications.put_nowait(None)
        self._stop()

    def stats(self, listener_id: int) -> "CASEHandshakeListenerStats":
        with self._lock:
            registered = self._listeners.get(listener_id)
            if registered is None:
                raise KeyError(f"no CASE handshake listener with id {listener_id}")
            return CASEHandshakeListenerStats(
                listener_id=listener_id, delivered=registered.delivered,
                backlog=registered.pending_notifications.qsize(), dropped=registered.dropped,
                has_own_thread=not registered.shared)

    def listener_ids(self) -> list[int]:
        with self._lock:
            return sorted(self._listeners)

    # -- delivery --------------------------------------------------------------------

    def _ensure_running_locked(self) -> None:
        """Start notifying and start the threads on the first registration."""
        if self._running:
            return
        # Opening switches the whole thing on: it registers the backend so handshakes are timed,
        # and starts queueing the ones that complete.
        StartCASEHandshakeNotifications()
        self._running = True
        self._delivery_thread = threading.Thread(
            target=self._dispatch_notifications, name="case-metrics-delivery", daemon=True)
        self._delivery_thread.start()
        self._shared_worker = threading.Thread(
            target=self._deliver_on_shared_thread, name="case-listener-shared", daemon=True)
        self._shared_worker.start()

    def _stop(self) -> None:
        with self._lock:
            if not self._running:
                return
            self._running = False
        # Closing wakes the delivery thread so it can see it should exit.
        StopCASEHandshakeNotifications()
        self._shared_delivery_queue.put(None)

    def _dispatch_notifications(self) -> None:
        while True:
            with self._lock:
                if not self._running:
                    return
            # Parks in native code with the interpreter lock released, so other threads run.
            metrics = _WaitForCompletedCASEHandshake(_DELIVERY_WAIT_MS)
            if metrics is None:
                continue
            with self._lock:
                targets = [r for r in self._listeners.values() if r.active]
            # One immutable metrics object, shared by reference. Offering never blocks, so a
            # slow listener cannot hold up the others or the next handshake.
            for registered in targets:
                registered.enqueue_notification(metrics)
                if registered.shared:
                    self._shared_delivery_queue.put(registered)

    def _deliver_on_shared_thread(self) -> None:
        """Runs the cheap listeners in turn on one thread."""
        while True:
            registered = self._shared_delivery_queue.get()
            if registered is None:
                return
            try:
                metrics = registered.pending_notifications.get_nowait()
            except queue.Empty:
                continue
            if metrics is not None and registered.active:
                registered.invoke_listener(metrics)

    def _deliver_on_dedicated_thread(self, registered: _RegisteredListener) -> None:
        """Runs one slow listener on a thread of its own, isolated from the rest."""
        while True:
            metrics = registered.pending_notifications.get()
            if metrics is None:
                return
            if registered.active:
                registered.invoke_listener(metrics)


@dataclass(frozen=True)
class CASEHandshakeListenerStats:
    """How a listener is keeping up. A non-zero dropped count means it fell far enough behind
    that some handshakes were discarded for it."""

    listener_id: int
    delivered: int
    backlog: int
    dropped: int
    has_own_thread: bool


_listener_registry = _ListenerRegistry()


def AddCASEHandshakeListener(listener, own_thread: bool = False) -> int:
    """Register a function to be called with the metrics for every completed handshake.

    The function is handed one CASEHandshakeMetrics argument, and keeps being called until it is
    removed. Registering is all that is needed; there is no separate capture to start.

    Set own_thread for a listener that does something slow, such as writing to a database or a
    spreadsheet, so it runs on a thread of its own and cannot hold up the others. Cheap listeners
    should leave it False and share a thread.

    Returns an id to pass to RemoveCASEHandshakeListener."""
    return _listener_registry.add(listener, own_thread)


def RemoveCASEHandshakeListener(listener_id: int) -> None:
    """Stop calling a listener. Once this returns it receives nothing further, though a call
    already in progress finishes on its own."""
    _listener_registry.remove(listener_id)


def RemoveAllCASEHandshakeListeners() -> None:
    """Stop calling every registered listener and shut the delivery threads down."""
    _listener_registry.remove_all()


def GetCASEHandshakeListenerIds() -> list[int]:
    """The ids of every currently registered listener."""
    return _listener_registry.listener_ids()


def GetCASEHandshakeListenerStats(listener_id: int) -> CASEHandshakeListenerStats:
    """How many handshakes a listener has been given, how many are waiting, and how many were
    discarded because it could not keep up."""
    return _listener_registry.stats(listener_id)
