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

Metrics capture: StartCASEMetricsCapture() begins measuring the handshake in both
directions, and the results can be read whenever you like:

    case_capture.StartCASEMetricsCapture()   # also clears any earlier records
    await dev_ctrl.GetConnectedDevice(node_id, allowPASE=False)

    metrics = case_capture.GetLastCASEHandshakeMetrics()
    print(metrics.device_discovery_duration_ms)
    print(metrics.sigma1_sigma2_exchange_duration_ms)
    print(metrics.sigma3_exchange_duration_ms)

Use GetAllCASEHandshakeMetrics() instead when several handshakes ran, and match them up by
peer_node_id. StopCASEMetricsCapture() is optional; leaving the backend registered costs
one comparison per message.
See the CASEHandshakeMetrics class for the phases reported."""

import ctypes
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

# Must match PYCHIP_CASE_HANDSHAKE_METRICS_PEER_ADDRESS_MAX_LENGTH in CASECapture.h. This is the only size that
# has to agree between the two sides; how many records exist is decided at runtime.
PEER_TRANSPORT_ADDRESS_MAX_LENGTH = 80

# The native layer records microseconds, because the Sigma phases are around a millisecond and
# millisecond resolution would round most of the detail away. Durations are reported in
# milliseconds, so every raw difference passes through this divisor.
_MICROSECONDS_PER_MILLISECOND = 1000.0

# A StatusReport says success only when both of its codes are zero; see GeneralStatusCode
# and kProtocolCodeSuccess in src/protocols/secure_channel/Constants.h.
STATUS_REPORT_GENERAL_CODE_SUCCESS = 0
STATUS_REPORT_PROTOCOL_CODE_SUCCESS = 0

# Passed as max_case_handshakes to StartCASEMetricsCapture to take the native default capacity.
CASE_HANDSHAKE_CAPTURE_DEFAULT_CAPACITY = 0

# Passed as the capacity to _FetchCASEHandshakeRecords to read the counters without copying any records.
_READ_COUNTERS_ONLY = 0

# How many times GetAllCASEHandshakeMetrics will resize and refetch when handshakes are being captured while
# it reads. Each retry uses the count the native side just reported, so it converges at once
# unless establishment is ongoing.
_MAX_FETCH_ATTEMPTS = 4

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
        total_case_handshake_duration_ms           discovery start -> StatusReport in

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


def _GetCASEMetricsLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_case_handshake_metrics_get_records.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_case_handshake_metrics_start_capture', PyChipError, [ctypes.c_uint32])
        setter.Set('pychip_case_handshake_metrics_stop_capture', PyChipError, [])
        setter.Set('pychip_case_handshake_metrics_reset_capture', PyChipError, [])
        setter.Set('pychip_case_handshake_metrics_get_records', PyChipError,
                   [ctypes.POINTER(PyCASEHandshakeMetricsRecord), ctypes.c_uint32,
                    ctypes.POINTER(ctypes.c_uint32), ctypes.POINTER(ctypes.c_uint32),
                    ctypes.POINTER(ctypes.c_uint32)])
    return handle


def StartCASEMetricsCapture(max_case_handshakes: int = CASE_HANDSHAKE_CAPTURE_DEFAULT_CAPACITY) -> None:
    """Register the timing backend, discarding any previously captured handshakes.

    max_case_handshakes is how many handshakes to retain, and CASE_HANDSHAKE_CAPTURE_DEFAULT_CAPACITY takes the native
    default from PYCHIP_CASE_HANDSHAKE_METRICS_DEFAULT_CAPACITY. Raise it when capturing more
    handshakes than that between calls, for instance a long run of concurrent establishments
    with no reset in between; GetDroppedCASEHandshakeCount reports whether the capacity was exceeded.

    Raises if max_case_handshakes exceeds the native ceiling PYCHIP_CASE_HANDSHAKE_METRICS_MAX_CAPACITY, rather
    than silently retaining fewer than asked for."""
    _GetCASEMetricsLibraryHandle().pychip_case_handshake_metrics_start_capture(max_case_handshakes).raise_on_error()


def StopCASEMetricsCapture() -> None:
    """Unregister the backend. Captured handshakes stay readable until the next start."""
    _GetCASEMetricsLibraryHandle().pychip_case_handshake_metrics_stop_capture().raise_on_error()


def ResetCASEMetricsCapture() -> None:
    """Discard captured handshakes without unregistering the backend."""
    _GetCASEMetricsLibraryHandle().pychip_case_handshake_metrics_reset_capture().raise_on_error()


def _FetchCASEHandshakeRecords(capacity: int) -> tuple[list[PyCASEHandshakeMetricsRecord], int, int]:
    """Copies up to `capacity` records out of the native buffer.

    Returns the records copied, how many exist in total, and the dropped count. Pass 0 to
    query the counts without copying anything."""
    buffer = (PyCASEHandshakeMetricsRecord * capacity)() if capacity > 0 else None
    written = ctypes.c_uint32(0)
    available = ctypes.c_uint32(0)
    dropped = ctypes.c_uint32(0)
    _GetCASEMetricsLibraryHandle().pychip_case_handshake_metrics_get_records(
        buffer, capacity, ctypes.byref(written), ctypes.byref(available),
        ctypes.byref(dropped)).raise_on_error()
    records = [buffer[i] for i in range(written.value)] if buffer is not None else []
    return records, available.value, dropped.value


def GetAllCASEHandshakeMetrics() -> list[CASEHandshakeMetrics]:
    """Every handshake captured since the last StartCASEMetricsCapture/ResetCASEMetricsCapture, oldest first."""
    # Sizing the buffer and filling it are two separate calls, so more handshakes can be
    # captured in between while others are still in flight. Retry on a short buffer rather than
    # silently returning fewer handshakes than exist.
    _, capacity, _ = _FetchCASEHandshakeRecords(_READ_COUNTERS_ONLY)
    for _ in range(_MAX_FETCH_ATTEMPTS):
        if capacity == 0:
            return []
        records, available, _ = _FetchCASEHandshakeRecords(capacity)
        if available <= capacity:
            return [CASEHandshakeMetrics._from_native_record(record) for record in records]
        capacity = available
    raise RuntimeError(
        f"handshake count kept growing over {_MAX_FETCH_ATTEMPTS} attempts; capture is still active")


def GetLastCASEHandshakeMetrics() -> CASEHandshakeMetrics | None:
    """The most recent handshake, or None if none has been captured yet.

    With several handshakes in flight this is whichever sent Sigma1 last, which is rarely
    the one you want; use GetAllCASEHandshakeMetrics and match on peer_node_id instead."""
    case_handshakes = GetAllCASEHandshakeMetrics()
    return case_handshakes[-1] if case_handshakes else None


def GetDroppedCASEHandshakeCount() -> int:
    """Handshakes seen after the capture filled up, and so not recorded. Raise the capacity
    via StartCASEMetricsCapture(max_case_handshakes=...) if this is ever non-zero."""
    _, _, dropped = _FetchCASEHandshakeRecords(_READ_COUNTERS_ONLY)
    return dropped
