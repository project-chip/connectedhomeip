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

// Handshake timing. Independent of the header capture above: it observes the messages
// through the tracing hooks in SessionManager, which report both directions, and so can
// time the outbound Sigma1/Sigma3 that the inbound-only observer cannot see.

// Handshakes retained per capture session when the caller does not ask for a specific
// capacity. Enough for a batch of concurrent handshakes without reserving much; override it
// via the maxRecords argument to pychip_case_timing_start when capturing more.
#define PYCHIP_CASE_TIMING_DEFAULT_MAX_RECORDS 64

// Largest capacity pychip_case_timing_start will accept. The records are allocated up front, so
// this bounds that allocation to roughly 640 kB and keeps a mistyped capacity from exhausting
// memory. A request above it is rejected rather than silently reduced, so a caller never
// believes it has more room than it does.
#define PYCHIP_CASE_TIMING_MAX_RECORDS 4096

// Bits set in PychipCaseTimingRecord::marks, indicating which timestamps are valid.
#define PYCHIP_CASE_TIMING_MARK_SIGMA1_SENT 0x01u
#define PYCHIP_CASE_TIMING_MARK_SIGMA2_RECEIVED 0x02u
#define PYCHIP_CASE_TIMING_MARK_SIGMA3_SENT 0x04u
#define PYCHIP_CASE_TIMING_MARK_STATUS_REPORT_RECEIVED 0x08u
#define PYCHIP_CASE_TIMING_MARK_SIGMA2_RESUME_RECEIVED 0x10u
#define PYCHIP_CASE_TIMING_MARK_DISCOVERY 0x20u
// The closing StatusReport's body decoded, so the status*Code fields below are valid.
#define PYCHIP_CASE_TIMING_MARK_STATUS_PARSED 0x40u
// This node sent a StatusReport carrying a failure, i.e. it rejected the peer.
#define PYCHIP_CASE_TIMING_MARK_LOCAL_FAILURE 0x80u

// Room for a PeerAddress rendered as text, e.g. "UDP:[fe80::1%wlan0]:5540". Sized above
// Transport::PeerAddress::kMaxToStringSize rather than set to it, so the Python mirror stays a
// plain literal; CASECapture.cpp static_asserts that it is still large enough.
#define PYCHIP_CASE_TIMING_PEER_ADDR_LEN 80

// One initiator-side CASE handshake. Timestamps are monotonic microseconds from
// System::SystemClock(), taken as the message crosses the transport layer. A field is
// only meaningful when its corresponding bit is set in `marks`.
// Layout must match the ctypes mirror in case_capture/__init__.py.
struct PychipCaseTimingRecord
{
    uint64_t sigma1SentUs;
    uint64_t sigma2ReceivedUs;
    uint64_t sigma3SentUs;
    uint64_t statusReportReceivedUs;
    uint64_t sigma2ResumeReceivedUs;
    // Operational discovery preceding this handshake. Captured before Sigma1 is sent and
    // attached to the handshake that followed it.
    uint64_t discoveryStartUs;
    uint64_t discoveryDoneUs;
    // Identifies which handshake a message belongs to, so concurrent handshakes stay
    // separate. localNodeId is this node's ephemeral initiator id for the handshake, which
    // distinguishes controllers whose exchange id counters could otherwise collide.
    uint64_t localNodeId;
    // Which peer this handshake is with. Learned from the first message the peer sends, so
    // it stays zero on a handshake that never got a reply. Resolved from the discovery that
    // produced the peer's address; zero when the address was already cached.
    uint64_t peerNodeId;
    // Codes carried by the StatusReport that closed Sigma3. Valid only when
    // PYCHIP_CASE_TIMING_MARK_STATUS_PARSED is set. Both zero means success.
    uint16_t statusGeneralCode;
    uint16_t statusProtocolCode;
    // The exchange carrying this handshake. Together with localNodeId it identifies the
    // handshake, which is how a message is routed to its own record while others are in flight.
    uint16_t exchangeId;
    // Bitmask of PYCHIP_CASE_TIMING_MARK_*, saying which fields above have been filled in.
    uint8_t marks;
    // The peer's transport address, empty until the peer sends its first message. Always
    // available even when peerNodeId is not, so it is the reliable way to tell two DUTs
    // apart.
    char peerAddress[PYCHIP_CASE_TIMING_PEER_ADDR_LEN];
};

// Register the timing backend and clear any previously captured records. maxRecords sets how
// many handshakes to retain; pass 0 for PYCHIP_CASE_TIMING_DEFAULT_MAX_RECORDS. Returns
// CHIP_ERROR_INVALID_ARGUMENT if it exceeds PYCHIP_CASE_TIMING_MAX_RECORDS.
PyChipError pychip_case_timing_start(uint32_t maxRecords);

// Unregister the backend. Captured records remain readable until the next start.
PyChipError pychip_case_timing_stop(void);

// Discard all captured records without unregistering.
PyChipError pychip_case_timing_reset(void);

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
PyChipError pychip_case_timing_get_records(PychipCaseTimingRecord * out, uint32_t capacity, uint32_t * written,
                                           uint32_t * available, uint32_t * dropped);

} // extern "C"
