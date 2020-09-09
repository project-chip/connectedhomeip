/*
 *
 *    Copyright (c) 2020 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements types and objects for managing CHIP session
 *      security state.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

#include "CHIPSecurityMgr.h"
#include <core/CHIPCore.h>
#include <message/CHIPServerBase.h>
#include <protocols/CHIPProtocols.h>
#include <protocols/common/CHIPMessage.h>
#include <protocols/common/CommonProtocol.h>
#include <protocols/security/CHIPSecurity.h>
#include <support/CHIPFaultInjection.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {

#if CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY

namespace Platform {
namespace Security {

// static inline void OnTimeConsumingCryptoStart() {}

// static inline void OnTimeConsumingCryptoDone() {}

} // namespace Security
} // namespace Platform

#endif // CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_DUMMY

using namespace chip::Protocols;
using namespace chip::Protocols::Common;
using namespace chip::Protocols::StatusReporting;
using namespace chip::Protocols::Security;
using namespace chip::Protocols::Security::AppKeys;
using namespace chip::Encoding;

ChipSecurityManager::ChipSecurityManager(void)
{
    State        = kState_NotInitialized;
    mSystemLayer = NULL;
}

CHIP_ERROR ChipSecurityManager::Init(ChipExchangeManager & aExchangeMgr, System::Layer & aSystemLayer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (State != kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    ExchangeManager         = &aExchangeMgr;
    mSystemLayer            = &aSystemLayer;
    SessionEstablishTimeout = CHIP_CONFIG_DEFAULT_SECURITY_SESSION_ESTABLISHMENT_TIMEOUT;
    IdleSessionTimeout      = CHIP_CONFIG_DEFAULT_SECURITY_SESSION_IDLE_TIMEOUT;
    FabricState             = aExchangeMgr.FabricState;
    OnSessionEstablished    = NULL;
    OnSessionError          = NULL;
    OnKeyErrorMsgRcvd       = NULL;
    mEC                     = NULL;
    mCon                    = NULL;
#if CHIP_CONFIG_SECURITY_TEST_MODE
    CASEUseKnownECDHKey = false;
#endif
    mStartSecureSession_OnComplete = NULL;
    mStartSecureSession_OnError    = NULL;
    mStartSecureSession_ReqState   = NULL;
    mRequestedAuthMode             = kChipAuthMode_NotSpecified;
    mSessionKeyId                  = ChipKeyId::kNone;
    mEncType                       = kChipEncryptionType_None;

    mFlags = 0;

    err = ExchangeManager->RegisterUnsolicitedMessageHandler(kChipProtocol_Security, HandleUnsolicitedMessage, this);
    SuccessOrExit(err);

    aExchangeMgr.MessageLayer->SecurityMgr = this;

    State = kState_Idle;

exit:
    return err;
}

CHIP_ERROR ChipSecurityManager::Shutdown(void)
{
    if (State != kState_NotInitialized)
    {
        ExchangeManager->UnregisterUnsolicitedMessageHandler(kChipProtocol_Security);
        ExchangeManager = NULL;

        // TODO: clean-up in-progress session establishment

        Reset();

        State = kState_NotInitialized;
    }

    return CHIP_NO_ERROR;
}

void ChipSecurityManager::HandleUnsolicitedMessage(ExchangeContext * ec, const IPPacketInfo * pktInfo,
                                                   const ChipMessageInfo * msgInfo, uint32_t profileId, uint8_t msgType,
                                                   PacketBuffer * msgBuf)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    ChipSecurityManager * secMgr = (ChipSecurityManager *) ec->AppState;

    // Handle Key Error Messages.
    if (profileId == kChipProtocol_Security && msgType == kMsgType_KeyError)
    {
        secMgr->HandleKeyErrorMsg(ec, msgBuf);
        msgBuf = NULL;
        ec     = NULL;

        ExitNow();
    }

    // Verify that we don't already have a session establishment in progress.
    VerifyOrExit(secMgr->State == kState_Idle, err = CHIP_ERROR_SECURITY_MANAGER_BUSY);

    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_SecMgrBusy, {
        secMgr->AsyncNotifySecurityManagerAvailable();
        ExitNow(err = CHIP_ERROR_SECURITY_MANAGER_BUSY);
    });

    if (!ec->HasPeerRequestedAck())
    {
        // Reject the request if it did not arrive over a connection.
        VerifyOrExit(ec->Con != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    }
    // Reject all other message types.
    else
        ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

exit:
    if (msgBuf != NULL)
        PacketBuffer::Free(msgBuf);
    if (ec != NULL)
    {
        if (err != CHIP_NO_ERROR)
            SendStatusReport(err, ec);
        ec->Release();
    }
}

/**
 * Checks if the specified CHIP error code is one of the key error codes.
 * This function is called to determine whether key error message should be sent
 * to the initiator of the message that failed to find a correct key during decoding.
 *
 * @param[in]  err        A CHIP error code.
 *
 * @retval     true       If specified CHIP error code is a key error.
 * @retval     false      Otherwise.
 *
 */
bool ChipSecurityManager::IsKeyError(CHIP_ERROR err)
{
    return (err == CHIP_ERROR_KEY_NOT_FOUND || err == CHIP_ERROR_WRONG_ENCRYPTION_TYPE || err == CHIP_ERROR_UNKNOWN_KEY_TYPE ||
            err == CHIP_ERROR_INVALID_USE_OF_SESSION_KEY || err == CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);

    // NOTE: This method should NOT return true for CHIP_ERROR_SESSION_KEY_SUSPENDED.
    // A suspended session is one that has been serialized for persistence.  Once this
    // has happened, subsequent inbound messages referencing the session should not
    // provoke a Key Error back to the sender, lest they cause the other end of the
    // session to abort.
}

/**
 * Send key error message.
 * This function is called when received CHIP message decoding fails due to key error.
 *
 * @param[in]  rcvdMsgInfo        A pointer to the message information for the received CHIP message.
 *
 * @param[in]  rcvdMsgPacketInfo  A pointer to the IPPacketInfo object of the received CHIP message.
 *
 * @param[in]  con                A pointer to the ChipConnection object.
 *
 * @param[in]  keyErr             CHIP key error code.
 *
 * @retval  #CHIP_ERROR_NO_MEMORY         If memory could not be allocated for the new
 *                                         exchange context or new message buffer.
 * @retval  #CHIP_ERROR_BUFFER_TOO_SMALL  If buffer is too small
 * @retval  #CHIP_NO_ERROR                If the method succeeded.
 *
 */
CHIP_ERROR ChipSecurityManager::SendKeyErrorMsg(ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo,
                                                ChipConnection * con, CHIP_ERROR keyErr)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    ExchangeContext * ec  = NULL;
    PacketBuffer * msgBuf = NULL;
    uint8_t * p;
    uint16_t keyErrCode;

    // Create new exchange context.
    if (con == NULL)
    {
        VerifyOrExit(rcvdMsgPacketInfo != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
        ec = ExchangeManager->NewContext(rcvdMsgInfo->SourceNodeId, rcvdMsgPacketInfo->SrcAddress, rcvdMsgPacketInfo->SrcPort,
                                         rcvdMsgPacketInfo->Interface, this);
    }
    else
    {
        ec = ExchangeManager->NewContext(con, this);
    }
    VerifyOrExit(ec != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Encode key error status code.
    switch (keyErr)
    {
    case CHIP_ERROR_KEY_NOT_FOUND:
        keyErrCode = kStatusCode_KeyNotFound;
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE:
        keyErrCode = kStatusCode_WrongEncryptionType;
        break;
    case CHIP_ERROR_UNKNOWN_KEY_TYPE:
        keyErrCode = kStatusCode_UnknownKeyType;
        break;
    case CHIP_ERROR_INVALID_USE_OF_SESSION_KEY:
        keyErrCode = kStatusCode_InvalidUseOfSessionKey;
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE:
        keyErrCode = kStatusCode_UnsupportedEncryptionType;
        break;
    default:
        keyErrCode = kStatusCode_InternalKeyError;
        break;
    }

    // Allocate new buffer.
    msgBuf = PacketBuffer::New();
    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Verify that the buffer is big enough for this message.
    VerifyOrExit(msgBuf->AvailableDataLength() >= kChipKeyErrorMessageSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // Initialize pointer to the buffer payload start.
    p = msgBuf->Start();

    // Write the key Id field (2 bytes).
    LittleEndian::Write16(p, rcvdMsgInfo->KeyId);

    // Write the encryption type field (1 byte).
    Write8(p, rcvdMsgInfo->EncryptionType);

    // Write the message Id field (4 bytes).
    LittleEndian::Write32(p, rcvdMsgInfo->MessageId);

    // Write the key error code field (2 bytes).
    LittleEndian::Write16(p, keyErrCode);

    // Set message length.
    msgBuf->SetDataLength(kChipKeyErrorMessageSize);

    // Send key error message.
    err    = ec->SendMessage(kChipProtocol_Security, kMsgType_KeyError, msgBuf, 0);
    msgBuf = NULL;

exit:
    if (msgBuf != NULL)
        PacketBuffer::Free(msgBuf);

    if (ec != NULL)
        ec->Close();

    return err;
}

void ChipSecurityManager::HandleKeyErrorMsg(ExchangeContext * ec, PacketBuffer * msgBuf)
{
    CHIP_ERROR err;
    ChipSessionKey * sessionKey;
    uint8_t * p             = msgBuf->Start();
    uint64_t srcNodeId      = ec->PeerNodeId;
    ChipConnection * msgCon = ec->Con;
    uint16_t keyId;
    uint8_t encType;
    uint16_t keyErrCode;
    uint32_t messageId;
    CHIP_ERROR keyErr;
    uint64_t endNodeIds[CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION + 1];
    uint8_t endNodeIdsCount = 0;

    // Verify correct message size.
    if (msgBuf->DataLength() != kChipKeyErrorMessageSize)
        ExitNow();

    // Read the message fields.
    keyId      = LittleEndian::Read16(p);
    encType    = Read8(p);
    messageId  = LittleEndian::Read32(p);
    keyErrCode = LittleEndian::Read16(p);

    // Free the received message buffer so that it can be reused to initiate additional communication.
    PacketBuffer::Free(msgBuf);
    msgBuf = NULL;

    // Close exchange context that delivered key error message.
    ec->Close();
    ec = NULL;

    // Decode error status code.
    switch (keyErrCode)
    {
    case kStatusCode_KeyNotFound:
        keyErr = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER;
        break;
    case kStatusCode_WrongEncryptionType:
        keyErr = CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER;
        break;
    case kStatusCode_UnknownKeyType:
        keyErr = CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER;
        break;
    case kStatusCode_InvalidUseOfSessionKey:
        keyErr = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER;
        break;
    case kStatusCode_UnsupportedEncryptionType:
        keyErr = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER;
        break;
    default:
        keyErr = CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER;
        break;
    }

    // If the failed key is a session key...
    if (ChipKeyId::IsSessionKey(keyId))
    {
        // Attempt to find the referenced session key.  If found...
        err = FabricState->FindSessionKey(keyId, srcNodeId, false, sessionKey);
        if (err == CHIP_NO_ERROR)
        {
            // Ignore KeyError if it wasn't received over the same transport mechanism
            // as was used to establish the security session.
            if (msgCon != sessionKey->BoundCon)
                ExitNow();

            // Ignore KeyError if the message Id value presented in the Key Error message
            // doesn't correspond to the last used message id.
            if (messageId != sessionKey->NextMsgId.GetValue() - 1)
                ExitNow();

            // If the key refers to a shared session, add the shared session end nodes to the list
            // of peer nodes associated with the key.
            if (sessionKey->IsSharedSession())
            {
                FabricState->GetSharedSessionEndNodeIds(sessionKey, endNodeIds, sizeof(endNodeIds) / sizeof(uint64_t),
                                                        endNodeIdsCount);
            }

            // Add the terminating node to the list of peer nodes associated with the key.
            endNodeIds[endNodeIdsCount++] = sessionKey->NodeId;

            // Discard the failed session key.
            FabricState->RemoveSessionKey(keyId, srcNodeId);
        }
    }

    // Otherwise, the key is some other form of key (e.g. a group key), so add the node that sent the
    // key error to the list of peers associated with the key.
    else
    {
        endNodeIds[endNodeIdsCount++] = srcNodeId;
    }

    // For each peer node associated with the key, notify the exchange manager that the key has failed
    // with respect to that peer.
    for (int i = 0; i < endNodeIdsCount; i++)
        ExchangeManager->NotifyKeyFailed(endNodeIds[i], keyId, keyErr);

    // TODO: fail the current in-progress session if the key error identifies the proposed key.

    // Call the general key error message handler.
    if (OnKeyErrorMsgRcvd != NULL)
        OnKeyErrorMsgRcvd(keyId, encType, messageId, srcNodeId, keyErr);

exit:
    if (msgBuf != NULL)
        PacketBuffer::Free(msgBuf);

    if (ec != NULL)
        ec->Close();

    return;
}

CHIP_ERROR ChipSecurityManager::NewSessionExchange(uint64_t peerNodeId, IPAddress peerAddr, uint16_t peerPort)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mEC != NULL)
    {
        mEC->Close();
        mEC = NULL;
    }

    // Create a new exchange context.
    if (mCon)
    {
        mEC = ExchangeManager->NewContext(mCon, this);
        VerifyOrExit(mEC != NULL, err = CHIP_ERROR_NO_MEMORY);
    }
    else
    {
        VerifyOrExit(peerNodeId != kNodeIdNotSpecified && peerNodeId != kAnyNodeId, err = CHIP_ERROR_INVALID_ARGUMENT);

        mEC = ExchangeManager->NewContext(peerNodeId, peerAddr, peerPort, INET_NULL_INTERFACEID, this);
        VerifyOrExit(mEC != NULL, err = CHIP_ERROR_NO_MEMORY);

        mEC->OnAckRcvd   = RMPHandleAckRcvd;
        mEC->OnSendError = RMPHandleSendError;
    }

exit:
    return err;
}

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

// Create and initialize new exchange for the message counter synchronization request/response messages.
CHIP_ERROR ChipSecurityManager::NewMsgCounterSyncExchange(const ChipMessageInfo * rcvdMsgInfo,
                                                          const IPPacketInfo * rcvdMsgPacketInfo, ExchangeContext *& ec)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Verify input arguments.
    VerifyOrExit(rcvdMsgInfo != NULL && rcvdMsgPacketInfo != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Message counter synchronization protocol is only applicable for application group keys.
    VerifyOrExit(ChipKeyId::IsAppGroupKey(rcvdMsgInfo->KeyId), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Create new exchange context.
    ec = ExchangeManager->NewContext(rcvdMsgInfo->SourceNodeId, rcvdMsgPacketInfo->SrcAddress, rcvdMsgPacketInfo->SrcPort,
                                     rcvdMsgPacketInfo->Interface, this);
    VerifyOrExit(ec != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Set encryption type and key Id.
    ec->EncryptionType = rcvdMsgInfo->EncryptionType;
    ec->KeyId          = rcvdMsgInfo->KeyId;

exit:
    return err;
}

/**
 * Send peer message counter synchronization request.
 * This function is called while processing a message encrypted with an application key from a peer whose message counter is not
 * synchronized. This message is sent on a newly created exchange, which is closed immediately after.
 *
 * @param[in]  rcvdMsgInfo        A pointer to the message information for the received CHIP message.
 *
 * @param[in]  rcvdMsgPacketInfo  A pointer to the IPPacketInfo object of the received CHIP message.
 *
 * @retval  #CHIP_ERROR_INVALID_ARGUMENT  If keyId input has invalid value.
 * @retval  #CHIP_ERROR_NO_MEMORY         If memory could not be allocated for the new
 *                                         exchange context or new message buffer.
 * @retval  #CHIP_NO_ERROR                On success.
 *
 */
CHIP_ERROR ChipSecurityManager::SendSolitaryMsgCounterSyncReq(const ChipMessageInfo * rcvdMsgInfo,
                                                              const IPPacketInfo * rcvdMsgPacketInfo)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    ExchangeContext * ec = NULL;

    // Create and initialize new exchange.
    err = NewMsgCounterSyncExchange(rcvdMsgInfo, rcvdMsgPacketInfo, ec);
    SuccessOrExit(err);

    // Send the message counter synchronization request in a Common::Null message.
    err = ec->SendCommonNullMessage();
    SuccessOrExit(err);

exit:
    if (ec != NULL)
        ec->Close();

    return err;
}

/**
 * Send message counter synchronization response message.
 * This function is called when CHIP node receives message with message counter synchronization request flag.
 * This message is sent on a newly created exchange, which is closed immediately after.
 *
 * @param[in]  rcvdMsgInfo        A pointer to the message information for the received CHIP message.
 *
 * @param[in]  rcvdMsgPacketInfo  A pointer to the IPPacketInfo object of the received CHIP message.
 *
 * @retval  #CHIP_ERROR_INVALID_ARGUMENT  If received message with message counter synchronization
 *                                         request was unencrypted.
 * @retval  #CHIP_ERROR_NO_MEMORY         If memory could not be allocated for the new
 *                                         exchange context or new message buffer.
 * @retval  #CHIP_ERROR_BUFFER_TOO_SMALL  If allocated message buffer is too small.
 * @retval  #CHIP_NO_ERROR                On success.
 *
 */
CHIP_ERROR ChipSecurityManager::SendMsgCounterSyncResp(const ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    ExchangeContext * ec  = NULL;
    PacketBuffer * msgBuf = NULL;

    // Create and initialize new exchange.
    err = NewMsgCounterSyncExchange(rcvdMsgInfo, rcvdMsgPacketInfo, ec);
    SuccessOrExit(err);

    // Allocate new buffer.
    msgBuf = PacketBuffer::New();
    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Verify that the buffer is big enough for this message.
    VerifyOrExit(msgBuf->AvailableDataLength() >= kChipMsgCounterSyncRespMsgSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    // Write the requester message id (counter) field.
    LittleEndian::Put32(msgBuf->Start(), rcvdMsgInfo->MessageId);

    // Set message length.
    msgBuf->SetDataLength(kChipMsgCounterSyncRespMsgSize);

    // Send message counter synchronization response message.
    err    = ec->SendMessage(kChipProtocol_Security, kMsgType_MsgCounterSyncResp, msgBuf);
    msgBuf = NULL;

exit:
    if (msgBuf != NULL)
        PacketBuffer::Free(msgBuf);

    if (ec != NULL)
        ec->Close();

    return err;
}

/**
 * Handle message counter synchronization response message.
 *
 * @param[in]  msgInfo        A pointer to the message information for the received CHIP message.
 *
 * @param[in]  msgBuf         A pointer to the PacketBuffer object holding the received CHIP message.
 *
 * @retval None.
 *
 */
void ChipSecurityManager::HandleMsgCounterSyncRespMsg(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf)
{
    // Verify correct message size and that the message was encrypted with application group key.
    VerifyOrExit(msgBuf->DataLength() == kChipMsgCounterSyncRespMsgSize && ChipKeyId::IsAppGroupKey(msgInfo->KeyId), /* no-op */);

    // Initialize/synchronize peer's message counter.
    FabricState->OnMsgCounterSyncRespRcvd(msgInfo->SourceNodeId, msgInfo->MessageId, LittleEndian::Get32(msgBuf->Start()));

exit:
    PacketBuffer::Free(msgBuf);

    return;
}

#endif // CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

CHIP_ERROR ChipSecurityManager::HandleSessionEstablished(void)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    uint64_t peerNodeId   = mEC->PeerNodeId;
    uint16_t sessionKeyId = mSessionKeyId;
    uint8_t encType       = mEncType;
    const ChipEncryptionKey * sessionKey;
    ChipAuthMode authMode;

    switch (State)
    {
    default:
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    // Save the session key into the session key table.
    err = FabricState->SetSessionKey(sessionKeyId, peerNodeId, encType, authMode, sessionKey);
    SuccessOrExit(err);

exit:
    return err;
}

void ChipSecurityManager::HandleSessionComplete(void)
{
    ChipConnection * con                   = mCon;
    uint64_t peerNodeId                    = mEC->PeerNodeId;
    uint16_t sessionKeyId                  = mSessionKeyId;
    uint8_t encType                        = mEncType;
    SessionEstablishedFunct userOnComplete = mStartSecureSession_OnComplete;
    void * reqState                        = mStartSecureSession_ReqState;

    // Reset state.
    Reset();

    // Call the general session established handler.
    if (OnSessionEstablished != NULL)
        OnSessionEstablished(this, con, NULL, sessionKeyId, peerNodeId, encType);

    // Call the user's completion function.
    if (userOnComplete != NULL)
        userOnComplete(this, con, reqState, sessionKeyId, peerNodeId, encType);

    // If the session was initiated the remote party, release the reservation that was
    // made when the session key record was allocated.  Provided that the application
    // hasn't increased the reservation count during one of the above callbacks,
    // this will result in the reservation count going to zero, which will make
    // eligible for removal if it remains idle past the idle timeout period.
    {
        ChipSessionKey * sessionKey;
        if (FabricState->FindSessionKey(sessionKeyId, peerNodeId, false, sessionKey) == CHIP_NO_ERROR &&
            !sessionKey->IsLocallyInitiated())
        {
            ReleaseSessionKey(sessionKey);
        }
    }

    // Asynchronously notify other subsystems that the security manager is now available
    // for initiating additional sessions.
    AsyncNotifySecurityManagerAvailable();
}

void ChipSecurityManager::HandleSessionError(CHIP_ERROR err, PacketBuffer * statusReportMsgBuf)
{
    // If session establishment in progress...
    //
    // NOTE: This check is necessary because it is possible for HandleSessionError() to be
    // called twice in certain situations: If a call to ExchangeContext::SendMessage() fails
    // because the underlying connection has been closed, it will trigger a callback, while
    // in the SendMessage() function, to HandleConnectionClosed() which calls this function.
    // Then when SendMessage() returns, the function that called it will also call this
    // function with the error returned by SendMessage().
    //
    if (State != kState_Idle)
    {
        ChipConnection * con          = mCon;
        uint64_t peerNodeId           = mEC->PeerNodeId;
        uint16_t sessionKeyId         = mSessionKeyId;
        SessionErrorFunct userOnError = mStartSecureSession_OnError;
        void * reqState               = mStartSecureSession_ReqState;
        StatusReport rcvdStatusReport;
        StatusReport * statusReportPtr = NULL;

        // If a status report was received from the peer, parse it and arrange to pass it
        // to the callbacks.
        if (err == CHIP_ERROR_STATUS_REPORT_RECEIVED)
        {
            CHIP_ERROR parseErr = StatusReport::parse(statusReportMsgBuf, rcvdStatusReport);
            if (parseErr == CHIP_NO_ERROR)
                statusReportPtr = &rcvdStatusReport;
            else
                err = parseErr;
        }

        // Otherwise, send a status report to the peer with our reason for the failure.
        else
            SendStatusReport(err, mEC);

        // Remove the session key from the key table.
        FabricState->RemoveSessionKey(sessionKeyId, peerNodeId);

        // Reset state.
        Reset();

        // Call the general session error handler.
        if (OnSessionError != NULL)
            OnSessionError(this, con, NULL, err, peerNodeId, statusReportPtr);

        // Call the user's error handler.
        if (userOnError != NULL)
            userOnError(this, con, reqState, err, peerNodeId, statusReportPtr);

        // Asynchronously notify other subsystems that the security manager is now available
        // for initiating another session.
        AsyncNotifySecurityManagerAvailable();
    }
}

void ChipSecurityManager::HandleConnectionClosed(ExchangeContext * ec, ChipConnection * con, CHIP_ERROR conErr)
{
    ChipSecurityManager * secMgr = (ChipSecurityManager *) ec->AppState;

    if (conErr == CHIP_NO_ERROR)
        conErr = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;

    // Clean-up the local state and invoke the appropriate callbacks.
    secMgr->HandleSessionError(conErr, NULL);
}

CHIP_ERROR ChipSecurityManager::SendStatusReport(CHIP_ERROR localErr, ExchangeContext * ec)
{
    CHIP_ERROR err;
    uint32_t profileId;
    uint16_t statusCode;
    uint16_t sendFlags;

    // Verify that specified exchange isn't closed.
    VerifyOrExit(ec != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (ec->Con != NULL)
    {
        // Verify that underlying connection isn't closed.
        VerifyOrExit(!ec->IsConnectionClosed(), err = CHIP_ERROR_INVALID_ARGUMENT);
        sendFlags = 0;
    }
    else
    {
        sendFlags = ExchangeContext::kSendFlag_RequestAck;
    }

    // TODO: map CASE errors

    switch (localErr)
    {
    case CHIP_ERROR_INCORRECT_STATE:
    case CHIP_ERROR_INVALID_MESSAGE_TYPE:
        profileId  = kChipProtocol_Common;
        statusCode = kStatus_UnexpectedMessage;
        break;
    case CHIP_ERROR_NOT_IMPLEMENTED:
        profileId  = kChipProtocol_Common;
        statusCode = kStatus_UnsupportedMessage;
        break;
    case CHIP_ERROR_SECURITY_MANAGER_BUSY:
    case CHIP_ERROR_RATE_LIMIT_EXCEEDED:
        profileId  = kChipProtocol_Common;
        statusCode = kStatus_Busy;
        break;
    case CHIP_ERROR_TIMEOUT:
        profileId  = kChipProtocol_Common;
        statusCode = kStatus_Timeout;
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_UnsupportedEncryptionType;
        break;
    case CHIP_ERROR_WRONG_KEY_TYPE:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_InvalidKeyId;
        break;
    case CHIP_ERROR_DUPLICATE_KEY_ID:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_DuplicateKeyId;
        break;
    case CHIP_ERROR_KEY_CONFIRMATION_FAILED:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_KeyConfirmationFailed;
        break;
    case CHIP_ERROR_INVALID_PASE_PARAMETER:
    case CHIP_ERROR_CERT_USAGE_NOT_ALLOWED:
    case CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED:
    case CHIP_ERROR_CERT_NOT_VALID_YET:
    case CHIP_ERROR_CERT_EXPIRED:
    case CHIP_ERROR_CERT_PATH_TOO_LONG:
    case CHIP_ERROR_CA_CERT_NOT_FOUND:
    case CHIP_ERROR_INVALID_SIGNATURE:
    case CHIP_ERROR_CERT_NOT_TRUSTED:
    case CHIP_ERROR_WRONG_CERT_SUBJECT:
    case CHIP_ERROR_WRONG_CERT_TYPE:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_AuthenticationFailed;
        break;
    case CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_PASESupportsOnlyConfig1;
        break;
    case CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_NoCommonPASEConfigurations;
        break;
    case CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_UnsupportedCASEConfiguration;
        break;
    case CHIP_ERROR_UNSUPPORTED_CERT_FORMAT:
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_UnsupportedCertificate;
        break;
    default:
        ChipLogError(SecurityManager, "Internal security error %d", localErr);
        profileId  = kChipProtocol_Security;
        statusCode = kStatusCode_InternalError;
        break;
    }

    // TODO: add support for conveying system error
    // (HOWEVER, be careful to suppress error information that should not be revealed to the peer).

    err = chip::ChipServerBase::SendStatusReport(ec, profileId, statusCode, CHIP_NO_ERROR, sendFlags);
    SuccessOrExit(err);

exit:
    return err;
}

void ChipSecurityManager::Reset(void)
{
    if (mEC != NULL)
    {
        mEC->Abort();
        mEC = NULL;
    }

    switch (State)
    {
    default:
        break;
    }

    chip::Platform::MemoryShutdown();

    CancelSessionTimer();

    State                          = kState_Idle;
    mCon                           = NULL;
    mRequestedAuthMode             = kChipAuthMode_NotSpecified;
    mSessionKeyId                  = ChipKeyId::kNone;
    mEncType                       = kChipEncryptionType_None;
    mStartSecureSession_OnComplete = NULL;
    mStartSecureSession_OnError    = NULL;
    mStartSecureSession_ReqState   = NULL;
}

void ChipSecurityManager::StartSessionTimer(void)
{
    ChipLogProgress(SecurityManager, "%s", __FUNCTION__);

    if (SessionEstablishTimeout != 0)
    {
        mSystemLayer->StartTimer(SessionEstablishTimeout, HandleSessionTimeout, this);
    }
}

void ChipSecurityManager::CancelSessionTimer(void)
{
    ChipLogProgress(SecurityManager, "%s", __FUNCTION__);
    mSystemLayer->CancelTimer(HandleSessionTimeout, this);
}

void ChipSecurityManager::HandleSessionTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError)
{
    ChipLogProgress(SecurityManager, "%s", __FUNCTION__);

    ChipSecurityManager * securityMgr = reinterpret_cast<ChipSecurityManager *>(aAppState);
    if (securityMgr)
    {
        securityMgr->HandleSessionError(CHIP_ERROR_TIMEOUT, NULL);
    }
}

void ChipSecurityManager::StartIdleSessionTimer(void)
{
    if (IdleSessionTimeout != 0 && !GetFlag(mFlags, kFlag_IdleSessionTimerRunning))
    {
        System::Layer * systemLayer = FabricState->MessageLayer->SystemLayer;
        System::Error err           = systemLayer->StartTimer(IdleSessionTimeout, HandleIdleSessionTimeout, this);
        if (err == CHIP_SYSTEM_NO_ERROR)
        {
            ChipLogDetail(SecurityManager, "Session idle timer started");
            SetFlag(mFlags, kFlag_IdleSessionTimerRunning);
        }
    }
}

void ChipSecurityManager::StopIdleSessionTimer(void)
{
    System::Layer * systemLayer = FabricState->MessageLayer->SystemLayer;
    systemLayer->CancelTimer(HandleIdleSessionTimeout, this);
    ClearFlag(mFlags, kFlag_IdleSessionTimerRunning);
    ChipLogDetail(SecurityManager, "Session idle timer stopped");
}

void ChipSecurityManager::HandleIdleSessionTimeout(System::Layer * aLayer, void * aAppState, System::Error aError)
{
    ChipSecurityManager * _this = (ChipSecurityManager *) aAppState;
    bool unreservedSessionsExist;

    ClearFlag(_this->mFlags, kFlag_IdleSessionTimerRunning);

    unreservedSessionsExist = _this->FabricState->RemoveIdleSessionKeys();

    if (unreservedSessionsExist)
    {
        _this->StartIdleSessionTimer();
    }
}

void ChipSecurityManager::OnEncryptedMsgRcvd(uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType)
{
    // TODO(#2248): implement against SCTT defined security protocol
}

void ChipSecurityManager::RMPHandleAckRcvd(ExchangeContext * ec, void * msgCtxt)
{
    ChipLogProgress(SecurityManager, "%s", __FUNCTION__);
}

void ChipSecurityManager::RMPHandleSendError(ExchangeContext * ec, CHIP_ERROR err, void * msgCtxt)
{
    ChipLogProgress(SecurityManager, "%s", __FUNCTION__);
    ChipSecurityManager * secMgr = (ChipSecurityManager *) ec->AppState;

    secMgr->HandleSessionError(err, NULL);
}

void ChipSecurityManager::AsyncNotifySecurityManagerAvailable()
{
    mSystemLayer->ScheduleWork(DoNotifySecurityManagerAvailable, this);
}

void ChipSecurityManager::DoNotifySecurityManagerAvailable(System::Layer * systemLayer, void * appState, System::Error err)
{
    ChipSecurityManager * _this = (ChipSecurityManager *) appState;
    if (_this->State == kState_Idle)
    {
        _this->ExchangeManager->NotifySecurityManagerAvailable();
    }
}

/**
 * Cancel an in-progress session establishment.
 *
 * @param[in]  reqState         A pointer value that matches the value supplied by the application
 *                              when the session was started.
 *
 * @retval #CHIP_NO_ERROR      If a matching in-progress session establishment was found and canceled.
 *
 * @retval #CHIP_ERROR_INCORRECT_STATE   If there was no session establishment in progress, or the
 *                              in-progress session did not match the supplied request state pointer.
 */
CHIP_ERROR ChipSecurityManager::CancelSessionEstablishment(void * reqState)
{
    // If a session establishment is in progress and the supplied request state matches what was provided
    // when the session was started...
    if ((State == kState_CASEInProgress || State == kState_PASEInProgress || State == kState_TAKEInProgress) &&
        reqState == mStartSecureSession_ReqState)
    {
        // Clear the application's OnError handler to prevent a callback.
        mStartSecureSession_OnError = NULL;

        // Fail the session with a canceled error.
        HandleSessionError(CHIP_ERROR_TRANSACTION_CANCELED, NULL);

        return CHIP_NO_ERROR;
    }

    // Otherwise, tell the caller there was no match.
    else
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

/**
 * Place a reservation on a message encryption key.
 *
 * Key reservations are used to signal that a particular key is actively in use and should be retained.
 * Note that placing reservation on a key does not guarantee that the key wont be removed by an explicit
 * action such as the reception of a KeyError message.
 *
 * For every reservation placed on a particular key, a corresponding call to ReleaseKey() must be made.
 *
 * This method accepts any form of key id, including None. Key ids that do not name actual keys are ignored.
 *
 * @param[in]  peerNodeId       The CHIP node id of the peer with which the key shared.
 *
 * @param[in]  keyId            The id of the key to be reserved.
 *
 */
void ChipSecurityManager::ReserveKey(uint64_t peerNodeId, uint16_t keyId)
{
    // If the key is a session key, attempt to locate the specified key and increase its reservation count.
    // (Currently reservations only apply to session keys).
    if (ChipKeyId::IsSessionKey(keyId))
    {
        ChipSessionKey * sessionKey;
        if (FabricState->FindSessionKey(keyId, peerNodeId, false, sessionKey) == CHIP_NO_ERROR)
        {
            ReserveSessionKey(sessionKey);
        }
    }
}

/**
 * Release a message encryption key reservation.
 *
 * Release a reservations that was previously placed on a message encryption key.
 *
 * For every reservation placed on a particular key, the ReleaseKey() method must be called no more than once.
 *
 * This method accepts any form of key id, including None. Key ids that do not name actual keys are ignored.
 *
 * @param[in]  peerNodeId       The CHIP node id of the peer with which the key shared.
 *
 * @param[in]  keyId            The id of the key whose reservation should be released.
 *
 */
void ChipSecurityManager::ReleaseKey(uint64_t peerNodeId, uint16_t keyId)
{
    // If the key is a session key, attempt to locate the specified key and decrease its reservation count.
    // (Currently reservations only apply to session keys).
    if (ChipKeyId::IsSessionKey(keyId))
    {
        ChipSessionKey * sessionKey;
        if (FabricState->FindSessionKey(keyId, peerNodeId, false, sessionKey) == CHIP_NO_ERROR)
        {
            ReleaseSessionKey(sessionKey);
        }
    }
}

/**
 * Place a reservation on a session key.
 *
 * @param[in]  sessionKey       A pointer to the session key to be reserved.
 *
 */
void ChipSecurityManager::ReserveSessionKey(ChipSessionKey * sessionKey)
{
    VerifyOrDie(sessionKey->ReserveCount < UINT8_MAX);
    sessionKey->ReserveCount++;
    sessionKey->MarkRecentlyActive();
    ChipLogDetail(SecurityManager, "Reserve session key: Id=%04" PRIX16 " Peer=%016" PRIX64 " Reserve=%" PRId8,
                  sessionKey->MsgEncKey.KeyId, sessionKey->NodeId, sessionKey->ReserveCount);
}

/**
 * Release a reservation on a session key.
 *
 * @param[in]  sessionKey       A pointer to the session key to be released.
 *
 */
void ChipSecurityManager::ReleaseSessionKey(ChipSessionKey * sessionKey)
{
    VerifyOrDie(sessionKey->ReserveCount > 0);

    sessionKey->ReserveCount--;

    ChipLogDetail(SecurityManager, "Release session key: Id=%04" PRIX16 " Peer=%016" PRIX64 " Reserve=%" PRId8,
                  sessionKey->MsgEncKey.KeyId, sessionKey->NodeId, sessionKey->ReserveCount);

    // If the session key is subject to automatic removal and its reserve count is now zero...
    if (sessionKey->BoundCon == NULL && sessionKey->IsKeySet() && sessionKey->ReserveCount == 0)
    {
        // If the session key is marked remove-on-idle, enable the idle session timer and mark the key as
        // recently active.  This will give it the maximum lifetime before it gets removed for inactivity.
        if (sessionKey->IsRemoveOnIdle())
        {
            StartIdleSessionTimer();
            sessionKey->MarkRecentlyActive();
        }

        // Otherwise remove the session key immediately.
        else
        {
            FabricState->RemoveSessionKey(sessionKey);
        }
    }
}

} // namespace chip
