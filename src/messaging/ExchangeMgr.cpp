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

CHIP_ERROR ExchangeManager::Init(SecureSessionMgr * sessionMgr)
{
    if (mState != State::kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    mSessionMgr = sessionMgr;

    mNextExchangeId = GetRandU16();

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

ExchangeContext * ExchangeManager::NewContext(const NodeId & peerNodeId, ExchangeDelegate * delegate)
{
    return AllocContext(mNextExchangeId++, peerNodeId, true, delegate);
}

ExchangeContext * ExchangeManager::FindContext(NodeId peerNodeId, ExchangeDelegate * delegate, bool isInitiator)
{
    for (auto & ec : ContextPool)
    {
        if (ec.GetReferenceCount() > 0 && ec.GetPeerNodeId() == peerNodeId && ec.GetDelegate() == delegate &&
            ec.IsInitiator() == isInitiator)
            return &ec;
    }

    return nullptr;
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandler(uint32_t protocolId, ExchangeDelegate * delegate)
{
    return RegisterUMH(protocolId, kAnyMessageType, delegate);
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType, ExchangeDelegate * delegate)
{
    return RegisterUMH(protocolId, static_cast<int16_t>(msgType), delegate);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandler(uint32_t protocolId)
{
    return UnregisterUMH(protocolId, kAnyMessageType);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandler(uint32_t protocolId, uint8_t msgType)
{
    return UnregisterUMH(protocolId, static_cast<int16_t>(msgType));
}

void ExchangeManager::OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * msgLayer)
{
    ChipLogError(ExchangeManager, "Accept FAILED, err = %s", ErrorStr(error));
}

ExchangeContext * ExchangeManager::AllocContext(uint16_t ExchangeId, uint64_t PeerNodeId, bool Initiator,
                                                ExchangeDelegate * delegate)
{
    CHIP_FAULT_INJECT(FaultInjection::kFault_AllocExchangeContext, return nullptr);

    for (auto & ec : ContextPool)
    {
        if (ec.GetReferenceCount() == 0)
        {
            return ec.Alloc(this, ExchangeId, PeerNodeId, Initiator, delegate);
        }
    }

    ChipLogError(ExchangeManager, "Alloc ctxt FAILED");
    return nullptr;
}

void ExchangeManager::DispatchMessage(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                      System::PacketBufferHandle msgBuf)
{
    UnsolicitedMessageHandler * umh         = nullptr;
    UnsolicitedMessageHandler * matchingUMH = nullptr;
    CHIP_ERROR err                          = CHIP_NO_ERROR;

    // Search for an existing exchange that the message applies to. If a match is found...
    for (auto & ec : ContextPool)
    {
        if (ec.GetReferenceCount() > 0 && ec.MatchExchange(packetHeader, payloadHeader))
        {
            // Matched ExchangeContext; send to message handler.
            ec.HandleMessage(packetHeader, payloadHeader, std::move(msgBuf));

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
            if (umh->Delegate != nullptr && umh->ProtocolId == payloadHeader.GetProtocolID())
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
        auto * ec =
            AllocContext(payloadHeader.GetExchangeID(), packetHeader.GetSourceNodeId().Value(), false, matchingUMH->Delegate);
        VerifyOrExit(ec != nullptr, err = CHIP_ERROR_NO_MEMORY);

        ChipLogProgress(ExchangeManager, "ec pos: %d, id: %d, Delegate: 0x%x", ec - ContextPool.begin(), ec->GetExchangeId(),
                        ec->GetDelegate());

        ec->HandleMessage(packetHeader, payloadHeader, std::move(msgBuf));
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "DispatchMessage failed, err = %d", err);
    }
}

CHIP_ERROR ExchangeManager::RegisterUMH(uint32_t protocolId, int16_t msgType, ExchangeDelegate * delegate)
{
    UnsolicitedMessageHandler * umh      = UMHandlerPool;
    UnsolicitedMessageHandler * selected = nullptr;

    for (int i = 0; i < CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS; i++, umh++)
    {
        if (umh->Delegate == nullptr)
        {
            if (selected == nullptr)
                selected = umh;
        }
        else if (umh->ProtocolId == protocolId && umh->MessageType == msgType)
        {
            umh->Delegate = delegate;
            return CHIP_NO_ERROR;
        }
    }

    if (selected == nullptr)
        return CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS;

    selected->Delegate    = delegate;
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
        if (umh->Delegate != nullptr && umh->ProtocolId == protocolId && umh->MessageType == msgType)
        {
            umh->Delegate = nullptr;
            SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumUMHandlers);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER;
}

void ExchangeManager::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        const Transport::PeerConnectionState * state, System::PacketBufferHandle msgBuf,
                                        SecureSessionMgr * msgLayer)
{
    DispatchMessage(packetHeader, payloadHeader, std::move(msgBuf));
}

void ExchangeManager::OnConnectionExpired(const Transport::PeerConnectionState * state, SecureSessionMgr * mgr)
{
    for (auto & ec : ContextPool)
    {
        if (ec.GetReferenceCount() > 0 && ec.mPeerNodeId == state->GetPeerNodeId())
        {
            ec.Close();
            // Continue iterate because there can be multiple contexts associated with the connection.
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
