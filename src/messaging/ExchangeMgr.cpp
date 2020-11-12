/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements the ExchangeManager class.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <cstring>
#include <inttypes.h>
#include <stddef.h>

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <support/CHIPFaultInjection.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Encoding;
using namespace chip::Inet;
using namespace chip::System;

namespace chip {

/**
 *  Constructor for the ExchangeManager class.
 *  It sets the state to kState_NotInitialized.
 *
 *  @note
 *    The class must be initialized via ExchangeManager::Init()
 *    prior to use.
 *
 */
ExchangeManager::ExchangeManager()
{
    mState = State::kState_NotInitialized;
}

CHIP_ERROR ExchangeManager::Init(SecureSessionMgrBase * sessionMgr)
{
    if (mState != State::kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    mSessionMgr = sessionMgr;

    mNextExchangeId = GetRandU16();

    memset((void *) ContextPool, 0, sizeof(ContextPool));
    mContextsInUse = 0;

    memset(UMHandlerPool, 0, sizeof(UMHandlerPool));
    OnExchangeContextChanged = nullptr;

    sessionMgr->SetDelegate(this);

    mState = State::kState_Initialized;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExchangeManager::Shutdown()
{
    if (mSessionMgr != nullptr)
    {
        mSessionMgr->SetDelegate(nullptr);
        mSessionMgr = nullptr;
    }

    OnExchangeContextChanged = nullptr;

    mState = State::kState_NotInitialized;

    return CHIP_NO_ERROR;
}

ExchangeContext * ExchangeManager::NewContext(SecureSessionHandle session, void * appState)
{
    return AllocContext(mNextExchangeId++, session, true, appState);
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeContext::MessageReceiveFunct handler,
                                                              void * appState)
{
    return RegisterUMH(protocolId, kAnyMessageType, handler, appState);
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType,
                                                              ExchangeContext::MessageReceiveFunct handler, void * appState)
{
    return RegisterUMH(protocolId, static_cast<int16_t>(msgType), handler, appState);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandler(uint32_t protocolId)
{
    return UnregisterUMH(protocolId, kAnyMessageType);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType)
{
    return UnregisterUMH(protocolId, static_cast<int16_t>(msgType));
}

void ExchangeManager::OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * msgLayer)
{
    ChipLogError(ExchangeManager, "Accept FAILED, err = %s", ErrorStr(error));
}

ExchangeContext * ExchangeManager::AllocContext(uint16_t ExchangeId, SecureSessionHandle session, bool Initiator, void * AppState)
{
    ExchangeContext * ec = ContextPool;

    CHIP_FAULT_INJECT(FaultInjection::kFault_AllocExchangeContext, return nullptr);

    for (int i = 0; i < CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS; i++, ec++)
    {
        if (ec->GetReferenceCount() == 0)
        {
            ec->Alloc(this, ExchangeId, session, Initiator, AppState);
            return ec;
        }
    }

    ChipLogError(ExchangeManager, "Alloc ctxt FAILED");
    return nullptr;
}

void ExchangeManager::DispatchMessage(SecureSessionHandle session, const PacketHeader & packetHeader,
                                      const PayloadHeader & payloadHeader, PacketBuffer * msgBuf)
{
    UnsolicitedMessageHandler * umh         = nullptr;
    UnsolicitedMessageHandler * matchingUMH = nullptr;
    ExchangeContext * ec                    = nullptr;
    CHIP_ERROR err                          = CHIP_NO_ERROR;

    // Search for an existing exchange that the message applies to. If a match is found...
    ec = ContextPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS; i++, ec++)
    {
        if (ec->GetReferenceCount() > 0 && ec->MatchExchange(session, packetHeader, payloadHeader))
        {
            // Matched ExchangeContext; send to message handler.
            ec->HandleMessage(packetHeader, payloadHeader, msgBuf);

            msgBuf = nullptr;

            ExitNow(err = CHIP_NO_ERROR);
        }
    }

    // Search for an unsolicited message handler if it marked as being sent by an initiator. Since we didn't
    // find an existing exchange that matches the message, it must be an unsolicited message. However all
    // unsolicited messages must be marked as being from an initiator.
    if (payloadHeader.IsInitiator())
    {
        // Search for an unsolicited message handler that can handle the message. Prefer handlers that can explicitly
        // handle the message type over handlers that handle all messages for a profile.
        umh = (UnsolicitedMessageHandler *) UMHandlerPool;

        matchingUMH = nullptr;

        for (int i = 0; i < CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS; i++, umh++)
        {
            if (umh->Handler != nullptr && umh->ProtocolId == payloadHeader.GetProtocolID())
            {
                if (umh->MessageType == payloadHeader.GetMessageType())
                {
                    matchingUMH = umh;
                    break;
                }

                if (umh->MessageType == kAnyMessageType)
                    matchingUMH = umh;
            }
        }
    }
    // Discard the message if it isn't marked as being sent by an initiator.
    else
    {
        ExitNow(err = CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR);
    }

    // If we found a handler or we need to create a new exchange context (EC).
    if (matchingUMH != nullptr)
    {
        ExchangeContext::MessageReceiveFunct umhandler = nullptr;

        ec = AllocContext(payloadHeader.GetExchangeID(), session, false, matchingUMH->AppState);
        VerifyOrExit(ec != nullptr, err = CHIP_ERROR_NO_MEMORY);

        umhandler = matchingUMH->Handler;

        ChipLogProgress(ExchangeManager, "ec id: %d, AppState: 0x%x", (ec - ContextPool + 1), ec->GetAppState());

        ec->HandleMessage(packetHeader, payloadHeader, msgBuf, umhandler);
        msgBuf = nullptr;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "DispatchMessage failed, err = %d", err);
    }

    if (msgBuf != nullptr)
    {
        PacketBuffer::Free(msgBuf);
    }
}

CHIP_ERROR ExchangeManager::RegisterUMH(uint32_t protocolId, int16_t msgType, ExchangeContext::MessageReceiveFunct handler,
                                        void * appState)
{
    UnsolicitedMessageHandler * umh      = UMHandlerPool;
    UnsolicitedMessageHandler * selected = nullptr;

    for (int i = 0; i < CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS; i++, umh++)
    {
        if (umh->Handler == nullptr)
        {
            if (selected == nullptr)
                selected = umh;
        }
        else if (umh->ProtocolId == protocolId && umh->MessageType == msgType)
        {
            umh->Handler  = handler;
            umh->AppState = appState;
            return CHIP_NO_ERROR;
        }
    }

    if (selected == nullptr)
        return CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS;

    selected->Handler     = handler;
    selected->AppState    = appState;
    selected->ProtocolId  = protocolId;
    selected->MessageType = msgType;

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumUMHandlers);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExchangeManager::UnregisterUMH(uint32_t protocolId, int16_t msgType)
{
    UnsolicitedMessageHandler * umh = UMHandlerPool;

    for (int i = 0; i < CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS; i++, umh++)
    {
        if (umh->Handler != nullptr && umh->ProtocolId == protocolId && umh->MessageType == msgType)
        {
            umh->Handler = nullptr;
            SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumUMHandlers);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER;
}

void ExchangeManager::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        SecureSessionHandle session, System::PacketBuffer * msgBuf, SecureSessionMgrBase * msgLayer)
{
    DispatchMessage(session, packetHeader, payloadHeader, msgBuf);
}

void ExchangeManager::OnConnectionExpired(SecureSessionHandle session, SecureSessionMgrBase * mgr)
{
    // Search for an existing exchange that the message applies to. If a match is found...
    ExchangeContext * ec = ContextPool;
    for (int i = 0; i < CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS; i++, ec++)
    {
        if (ec->GetReferenceCount() > 0 && ec->mSecureSession == session)
        {
            ec->Close();
        }
    }
}

void ExchangeManager::IncrementContextsInUse()
{
    mContextsInUse++;
}

void ExchangeManager::DecrementContextsInUse()
{
    if (mContextsInUse >= 1)
    {
        mContextsInUse--;
    }
    else
    {
        ChipLogError(ExchangeManager, "No context in use, decrement failed");
    }
}

} // namespace chip
