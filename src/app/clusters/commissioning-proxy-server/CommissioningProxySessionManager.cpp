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
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

using Status = Protocols::InteractionModel::Status;

// One in-flight ProxyMessageRequest per session: keeps the IM exchange open until
// the commissionee replies and the transport hands the bytes back.
struct CommissioningProxySessionManager::PendingMessage
{
    CommissioningProxySessionManager * owner;
    app::CommandHandler::Handle handle;
    app::ConcreteCommandPath path;
    uint16_t sessionId;
};

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
    } while (mSessions.count(id) > 0);
    return id;
}

void CommissioningProxySessionManager::RegisterSession(uint16_t sessionId, CapabilitiesBitmap transport, FabricIndex fabricIndex)
{
    mSessions[sessionId] = SessionInfo{ transport, fabricIndex };
}

void CommissioningProxySessionManager::RemoveSession(uint16_t sessionId)
{
    AbortPending(sessionId);
    mSessions.erase(sessionId);
}

std::optional<CommissioningProxySessionManager::SessionInfo> CommissioningProxySessionManager::Find(uint16_t sessionId) const
{
    auto it = mSessions.find(sessionId);
    if (it == mSessions.end())
    {
        return std::nullopt;
    }
    return it->second;
}

void CommissioningProxySessionManager::ResponseTimeoutCallback(System::Layer * /*layer*/, void * appState)
{
    auto * pm   = static_cast<PendingMessage *>(appState);
    auto * self = pm->owner;

    auto it = self->mPending.find(pm->sessionId);
    if (it == self->mPending.end() || it->second != pm)
    {
        return; // Already resolved (reply arrived or session closed).
    }

    self->mPending.erase(it);
    ChipLogProgress(Zcl, "CommissioningProxy: ProxyMessageRequest responseTimeout expired for session %u", pm->sessionId);
    // Per spec, an expired ProxyMessageRequest ResponseTimeout SHALL return TIMEOUT.
    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        cmd->AddStatus(pm->path, Status::Timeout);
    }
    delete pm;
}

Status CommissioningProxySessionManager::BeginMessage(uint16_t sessionId, app::CommandHandler * commandObj,
                                                      const DataModel::InvokeRequest & request, uint8_t responseTimeoutSeconds)
{
    // Reject if another request for this session is still live; clean up an already
    // expired one rather than blocking the session forever.
    auto pendingIt = mPending.find(sessionId);
    if (pendingIt != mPending.end())
    {
        PendingMessage * existing = pendingIt->second;
        if (existing->handle.Get() != nullptr)
        {
            ChipLogError(Zcl, "CommissioningProxy: session %u already has a pending ProxyMessageRequest (BUSY)", sessionId);
            return Status::Busy;
        }
        DeviceLayer::SystemLayer().CancelTimer(ResponseTimeoutCallback, existing);
        delete existing;
        mPending.erase(pendingIt);
    }

    auto * pm = new PendingMessage{ this, app::CommandHandler::Handle(commandObj), request.path, sessionId };
    commandObj->FlushAcksRightAwayOnSlowCommand();
    mPending[sessionId] = pm;

    if (auto * exchange = commandObj->GetExchangeContext())
    {
        exchange->SetResponseTimeout(System::Clock::Seconds16(responseTimeoutSeconds));
    }

    CHIP_ERROR err =
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(responseTimeoutSeconds), ResponseTimeoutCallback, pm);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "CommissioningProxy: failed to start ProxyMessage response timer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return Status::Success;
}

void CommissioningProxySessionManager::AbortPending(uint16_t sessionId)
{
    auto it = mPending.find(sessionId);
    if (it == mPending.end())
    {
        return;
    }
    PendingMessage * pm = it->second;
    DeviceLayer::SystemLayer().CancelTimer(ResponseTimeoutCallback, pm);
    mPending.erase(it);
    delete pm;
}

void CommissioningProxySessionManager::DispatchMessageResponse(uint16_t sessionId, const uint8_t * data, size_t length)
{
    auto it = mPending.find(sessionId);
    if (it == mPending.end())
    {
        ChipLogDetail(Zcl, "CommissioningProxy: no pending request for session %u — dropping commissionee data", sessionId);
        return;
    }

    PendingMessage * pm = it->second;
    mPending.erase(it);
    DeviceLayer::SystemLayer().CancelTimer(ResponseTimeoutCallback, pm);

    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        Commands::ProxyMessageResponse::Type response;
        response.sessionID = sessionId;
        response.message.SetNonNull(ByteSpan(data, length));
        cmd->AddResponse(pm->path, response);
    }
    delete pm;
}

void CommissioningProxySessionManager::DispatchMessageFailure(uint16_t sessionId, Status status)
{
    auto it = mPending.find(sessionId);
    if (it == mPending.end())
    {
        return;
    }

    PendingMessage * pm = it->second;
    mPending.erase(it);
    DeviceLayer::SystemLayer().CancelTimer(ResponseTimeoutCallback, pm);

    if (app::CommandHandler * cmd = pm->handle.Get())
    {
        cmd->AddStatus(pm->path, status);
    }
    delete pm;
}

void CommissioningProxySessionManager::Shutdown()
{
    for (auto & [sessionId, pm] : mPending)
    {
        DeviceLayer::SystemLayer().CancelTimer(ResponseTimeoutCallback, pm);
        delete pm;
    }
    mPending.clear();
    mSessions.clear();
}

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
