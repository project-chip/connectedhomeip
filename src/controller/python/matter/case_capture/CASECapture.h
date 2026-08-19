/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <stdint.h>

#include <controller/CHIPDeviceController.h>
#include <controller/python/matter/native/PyChipError.h>

extern "C" {

// Layout must match the ctypes mirror in case_capture/__init__.py.
struct PychipCaseCapturedHeaders
{
    uint8_t sessionType;
    uint8_t sFlag; // 0/1
    uint8_t dsiz;  // 0 = neither, 1 = NodeID, 2 = GroupID
    uint16_t protocolId;
    uint8_t opcode;
    uint8_t isInitiator;
    uint8_t present;
};

struct PychipCaseCaptureSnapshot
{
    PychipCaseCapturedHeaders sigma2;
    PychipCaseCapturedHeaders sigma2Resume;
    PychipCaseCapturedHeaders statusReport;
    uint8_t statusReportParsed; // 1 if the StatusReport body decoded; if 0, the *Code fields below are not meaningful.
    uint16_t statusReportGeneralCode;
    uint32_t statusReportProtocolId;
    uint16_t statusReportProtocolCode;
};

PyChipError pychip_case_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl);
PyChipError pychip_case_capture_reset(void);
PyChipError pychip_case_capture_get_snapshot(PychipCaseCaptureSnapshot * out);

// CASE handshake metrics. Independent of the header capture above: it observes the messages
// through the tracing hooks in SessionManager, which report both directions, and so can
// time the outbound Sigma1/Sigma3 that the inbound-only observer cannot see.

// CASE handshakes retained per capture session when the caller does not ask for a specific
// capacity. Enough for a batch of concurrent handshakes without reserving much; override it
// via the maxCASEHandshakes argument to pychip_case_handshake_metrics_start_capture when capturing more.
#define PYCHIP_CASE_HANDSHAKE_METRICS_DEFAULT_CAPACITY 64

// Largest capacity pychip_case_handshake_metrics_start_capture will accept. The records are allocated up front, so
// this bounds that allocation to roughly 640 kB and keeps a mistyped capacity from exhausting
// memory. A request above it is rejected rather than silently reduced, so a caller never
// believes it has more room than it does.
#define PYCHIP_CASE_HANDSHAKE_METRICS_MAX_CAPACITY 4096

// Bits set in PychipCASEHandshakeMetricsRecord::recordedFields, indicating which timestamps are valid.
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA1_SENT 0x01u
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA2_RECEIVED 0x02u
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA3_SENT 0x04u
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_RECEIVED 0x08u
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA2_RESUME_RECEIVED 0x10u
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_DEVICE_DISCOVERY 0x20u
// The closing StatusReport's body decoded, so the status*Code fields below are valid.
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES 0x40u
// This node sent a StatusReport carrying a failure, i.e. it rejected the peer.
#define PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_THIS_NODE_REJECTED_PEER 0x80u

// Room for a PeerAddress rendered as text, e.g. "UDP:[fe80::1%wlan0]:5540". Sized above
// Transport::PeerAddress::kMaxToStringSize rather than set to it, so the Python mirror stays a
// plain literal; CASECapture.cpp static_asserts that it is still large enough.
#define PYCHIP_CASE_HANDSHAKE_METRICS_PEER_ADDRESS_MAX_LENGTH 80

// One initiator-side CASE handshake. Timestamps are monotonic microseconds from
// System::SystemClock(), taken as the message crosses the transport layer. A field is
// only meaningful when its corresponding bit is set in `recordedFields`.
// Layout must match the ctypes mirror in case_capture/__init__.py.
struct PychipCASEHandshakeMetricsRecord
{
    uint64_t sigma1SentTimestampUs;
    uint64_t sigma2ReceivedTimestampUs;
    uint64_t sigma3SentTimestampUs;
    uint64_t statusReportReceivedTimestampUs;
    uint64_t sigma2ResumeReceivedTimestampUs;
    // Operational discovery that resolved this handshake's peer. Recorded before Sigma1 is
    // sent, then attached once the peer replies and its address identifies which lookup it
    // came from. Left unset when the address was not resolved during this capture, so a span
    // is never attributed to the wrong handshake.
    uint64_t discoveryStartedTimestampUs;
    uint64_t discoveryCompletedTimestampUs;
    // Identifies which handshake a message belongs to, so concurrent handshakes stay
    // separate. localEphemeralNodeId is this node's ephemeral initiator id for the handshake, which
    // distinguishes controllers whose exchange id counters could otherwise collide.
    uint64_t localEphemeralNodeId;
    // Which peer this handshake is with. Learned from the first message the peer sends, so
    // it stays zero on a handshake that never got a reply. Resolved from the discovery that
    // produced the peer's address; zero when the address was already cached.
    uint64_t peerNodeId;
    // Codes carried by the StatusReport that closed Sigma3. Valid only when
    // PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES is set. Both zero means success.
    uint16_t statusReportGeneralCode;
    uint16_t statusReportProtocolCode;
    // The exchange carrying this handshake. Together with localEphemeralNodeId it identifies the
    // handshake, which is how a message is routed to its own record while others are in flight.
    uint16_t exchangeId;
    // Bitmask of PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_*, saying which fields above have been filled in.
    uint8_t recordedFields;
    // The peer's transport address, empty until the peer sends its first message. Always
    // available even when peerNodeId is not, so it is the reliable way to tell two DUTs
    // apart.
    char peerTransportAddress[PYCHIP_CASE_HANDSHAKE_METRICS_PEER_ADDRESS_MAX_LENGTH];
};

// Register the metrics backend and clear any previously captured records. maxCASEHandshakes sets how
// many handshakes to retain; pass 0 for PYCHIP_CASE_HANDSHAKE_METRICS_DEFAULT_CAPACITY. Returns
// CHIP_ERROR_INVALID_ARGUMENT if it exceeds PYCHIP_CASE_HANDSHAKE_METRICS_MAX_CAPACITY.
PyChipError pychip_case_handshake_metrics_start_capture(uint32_t maxCASEHandshakes);

// Unregister the backend. Captured records stay readable until the next start.
PyChipError pychip_case_handshake_metrics_stop_capture(void);

// Discard all captured records without unregistering.
PyChipError pychip_case_handshake_metrics_reset_capture(void);

// Copies up to `capacity` records into `out`, oldest first.
//
// The caller supplies the buffer, so the record count never has to be agreed between C and
// the ctypes mirror: only the layout of a single record does.
//
//   written   number of records copied, at most `capacity`
//   available total recorded, so a caller can size a buffer and ask again
//   dropped   handshakes seen after the capture filled up, and therefore not recorded
//
// `out` may be null when `capacity` is 0, to query the counts alone.
PyChipError pychip_case_handshake_metrics_get_records(PychipCASEHandshakeMetricsRecord * out, uint32_t capacity, uint32_t * written,
                                                      uint32_t * available, uint32_t * dropped);

} // extern "C"
