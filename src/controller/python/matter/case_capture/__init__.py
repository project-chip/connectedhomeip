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

Timing: StartTiming() begins timing the Sigma exchange in both directions, then read the
result whenever you like:

    case_capture.StartTiming()          # also clears any earlier records
    await dev_ctrl.GetConnectedDevice(node_id, allowPASE=False)

    hs = case_capture.GetLastHandshake()
    print(hs.device_discovery_ms, hs.sigma1_sigma2_exchange_ms, hs.sigma3_exchange_ms)

StopTiming() is optional; leaving the backend registered costs a comparison per message.
See the Handshake class for the phases reported."""

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


# --- Handshake timing -------------------------------------------------------------
#
# Timings come from a tracing backend in the native library that timestamps each Sigma
# message as SessionManager sends or receives it, so nothing here depends on log text or
# trace files.

# Must match PYCHIP_CASE_TIMING_PEER_ADDR_LEN in CASECapture.h. This is the only size that
# has to agree between the two sides; how many records exist is decided at runtime.
PEER_ADDR_LEN = 80

# The native layer records microseconds, because the Sigma phases are around a millisecond and
# millisecond resolution would round most of the detail away. Durations are reported in
# milliseconds, so every raw difference passes through this divisor.
_US_PER_MS = 1000.0

# A StatusReport says success only when both of its codes are zero; see GeneralStatusCode
# and kProtocolCodeSuccess in src/protocols/secure_channel/Constants.h.
STATUS_GENERAL_CODE_SUCCESS = 0
STATUS_PROTOCOL_CODE_SUCCESS = 0

# Passed as max_records to StartTiming to accept the native default capacity.
TIMING_CAPACITY_DEFAULT = 0

# Passed as the capacity to _FetchRecords to read the counters without copying any records.
_COUNTS_ONLY = 0

# Must match the PYCHIP_CASE_TIMING_MARK_* bits in CASECapture.h.
MARK_SIGMA1_SENT = 0x01
MARK_SIGMA2_RECEIVED = 0x02
MARK_SIGMA3_SENT = 0x04
MARK_STATUS_REPORT_RECEIVED = 0x08
MARK_SIGMA2_RESUME_RECEIVED = 0x10
MARK_DISCOVERY = 0x20
MARK_STATUS_PARSED = 0x40
MARK_LOCAL_FAILURE = 0x80


# Mirror of the C struct PychipCaseTimingRecord defined in CASECapture.h.
class PyCaseTimingRecord(ctypes.Structure):
    _fields_ = [
        ("sigma1SentUs", ctypes.c_uint64),
        ("sigma2ReceivedUs", ctypes.c_uint64),
        ("sigma3SentUs", ctypes.c_uint64),
        ("statusReportReceivedUs", ctypes.c_uint64),
        ("sigma2ResumeReceivedUs", ctypes.c_uint64),
        ("discoveryStartUs", ctypes.c_uint64),
        ("discoveryDoneUs", ctypes.c_uint64),
        ("localNodeId", ctypes.c_uint64),
        ("peerNodeId", ctypes.c_uint64),
        ("statusGeneralCode", ctypes.c_uint16),
        ("statusProtocolCode", ctypes.c_uint16),
        ("exchangeId", ctypes.c_uint16),
        ("marks", ctypes.c_uint8),
        ("peerAddress", ctypes.c_char * PEER_ADDR_LEN),
    ]


@dataclass(frozen=True)
class Handshake:
    """One initiator-side CASE handshake.

    Three durations, all in milliseconds and all None when the handshake did not reach
    that point:

        device_discovery_ms          operational discovery, before any Sigma message
        sigma1_sigma2_exchange_ms    Sigma1 out -> Sigma2 in
        sigma3_exchange_ms           Sigma3 out -> StatusReport in
        total_duration_ms            discovery start -> StatusReport in

    The raw microsecond marks are kept as fields, so anything else (the local turnaround
    between the two exchanges, an end-to-end total) can be derived from them.

    With several handshakes running at once, peer_node_id says which device each one is
    with, falling back to peer_address when the address came from cache rather than a
    fresh lookup."""

    sigma1_sent_us: int | None
    sigma2_received_us: int | None
    sigma3_sent_us: int | None
    status_report_received_us: int | None
    sigma2_resume_received_us: int | None
    discovery_start_us: int | None
    discovery_done_us: int | None
    status_general_code: int | None
    status_protocol_code: int | None
    local_failure_sent: bool
    # Identity of the handshake, so concurrent handshakes can be told apart. local_node_id
    # is this node's ephemeral initiator id, which differs per controller.
    exchange_id: int
    local_node_id: int
    # Which peer the handshake is with. See peer_node_id and peer_address below.
    peer_node_id: int
    peer_address: str

    @staticmethod
    def _from_record(record: PyCaseTimingRecord) -> "Handshake":
        def value(field: str, bit: int) -> int | None:
            return getattr(record, field) if record.marks & bit else None

        return Handshake(
            sigma1_sent_us=value("sigma1SentUs", MARK_SIGMA1_SENT),
            sigma2_received_us=value("sigma2ReceivedUs", MARK_SIGMA2_RECEIVED),
            sigma3_sent_us=value("sigma3SentUs", MARK_SIGMA3_SENT),
            status_report_received_us=value("statusReportReceivedUs", MARK_STATUS_REPORT_RECEIVED),
            sigma2_resume_received_us=value("sigma2ResumeReceivedUs", MARK_SIGMA2_RESUME_RECEIVED),
            discovery_start_us=value("discoveryStartUs", MARK_DISCOVERY),
            discovery_done_us=value("discoveryDoneUs", MARK_DISCOVERY),
            status_general_code=value("statusGeneralCode", MARK_STATUS_PARSED),
            status_protocol_code=value("statusProtocolCode", MARK_STATUS_PARSED),
            local_failure_sent=bool(record.marks & MARK_LOCAL_FAILURE),
            exchange_id=record.exchangeId,
            local_node_id=record.localNodeId,
            peer_node_id=record.peerNodeId,
            peer_address=record.peerAddress.decode("utf-8", errors="replace"),
        )

    @staticmethod
    def _delta_ms(start: int | None, end: int | None) -> float | None:
        if start is None or end is None:
            return None
        return (end - start) / _US_PER_MS

    @property
    def device_discovery_ms(self) -> float | None:
        """Operational discovery: address lookup started -> resolution done.

        None when the connect reused a cached address and no lookup ran."""
        return self._delta_ms(self.discovery_start_us, self.discovery_done_us)

    @property
    def sigma1_sigma2_exchange_ms(self) -> float | None:
        """The Sigma1/Sigma2 exchange: Sigma1 out -> Sigma2 in.

        A full round trip, so it includes the peer's Sigma1 verification and Sigma2
        generation as well as the network. It does not include this node's verification
        of Sigma2, which happens after Sigma2 arrives.

        The clock starts when Sigma1 is first sent, so if MRP had to retransmit, the retry
        wait is part of this figure."""
        return self._delta_ms(self.sigma1_sent_us, self.sigma2_received_us)

    @property
    def sigma3_exchange_ms(self) -> float | None:
        """The Sigma3/StatusReport exchange: Sigma3 out -> StatusReport in.

        A full round trip, so it includes the peer's Sigma3 verification. The
        StatusReport is a generic Secure Channel message rather than a Sigma one, but it
        is the only signal that Sigma3 was accepted.

        As with the Sigma1/Sigma2 exchange, any MRP retry wait is included."""
        return self._delta_ms(self.sigma3_sent_us, self.status_report_received_us)

    @property
    def total_duration_ms(self) -> float | None:
        """Everything observed, from the first mark to the last: discovery start through to
        the StatusReport closing Sigma3.

        Wider than the three phases summed, because it also covers the gap between
        discovery finishing and Sigma1 going out, and this node's own Sigma2 verification
        between the two exchanges.

        Falls back to starting at Sigma1 when no discovery ran, so it always reports
        whatever was actually measured. Runs a few milliseconds short of the wall clock
        around GetConnectedDevice, which additionally covers session setup before the
        lookup and session activation after the handshake."""
        start = self.discovery_start_us if self.discovery_start_us is not None else self.sigma1_sent_us
        return self._delta_ms(start, self.status_report_received_us)

    @property
    def resumed(self) -> bool:
        """True if the peer answered Sigma1 with Sigma2_Resume, in which case no Sigma2
        or Sigma3 was exchanged and neither exchange duration is available."""
        return self.sigma2_resume_received_us is not None

    @property
    def complete(self) -> bool:
        """True if a full Sigma1/Sigma2/Sigma3 handshake ran to its StatusReport.

        Structural only: it says the four messages were exchanged, not that the peer
        accepted Sigma3. See success for that."""
        return all(value is not None for value in (
            self.sigma1_sent_us, self.sigma2_received_us,
            self.sigma3_sent_us, self.status_report_received_us))

    @property
    def _status_is_success(self) -> bool | None:
        """Whether the closing StatusReport reported success, or None if it never arrived."""
        if self.status_general_code is None or self.status_protocol_code is None:
            return None
        return (self.status_general_code == STATUS_GENERAL_CODE_SUCCESS
                and self.status_protocol_code == STATUS_PROTOCOL_CODE_SUCCESS)

    @property
    def success(self) -> bool:
        """True only when the StatusReport closing Sigma3 carried a success code.

        A resumed handshake reports False because it has no Sigma3 and so no closing
        report; check resumed to tell that apart from a real rejection."""
        return self.complete and self._status_is_success is True

    @property
    def sigma3_status_error(self) -> str | None:
        """The Sigma3 rejection reason, or None when Sigma3 was accepted.

        Reads as 'general=<code> protocol=<code>'. General code 0 is success and 1 is a
        generic failure, with the protocol code carrying the CASE-specific detail."""
        if self._status_is_success is not False:
            return None
        return f"general={self.status_general_code} protocol={self.status_protocol_code}"

    @property
    def exception_encountered(self) -> bool:
        """True when something went wrong during the exchange.

        Covers all three observable failures: this node rejected the peer, the peer
        rejected this node, or the handshake started and never finished. A resumed
        handshake is a legitimate outcome and does not count."""
        if self.resumed:
            return False
        if self.local_failure_sent or self.sigma3_status_error is not None:
            return True
        return not self.complete


def _GetTimingLibraryHandle() -> ctypes.CDLL:
    handle = GetLibraryHandle(HandleFlags(0))
    if not handle.pychip_case_timing_get_records.argtypes:
        setter = NativeLibraryHandleMethodArguments(handle)
        setter.Set('pychip_case_timing_start', PyChipError, [ctypes.c_uint32])
        setter.Set('pychip_case_timing_stop', PyChipError, [])
        setter.Set('pychip_case_timing_reset', PyChipError, [])
        setter.Set('pychip_case_timing_get_records', PyChipError,
                   [ctypes.POINTER(PyCaseTimingRecord), ctypes.c_uint32,
                    ctypes.POINTER(ctypes.c_uint32), ctypes.POINTER(ctypes.c_uint32),
                    ctypes.POINTER(ctypes.c_uint32)])
    return handle


def StartTiming(max_records: int = TIMING_CAPACITY_DEFAULT) -> None:
    """Register the timing backend, discarding any previously captured handshakes.

    max_records is how many handshakes to retain, and TIMING_CAPACITY_DEFAULT takes the native
    default from PYCHIP_CASE_TIMING_DEFAULT_MAX_RECORDS. Raise it when capturing more
    handshakes than that between calls, for instance a long run of concurrent establishments
    with no reset in between; GetDroppedCount reports whether the capacity was exceeded."""
    _GetTimingLibraryHandle().pychip_case_timing_start(max_records).raise_on_error()


def StopTiming() -> None:
    """Unregister the backend. Captured handshakes stay readable until the next start."""
    _GetTimingLibraryHandle().pychip_case_timing_stop().raise_on_error()


def ResetTiming() -> None:
    """Discard captured handshakes without unregistering the backend."""
    _GetTimingLibraryHandle().pychip_case_timing_reset().raise_on_error()


def _FetchRecords(capacity: int) -> tuple[list[PyCaseTimingRecord], int, int]:
    """Copies up to `capacity` records out of the native buffer.

    Returns the records copied, how many exist in total, and the dropped count. Pass 0 to
    query the counts without copying anything."""
    buffer = (PyCaseTimingRecord * capacity)() if capacity > 0 else None
    written = ctypes.c_uint32(0)
    available = ctypes.c_uint32(0)
    dropped = ctypes.c_uint32(0)
    _GetTimingLibraryHandle().pychip_case_timing_get_records(
        buffer, capacity, ctypes.byref(written), ctypes.byref(available),
        ctypes.byref(dropped)).raise_on_error()
    records = [buffer[i] for i in range(written.value)] if buffer is not None else []
    return records, available.value, dropped.value


def GetHandshakes() -> list[Handshake]:
    """Every handshake captured since the last StartTiming/ResetTiming, oldest first."""
    _, available, _ = _FetchRecords(_COUNTS_ONLY)
    if available == 0:
        return []
    records, _, _ = _FetchRecords(available)
    return [Handshake._from_record(record) for record in records]


def GetLastHandshake() -> Handshake | None:
    """The most recent handshake, or None if none has been captured yet.

    With several handshakes in flight this is whichever sent Sigma1 last, which is rarely
    the one you want; use GetHandshakes and match on peer_node_id instead."""
    handshakes = GetHandshakes()
    return handshakes[-1] if handshakes else None


def GetDroppedCount() -> int:
    """Handshakes seen after the capture filled up, and so not recorded. Raise the capacity
    via StartTiming(max_records=...) if this is ever non-zero."""
    _, _, dropped = _FetchRecords(_COUNTS_ONLY)
    return dropped
