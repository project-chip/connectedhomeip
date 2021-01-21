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
#include <protocols/Protocols.h>
#include <support/CHIPFaultInjection.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Encoding;
using namespace chip::Inet;
using namespace chip::System;

namespace chip {
namespace Messaging {

/**
 *  Constructor for the ExchangeManager class.
 *  It sets the state to kState_NotInitialized.
 *
 *  @note
 *    The class must be initialized via ExchangeManager::Init()
 *    prior to use.
 *
 */
ExchangeManager::ExchangeManager() : mReliableMessageMgr(mContextPool)
{
    mState = State::kState_NotInitialized;
}

CHIP_ERROR ExchangeManager::Init(NodeId localNodeId, TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr)
{
    if (mState != State::kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    mLocalNodeId  = localNodeId;
    mTransportMgr = transportMgr;
    mSessionMgr   = sessionMgr;

    mNextExchangeId = GetRandU16();
    mNextPaseKeyId  = 0;

    mContextsInUse = 0;

    memset(UMHandlerPool, 0, sizeof(UMHandlerPool));

    mTransportMgr->SetRendezvousSession(this);

    sessionMgr->SetDelegate(this);

    mReliableMessageMgr.Init(sessionMgr->SystemLayer(), sessionMgr);

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

    mState = State::kState_NotInitialized;

    return CHIP_NO_ERROR;
}

ExchangeContext * ExchangeManager::NewContext(SecureSessionHandle session, ExchangeDelegate * delegate)
{
    return AllocContext(mNextExchangeId++, session, true, delegate);
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

ExchangeContext * ExchangeManager::AllocContext(uint16_t ExchangeId, SecureSessionHandle session, bool Initiator,
                                                ExchangeDelegate * delegate)
{
    CHIP_FAULT_INJECT(FaultInjection::kFault_AllocExchangeContext, return nullptr);

    for (auto & ec : mContextPool)
    {
        if (ec.GetReferenceCount() == 0)
        {
            return ec.Alloc(this, ExchangeId, session, Initiator, delegate);
        }
    }

    ChipLogError(ExchangeManager, "Alloc ctxt FAILED");
    return nullptr;
}

void ExchangeManager::DispatchMessage(SecureSessionHandle session, const PacketHeader & packetHeader,
                                      const PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    UnsolicitedMessageHandler * umh         = nullptr;
    UnsolicitedMessageHandler * matchingUMH = nullptr;
    bool sendAckAndCloseExchange            = false;

    // Search for an existing exchange that the message applies to. If a match is found...
    for (auto & ec : mContextPool)
    {
        if (ec.GetReferenceCount() > 0 && ec.MatchExchange(session, packetHeader, payloadHeader))
        {
            // Found a matching exchange. Set flag for correct subsequent CRMP
            // retransmission timeout selection.
            if (!ec.mReliableMessageContext.HasRcvdMsgFromPeer())
            {
                ec.mReliableMessageContext.SetMsgRcvdFromPeer(true);
            }

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
    // Discard the message if it isn't marked as being sent by an initiator and the message does not need to send
    // an ack to the peer.
    else if (!payloadHeader.IsNeedsAck())
    {
        ExitNow(err = CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR);
    }

    // If we didn't find an existing exchange that matches the message, and no unsolicited message handler registered
    // to hand this message, we need to create a temporary exchange to send an ack for this message and then close this exchange.
    sendAckAndCloseExchange = payloadHeader.IsNeedsAck() && (matchingUMH == nullptr);

    // If we found a handler or we need to create a new exchange context (EC).
    if (matchingUMH != nullptr || sendAckAndCloseExchange)
    {
        ExchangeContext * ec = nullptr;

        if (sendAckAndCloseExchange)
        {
            // If rcvd msg is from initiator then this exchange is created as not Initiator.
            // If rcvd msg is not from initiator then this exchange is created as Initiator.
            ec = AllocContext(payloadHeader.GetExchangeID(), session, !payloadHeader.IsInitiator(), nullptr);
        }
        else
        {
            ec = AllocContext(payloadHeader.GetExchangeID(), session, false, matchingUMH->Delegate);
        }

        VerifyOrExit(ec != nullptr, err = CHIP_ERROR_NO_MEMORY);

        ChipLogProgress(ExchangeManager, "ec pos: %d, id: %d, Delegate: 0x%x", ec - mContextPool.begin(), ec->GetExchangeId(),
                        ec->GetDelegate());

        ec->HandleMessage(packetHeader, payloadHeader, std::move(msgBuf));

        // Close exchange if it was created only to send ack for a duplicate message.
        if (sendAckAndCloseExchange)
            ec->Close();
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

ChannelHandle ExchangeManager::EstablishChannel(const ChannelBuilder & builder, ChannelDelegate * delegate)
{
    ChannelContext * channelContext = nullptr;

    // Find an existing Channel matching the builder
    mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
        if (context->MatchesBuilder(builder, mSessionMgr))
        {
            channelContext = context;
            return false;
        }
        return true;
    });

    if (channelContext == nullptr)
    {
        // create a new channel if not found
        channelContext = mChannelContexts.CreateObject(this);
        if (channelContext == nullptr)
            return ChannelHandle{ nullptr };
        channelContext->Start(builder);
    }
    else
    {
        channelContext->Retain();
    }

    auto association = mChannelHandles.CreateObject(channelContext, delegate);
    channelContext->Release();
    return ChannelHandle{ association };
}

void ExchangeManager::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        SecureSessionHandle session, System::PacketBufferHandle msgBuf, SecureSessionMgr * msgLayer)
{
    DispatchMessage(session, packetHeader, payloadHeader, std::move(msgBuf));
}

void ExchangeManager::OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr)
{
    mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
        if (context->MatchesSession(session, mgr))
        {
            context->OnNewConnection(session);
            return false;
        }
        return true;
    });
}

void ExchangeManager::OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr)
{
    for (auto & ec : mContextPool)
    {
        if (ec.GetReferenceCount() > 0 && ec.mSecureSession == session)
        {
            ec.Close();
            // Continue iterate because there can be multiple contexts associated with the connection.
        }
    }

    mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
        if (context->MatchesSession(session, mgr))
        {
            context->OnConnectionExpired(session);
            return false;
        }
        return true;
    });
}

void ExchangeManager::OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                                        System::PacketBufferHandle msgBuf)
{
    auto peer = header.GetSourceNodeId();
    if (!peer.HasValue())
    {
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        source.ToString(addrBuffer, sizeof(addrBuffer));
        ChipLogError(ExchangeManager, "Unencrypted message from %s is dropped since no source node id in packet header.",
                     addrBuffer);
        return;
    }

    auto node     = peer.Value();
    auto notFound = mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
        if (context->MatchesPaseParingSessoin(node))
        {
            CHIP_ERROR err = context->HandlePairingMessage(header, source, std::move(msgBuf));
            if (err != CHIP_NO_ERROR)
                ChipLogError(ExchangeManager, "HandlePairingMessage error %s from node %llu.", chip::ErrorStr(err), node);
            return false;
        }
        return true;
    });

    if (notFound)
    {
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        source.ToString(addrBuffer, sizeof(addrBuffer));
        ChipLogError(ExchangeManager, "Unencrypted message from %s is dropped since no session found for node %llu.", addrBuffer,
                     node);
        return;
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

} // namespace Messaging
} // namespace chip
