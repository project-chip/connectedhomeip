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

#include <controller/python/matter/native/PyChipError.h>
#include <lib/support/BitFlags.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace python {

// Which of a handshake record's fields have been filled in. Most mark a timestamp; the last two
// record what the StatusReport said. The values are a bitmask, and the Python mirror tests the
// same bits, so they are fixed rather than sequential.
enum class CASEHandshakeRecordedField : uint8_t
{
    kSigma1Sent           = 0x01u,
    kSigma2Received       = 0x02u,
    kSigma3Sent           = 0x04u,
    kStatusReportReceived = 0x08u,
    kSigma2ResumeReceived = 0x10u,
    kDeviceDiscovery      = 0x20u,
    // The closing StatusReport's body decoded, so the status*Code fields are valid.
    kStatusReportCodes = 0x40u,
    // This node sent a StatusReport carrying a failure, i.e. it rejected the peer.
    kThisNodeRejectedPeer = 0x80u,
};

using CASEHandshakeRecordedFields = BitFlags<CASEHandshakeRecordedField>;

// Handshakes that may be in progress at the same time. A record is scratch space held only
// between Sigma1 and the handshake concluding, so this bounds concurrency rather than how many
// handshakes may be run. If every slot is occupied the longest-running one is given up, so a
// handshake that never concludes cannot hold a slot for ever.
static constexpr size_t kCASEHandshakeMetricsMaxInFlight = 16;

// Room for a PeerAddress rendered as text, e.g. "UDP:[fe80::1%wlan0]:5540". Sized above
// Transport::PeerAddress::kMaxToStringSize rather than set to it, so the Python mirror stays a
// plain literal; the assertion below keeps the two in step.
static constexpr size_t kCASEHandshakeMetricsPeerAddressMaxLength = 80;
static_assert(Transport::PeerAddress::kMaxToStringSize <= kCASEHandshakeMetricsPeerAddressMaxLength,
              "PychipCASEHandshakeMetricsRecord::peerTransportAddress is too small to hold a rendered PeerAddress.");

// Completed handshakes the notification queue holds when the caller does not ask for a specific
// depth. Deep enough to absorb a burst of concurrent completions while the consumer is busy.
static constexpr uint32_t kCASEHandshakeMetricsNotificationQueueDefaultDepth = 256;

// Largest notification queue depth accepted, bounding the allocation to roughly 640 kB. A larger
// request is rejected rather than quietly reduced, so a caller never believes it has more room
// than it does.
static constexpr uint32_t kCASEHandshakeMetricsNotificationQueueMaxDepth = 4096;

} // namespace python
} // namespace chip

extern "C" {

// One initiator-side CASE handshake. Timestamps are monotonic microseconds from
// System::SystemClock(), taken as the message crosses the transport layer. A field is
// only meaningful when its corresponding bit is set in `recordedFields`.
//
// Layout must match the ctypes mirror in case_capture/__init__.py. The mirror checks its own
// sizeof against pychip_case_handshake_metrics_get_record_size() when it loads, so the two
// cannot drift apart unnoticed.
struct PychipCASEHandshakeMetricsRecord
{
    uint64_t sigma1SentTimestampUs;
    uint64_t sigma2ReceivedTimestampUs;
    uint64_t sigma3SentTimestampUs;
    uint64_t statusReportReceivedTimestampUs;
    uint64_t sigma2ResumeReceivedTimestampUs;
    // Operational discovery that resolved this handshake's peer. Recorded before Sigma1 is
    // sent, then attached once the peer replies and its address identifies which lookup it
    // came from. Left unset when the address was not resolved while listening, so a span is
    // never attributed to the wrong handshake.
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
    // CASEHandshakeRecordedField::kStatusReportCodes is set. Both zero means success.
    uint16_t statusReportGeneralCode;
    uint16_t statusReportProtocolCode;
    // The exchange carrying this handshake. Together with localEphemeralNodeId it identifies the
    // handshake, which is how a message is routed to its own record while others are in flight.
    uint16_t exchangeId;
    // Which fields above have been filled in.
    chip::python::CASEHandshakeRecordedFields recordedFields;
    // The peer's transport address, empty until the peer sends its first message. Always
    // available even when peerNodeId is not, so it is the reliable way to tell two DUTs
    // apart.
    char peerTransportAddress[chip::python::kCASEHandshakeMetricsPeerAddressMaxLength];
};

// The flags occupy exactly the byte the Python mirror reads them from.
static_assert(sizeof(chip::python::CASEHandshakeRecordedFields) == sizeof(uint8_t),
              "recordedFields must stay one byte wide for the ctypes mirror.");

// Register the metrics backend and start notifying, so handshakes start being timed and queued.
// This is the single switch that turns the feature on; there is nothing else to start.
//
// Pass 0 for kCASEHandshakeMetricsNotificationQueueDefaultDepth. Returns
// CHIP_ERROR_INVALID_ARGUMENT if the depth exceeds kCASEHandshakeMetricsNotificationQueueMaxDepth.
// Starting also clears anything left from a previous run.
PyChipError pychip_case_handshake_metrics_start_notifications(uint32_t depth);

// Unregister the backend, stop notifying, and wake any waiting consumer so it can exit.
// Queued records are discarded.
PyChipError pychip_case_handshake_metrics_stop_notifications(void);

// Waits for the next completed handshake and copies it into `out`.
//
// Blocks up to timeoutMs, so the consumer thread parks in native code with the interpreter lock
// released and other Python threads run freely. Returns with `received` false when the timeout
// expires or notifications are stopped, which is how a consumer notices it should stop.
//
//   received  1 when a record was copied into `out`, 0 when it timed out or notifications were stopped
//   dropped   running total of completed handshakes the notification queue had no room for, so a consumer
//             that cannot keep up is visible rather than silently lossy
//
// This runs on the calling thread rather than the CHIP event loop, which it would otherwise block.
PyChipError pychip_case_handshake_metrics_wait_for_notification(PychipCASEHandshakeMetricsRecord * out, uint32_t timeoutMs,
                                                                uint8_t * received, uint32_t * dropped);

// Handshakes that began but never reached a conclusion, so no listener was ever told about them.
// A timeout with no reply is the usual cause. This is what explains a run seeing fewer
// notifications than it ran establishments.
PyChipError pychip_case_handshake_metrics_get_abandoned_count(uint32_t * abandoned);

// sizeof(PychipCASEHandshakeMetricsRecord), so the ctypes mirror can prove it still agrees with
// this struct. A mismatch means the two definitions have drifted and every field read through the
// mirror is suspect, so the Python side refuses to run rather than reporting nonsense.
PyChipError pychip_case_handshake_metrics_get_record_size(uint32_t * size);

} // extern "C"
