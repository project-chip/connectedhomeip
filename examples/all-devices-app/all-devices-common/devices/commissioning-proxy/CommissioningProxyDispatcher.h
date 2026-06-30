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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

// Private API exposed by CommissioningProxyCommandDelegate.cpp (the dispatcher)
// to the per-transport modules (CommissioningProxyPafTransport.cpp,
// CommissioningProxyBleTransport.cpp).  All transport-agnostic session and
// pending-response bookkeeping lives in the dispatcher; the per-transport
// modules call these helpers to register / clean up sessions and to route
// commissionee responses back to the original ProxyMessageRequest exchange.
namespace ProxyDispatcher {

// Allocate the next free proxy session ID (skips 0 and any already-active session).
uint16_t AllocSessionId();

// Record a newly-established proxy session.  The dispatcher tracks transport +
// owning fabric so handlers can dispatch ProxyMessageRequest / ProxyDisconnectRequest
// without each transport maintaining its own fabric isolation logic.
void RegisterSession(uint16_t sessionId, CapabilitiesBitmap transport, chip::FabricIndex fabricIndex);

// Drop a session record.  Idempotent.
void RemoveSession(uint16_t sessionId);

// Forward a commissionee reply to the commissioner as a ProxyMessageResponse.
// If no ProxyMessageRequest is currently pending for the sessionId, the call
// is silently dropped (matches existing PAF receive-callback behaviour).
void DispatchMessageResponse(uint16_t sessionId, const uint8_t * data, size_t length);

// Fail a pending ProxyMessageRequest with the given status (e.g. transport
// dropped the session mid-message).  No-op if nothing is pending.
void DispatchMessageFailure(uint16_t sessionId, chip::Protocols::InteractionModel::Status status);

// ------------------------------------------------------------------
// Multi-transport ProxyScanRequest aggregation
//
// A ProxyScanRequest MAY select more than one transport (CommissioningProxy
// cluster spec, ProxyScanRequest Transport field: "Multiple transports MAY be
// selected for the scan").  The dispatcher starts each requested transport's
// scan in parallel, owns the single command handle, and emits one combined
// ProxyScanResponse once every started sub-scan has reported its results here.
// ------------------------------------------------------------------

using ScanResultEntry = Structs::ScanResultStruct::Type;

// Reported once by each transport module when its sub-scan finishes (pass an
// empty span if it found nothing).  The dispatcher deep-copies the entries, so
// the caller's backing storage need not outlive the call.  When the last
// outstanding sub-scan reports, the combined ProxyScanResponse is sent.
void ContributeScanResults(chip::Span<const ScanResultEntry> results);

} // namespace ProxyDispatcher
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
