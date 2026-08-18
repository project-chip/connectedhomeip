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
#include <app/clusters/commissioning-proxy-server/CommissioningProxySessionManager.h>

#include <clusters/CommissioningProxy/Commands.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemClock.h>

#include <utility>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Status = Protocols::InteractionModel::Status;

namespace {
// Head-room so the IM exchange outlives our own response timer and can still carry
// the Status::Timeout back to the commissioner.
constexpr uint16_t kResponseTimeoutMarginSecs = 5;
} // namespace

CommissioningProxySessionManager::SessionSlot * CommissioningProxySessionManager::FindSlot(uint16_t sessionId)
{
    for (auto & slot : mSessions)
    {
        if (slot.inUse && slot.sessionId == sessionId)
        {
            return &slot;
        }
    }
    return nullptr;
}

const CommissioningProxySessionManager::SessionSlot * CommissioningProxySessionManager::FindSlot(uint16_t sessionId) const
{
    return const_cast<CommissioningProxySessionManager *>(this)->FindSlot(sessionId);
}

uint8_t CommissioningProxySessionManager::ActiveCount() const
{
    uint8_t count = 0;
    for (const auto & slot : mSessions)
    {
        count = static_cast<uint8_t>(count + (slot.inUse ? 1 : 0));
    }
    return count;
}

uint16_t CommissioningProxySessionManager::AllocSessionId()
{
    uint16_t id;
    do
    {
        if (mNextSessionId == 0)
        {
            mNextSessionId = 1;
        }
        id = mNextSessionId++;
    } while (FindSlot(id) != nullptr);
    return id;
}

void CommissioningProxySessionManager::RegisterSession(uint16_t sessionId, CapabilitiesBitmap transport, FabricIndex fabricIndex)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        for (auto & candidate : mSessions)
        {
            if (!candidate.inUse)
            {
                slot = &candidate;
                break;
            }
        }
    }
    // The cluster gates on MaxSessions before connecting, so the pool cannot be full here.
    VerifyOrDie(slot != nullptr);

    slot->inUse     = true;
    slot->sessionId = sessionId;
    slot->info      = SessionInfo{ transport, fabricIndex };
}

void CommissioningProxySessionManager::RemoveSession(uint16_t sessionId)
{
    AbortPending(sessionId);
    if (SessionSlot * slot = FindSlot(sessionId))
    {
        slot->inUse = false;
    }
}

std::optional<CommissioningProxySessionManager::SessionInfo> CommissioningProxySessionManager::Find(uint16_t sessionId) const
{
    const SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr)
    {
        return std::nullopt;
    }
    return slot->info;
}

std::optional<uint16_t> CommissioningProxySessionManager::FindAnyOnFabric(FabricIndex fabricIndex) const
{
    for (const auto & slot : mSessions)
    {
        if (slot.inUse && slot.info.fabricIndex == fabricIndex)
        {
            return slot.sessionId;
        }
    }
    return std::nullopt;
}

void CommissioningProxySessionManager::OnResponseTimeout(PendingMessage * pm)
{
    SessionSlot * slot = FindSlot(pm->sessionId);
    if (slot == nullptr || slot->pending != pm)
    {
        return; // Already resolved (reply arrived or session closed).
    }

    slot->pending = nullptr;
    ChipLogProgress(Zcl, "CommissioningProxy: ProxyMessageRequest responseTimeout expired for session %u", pm->sessionId);
    // Per spec, an expired ProxyMessageRequest ResponseTimeout SHALL return TIMEOUT.
    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        cmd->AddStatus(pm->path, Status::Timeout);
    }
    mPendingPool.ReleaseObject(pm);
}

Status CommissioningProxySessionManager::BeginMessage(uint16_t sessionId, app::CommandHandler * commandObj,
                                                      const DataModel::InvokeRequest & request, uint8_t responseTimeoutSeconds)
{
    SessionSlot * slot = FindSlot(sessionId);
    VerifyOrReturnError(slot != nullptr, Status::NotFound);

    // Reject if another request for this session is still live; clean up an already
    // expired one rather than blocking the session forever.
    if (slot->pending != nullptr)
    {
        if (slot->pending->handle.Get() != nullptr)
        {
            ChipLogError(Zcl, "CommissioningProxy: session %u already has a pending ProxyMessageRequest (BUSY)", sessionId);
            return Status::Busy;
        }
        mTimerDelegate.CancelTimer(slot->pending);
        mPendingPool.ReleaseObject(slot->pending);
        slot->pending = nullptr;
    }

    auto * pm = mPendingPool.CreateObject(this, app::CommandHandler::Handle(commandObj), request.path, sessionId);
    VerifyOrReturnError(pm != nullptr, Status::ResourceExhausted);
    commandObj->FlushAcksRightAwayOnSlowCommand();
    slot->pending = pm;

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        // The exchange must outlive our own timer, otherwise it expires first and the
        // Status::Timeout that OnResponseTimeout adds never reaches the commissioner.
        exchange->SetResponseTimeout(
            System::Clock::Seconds16(static_cast<uint16_t>(responseTimeoutSeconds + kResponseTimeoutMarginSecs)));
    }

    CHIP_ERROR err = mTimerDelegate.StartTimer(pm, System::Clock::Seconds16(responseTimeoutSeconds));
    if (err != CHIP_NO_ERROR)
    {
        // Nothing else would ever resolve this request: the session would stay Busy for
        // every later ProxyMessageRequest and the commissioner would get no response.
        ChipLogError(Zcl, "CommissioningProxy: failed to start ProxyMessage response timer: %" CHIP_ERROR_FORMAT, err.Format());
        AbortPending(sessionId);
        return Status::Failure;
    }

    return Status::Success;
}

void CommissioningProxySessionManager::AbortPending(uint16_t sessionId)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr || slot->pending == nullptr)
    {
        return;
    }
    PendingMessage * pm = slot->pending;
    slot->pending       = nullptr;
    mTimerDelegate.CancelTimer(pm);
    mPendingPool.ReleaseObject(pm);
}

void CommissioningProxySessionManager::DispatchMessageResponse(uint16_t sessionId, ByteSpan data)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr || slot->pending == nullptr)
    {
        ChipLogDetail(Zcl, "CommissioningProxy: no pending request for session %u — dropping commissionee data", sessionId);
        return;
    }

    PendingMessage * pm = slot->pending;
    slot->pending       = nullptr;
    mTimerDelegate.CancelTimer(pm);

    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        Commands::ProxyMessageResponse::Type response;
        response.sessionID = sessionId;
        response.message.SetNonNull(data);
        cmd->AddResponse(pm->path, response);
    }
    mPendingPool.ReleaseObject(pm);
}

void CommissioningProxySessionManager::DispatchMessageFailure(uint16_t sessionId, Status status)
{
    SessionSlot * slot = FindSlot(sessionId);
    if (slot == nullptr || slot->pending == nullptr)
    {
        return;
    }

    PendingMessage * pm = slot->pending;
    slot->pending       = nullptr;
    mTimerDelegate.CancelTimer(pm);

    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        cmd->AddStatus(pm->path, status);
    }
    mPendingPool.ReleaseObject(pm);
}

void CommissioningProxySessionManager::Shutdown()
{
    for (auto & slot : mSessions)
    {
        if (slot.pending != nullptr)
        {
            mTimerDelegate.CancelTimer(slot.pending);
            mPendingPool.ReleaseObject(slot.pending);
            slot.pending = nullptr;
        }
        slot.inUse = false;
    }
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
