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

#include <app/CommandHandler.h>
#include <app/data-model-provider/OperationTypes.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemLayer.h>

#include <cstdint>
#include <map>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

/**
 * @brief Transport-agnostic proxy-session bookkeeping and ProxyMessage routing.
 *
 * Owns everything about a live proxy session that does not depend on the physical
 * transport: session-id allocation, the id → {transport, fabric} table, per-fabric
 * isolation checks, the active-session count for the MaxSessions gate, and the
 * pending ProxyMessageRequest exchange (with its response-timeout timer).
 *
 * A platform transport calls Alloc()/Register() when a connection completes,
 * Remove() on teardown, and DispatchMessageResponse()/DispatchMessageFailure() when
 * a commissionee reply arrives or the link drops. All calls run on the Matter
 * thread with the stack lock held.
 */
class CommissioningProxySessionManager
{
public:
    struct SessionInfo
    {
        CapabilitiesBitmap transport;
        FabricIndex fabricIndex;
    };

    CommissioningProxySessionManager()  = default;
    ~CommissioningProxySessionManager() = default;

    // --- Session table ------------------------------------------------------

    /// Allocate the next free session id (never 0, never a currently-active id).
    uint16_t AllocSessionId();

    /// Record an established session. Idempotent overwrite for a re-registered id.
    void RegisterSession(uint16_t sessionId, CapabilitiesBitmap transport, FabricIndex fabricIndex);

    /// Drop a session record and cancel any pending message for it. Idempotent.
    void RemoveSession(uint16_t sessionId);

    /// Look up a session, or std::nullopt if unknown.
    std::optional<SessionInfo> Find(uint16_t sessionId) const;

    /// Number of established sessions (excludes in-flight connects — the cluster
    /// adds each transport's IsConnectPending() to this for the MaxSessions gate).
    uint8_t ActiveCount() const { return static_cast<uint8_t>(mSessions.size()); }

    bool IsEmpty() const { return mSessions.empty(); }

    // --- ProxyMessageRequest routing ----------------------------------------

    /**
     * @brief Record a pending ProxyMessageRequest so the commissionee reply can be
     *        matched back to it, keeping the IM exchange open and arming a
     *        response-timeout timer. Call this only for a non-zero responseTimeout
     *        (a zero timeout is a fire-and-forget poll the cluster answers directly).
     *
     * @return Busy if a live request is already pending for @p sessionId (an already
     *         expired one is cleaned up first); Success once the pending state is
     *         armed. The caller then forwards the bytes to the transport, and calls
     *         AbortPending() if that send fails.
     */
    Protocols::InteractionModel::Status BeginMessage(uint16_t sessionId, app::CommandHandler * commandObj,
                                                     const DataModel::InvokeRequest & request, uint8_t responseTimeoutSeconds);

    /// Tear down pending state for a session without answering the commissioner
    /// (used to roll back BeginMessage when the transport send fails).
    void AbortPending(uint16_t sessionId);

    /// Forward a commissionee reply as a ProxyMessageResponse. No-op if nothing is
    /// pending for @p sessionId.
    void DispatchMessageResponse(uint16_t sessionId, const uint8_t * data, size_t length);

    /// Fail a pending ProxyMessageRequest (e.g. session dropped mid-message).
    void DispatchMessageFailure(uint16_t sessionId, Protocols::InteractionModel::Status status);

    /// Cancel every session and pending message (cluster teardown).
    void Shutdown();

private:
    struct PendingMessage; // defined in the .cpp

    static void ResponseTimeoutCallback(System::Layer * layer, void * appState);

    std::map<uint16_t, SessionInfo> mSessions;
    std::map<uint16_t, PendingMessage *> mPending;
    uint16_t mNextSessionId = 1;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
