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

#include <controller/python/matter/case_capture/CASEHandshakeMetricsRecord.h>
#include <controller/python/matter/native/PyChipError.h>

extern "C" {

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

// Handshakes given up so that a later one could take their slot, and so never notified. Only
// counts handshakes actually discarded; it is never an estimate of one presumed dead.
PyChipError pychip_case_handshake_metrics_get_abandoned_count(uint32_t * abandoned);

// Handshakes still open, whether progressing or stuck with no reply.
//
// Reported rather than guessed at. How long a handshake may legitimately take is not knowable
// here, since a peer may advertise an MRP retry interval of up to an hour, so writing one off
// after any fixed delay risks discarding one that was still running. Together with the abandoned
// count this accounts exactly for a run that saw fewer notifications than it started handshakes:
// notified + abandoned + in flight is every handshake that began.
PyChipError pychip_case_handshake_metrics_get_in_flight_count(uint32_t * inFlight);

// sizeof(PychipCASEHandshakeMetricsRecord), so the ctypes mirror can prove it still agrees with
// this struct. A mismatch means the two definitions have drifted and every field read through the
// mirror is suspect, so the Python side refuses to run rather than reporting nonsense.
PyChipError pychip_case_handshake_metrics_get_record_size(uint32_t * size);

} // extern "C"
