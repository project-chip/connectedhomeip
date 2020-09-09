/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines types and objects for managing CHIP session
 *      security state.
 *
 */

#ifndef CHIPSECURITYMANAGER_H_
#define CHIPSECURITYMANAGER_H_

#include <core/CHIPError.h>
#include <message/CHIPExchangeMgr.h>
#include <protocols/common/CHIPMessage.h>
#include <protocols/status-report/StatusReportProtocol.h>
#include <support/DLLUtil.h>

/**
 *   @namespace chip::Platform::Security
 *
 *   @brief
 *     Functions in this namespace are to be implemented by platforms that use CHIP,
 *     according to the needs/constraints of the particular environment.
 *
 */

namespace chip {

namespace Platform {
namespace Security {

#if CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM

/**
 * This function is called to notify the application when a time-consuming
 * cryptographic operation is about to start.
 *
 * @note If application wants to receive these alerts and adjust platform settings
 *       accordingly then it should provide its own implementation of these functions
 *       and enable (1) #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM option.
 *
 */
extern void OnTimeConsumingCryptoStart(void);

/**
 * This function is called to notify the application when a time-consuming
 * cryptographic operation has just finished.
 *
 * @note If application wants to receive these alerts and adjust platform settings
 *       accordingly then it should provide its own implementation of these functions
 *       and enable (1) #CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM option.
 *
 */
extern void OnTimeConsumingCryptoDone(void);

#endif // CHIP_CONFIG_SECURITY_MGR_TIME_ALERTS_PLATFORM

} // namespace Security
} // namespace Platform

using chip::Protocols::StatusReporting::StatusReport;

class DLL_EXPORT ChipSecurityManager
{
public:
    enum State
    {
        kState_NotInitialized = 0,
        kState_Idle,
        kState_CASEInProgress,
        kState_PASEInProgress,
        kState_TAKEInProgress,
        kState_KeyExportInProgress
    };

    ChipFabricState * FabricState;         // [READ ONLY] Associated Fabric State object.
    ChipExchangeManager * ExchangeManager; // [READ ONLY] Associated Exchange Manager object.
    uint8_t State;                         // [READ ONLY] State of the CHIP Message Layer object
#if CHIP_CONFIG_SECURITY_TEST_MODE
    bool CASEUseKnownECDHKey; // Enable the use of a known ECDH key pair in CASE to allow man-in-the-middle
                              // key recovery for testing purposes.
#endif
    uint32_t SessionEstablishTimeout; // The amount of time after which an in-progress session establishment will timeout.
    uint32_t IdleSessionTimeout;      // The amount of time after which an idle session will be removed.

    ChipSecurityManager(void);

    CHIP_ERROR Init(ChipExchangeManager & aExchangeMgr, System::Layer & aSystemLayer);
    CHIP_ERROR Shutdown(void);

    typedef void (*SessionEstablishedFunct)(ChipSecurityManager * sm, ChipConnection * con, void * reqState, uint16_t sessionKeyId,
                                            uint64_t peerNodeId, uint8_t encType);
    typedef void (*SessionErrorFunct)(ChipSecurityManager * sm, ChipConnection * con, void * reqState, CHIP_ERROR localErr,
                                      uint64_t peerNodeId, StatusReport * statusReport);

    /**
     * Type of key error message handling function.
     *
     * @param[in] keyId         Encryption key caused the key error message response from the peer.
     * @param[in] encType       Encryption type associated with @a keyId.
     * @param[in] messageId     The identifier of the CHIP message resulted in the key error response from the peer.
     * @param[in] peerNodeId    The identifier of the CHIP node that sent key error message.
     * @param[in] keyErr        The error code received from the peer.
     *
     */
    typedef void (*KeyErrorMsgRcvdFunct)(uint16_t keyId, uint8_t encType, uint32_t messageId, uint64_t peerNodeId,
                                         CHIP_ERROR keyErr);

    // General callback functions. These will be called when a secure session is established or fails.
    SessionEstablishedFunct OnSessionEstablished;
    SessionErrorFunct OnSessionError;

    /**
     * The key error callback function. This function is called when
     * a key error message is received.
     */
    KeyErrorMsgRcvdFunct OnKeyErrorMsgRcvd;

    // Determine whether CHIP error code is a key error.
    bool IsKeyError(CHIP_ERROR err);

    // Send key error message when correct key has not been found and the message cannot be decrypted.
    CHIP_ERROR SendKeyErrorMsg(ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo, ChipConnection * con,
                               CHIP_ERROR keyErr);

    void OnEncryptedMsgRcvd(uint16_t sessionKeyId, uint64_t peerNodeId, uint8_t encType);

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    // Send message counter synchronization message.
    CHIP_ERROR SendMsgCounterSyncResp(const ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo);

    // Send peer message counter synchronization request.
    CHIP_ERROR SendSolitaryMsgCounterSyncReq(const ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo);

    // Handle message counter synchronization response message.
    void HandleMsgCounterSyncRespMsg(ChipMessageInfo * msgInfo, PacketBuffer * msgBuf);
#endif

    CHIP_ERROR CancelSessionEstablishment(void * reqState);

    void ReserveKey(uint64_t peerNodeId, uint16_t keyId);
    void ReleaseKey(uint64_t peerNodeId, uint16_t keyId);

private:
    enum Flags
    {
        kFlag_IdleSessionTimerRunning = 0x01
    };

    ExchangeContext * mEC;
    ChipConnection * mCon;
    union
    {
        SessionEstablishedFunct mStartSecureSession_OnComplete;
    };
    union
    {
        SessionErrorFunct mStartSecureSession_OnError;
    };
    union
    {
        void * mStartSecureSession_ReqState;
        void * mStartKeyExport_ReqState;
    };
    uint16_t mSessionKeyId;
    ChipAuthMode mRequestedAuthMode;
    uint8_t mEncType;
    System::Layer * mSystemLayer;
    uint8_t mFlags;

    void StartSessionTimer(void);
    void CancelSessionTimer(void);
    static void HandleSessionTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    void StartIdleSessionTimer(void);
    void StopIdleSessionTimer(void);
    static void HandleIdleSessionTimeout(System::Layer * aLayer, void * aAppState, System::Error aError);

    static void HandleUnsolicitedMessage(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                                         uint32_t profileId, uint8_t msgType, PacketBuffer * msgBuf);

    void HandleKeyErrorMsg(ExchangeContext * ec, PacketBuffer * msgBuf);

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    CHIP_ERROR NewMsgCounterSyncExchange(const ChipMessageInfo * rcvdMsgInfo, const IPPacketInfo * rcvdMsgPacketInfo,
                                         ExchangeContext *& ec);
#endif
    CHIP_ERROR NewSessionExchange(uint64_t peerNodeId, IPAddress peerAddr, uint16_t peerPort);
    CHIP_ERROR HandleSessionEstablished(void);
    void HandleSessionComplete(void);
    void HandleSessionError(CHIP_ERROR err, PacketBuffer * statusReportMsgBuf);
    static void HandleConnectionClosed(ExchangeContext * ec, ChipConnection * con, CHIP_ERROR conErr);
    static CHIP_ERROR SendStatusReport(CHIP_ERROR localError, ExchangeContext * ec);
    static void RMPHandleAckRcvd(ExchangeContext * ec, void * msgCtxt);
    static void RMPHandleSendError(ExchangeContext * ec, CHIP_ERROR err, void * msgCtxt);

    void Reset(void);

    void AsyncNotifySecurityManagerAvailable();
    static void DoNotifySecurityManagerAvailable(System::Layer * systemLayer, void * appState, System::Error err);

    void ReserveSessionKey(ChipSessionKey * sessionKey);
    void ReleaseSessionKey(ChipSessionKey * sessionKey);
};

} // namespace chip

#endif /* CHIPSECURITYMANAGER_H_ */
