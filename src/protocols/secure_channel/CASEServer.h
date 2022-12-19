/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/CertificateValidityPolicy.h>
#include <credentials/GroupDataProvider.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/CASESession.h>

namespace chip {

class CASEServer : public SessionEstablishmentDelegate,
                   public Messaging::UnsolicitedMessageHandler,
                   public Messaging::ExchangeDelegate
{
public:
    CASEServer() {}
    ~CASEServer() override { Shutdown(); }

    /*
     * This method will shutdown this object, releasing the strong reference to the pinned SecureSession object.
     * It will also unregister the unsolicited handler and clear out the session object (which will release the weak
     * reference through the underlying SessionHolder).
     *
     */
    void Shutdown()
    {
        if (mExchangeManager != nullptr)
        {
            mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1);
            mExchangeManager = nullptr;
        }

        GetSession().Clear();
        mPinnedSecureSession.ClearValue();
    }

    CHIP_ERROR ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, SessionManager * sessionManager,
                                             FabricTable * fabrics, SessionResumptionStorage * sessionResumptionStorage,
                                             Credentials::CertificateValidityPolicy * policy,
                                             Credentials::GroupDataProvider * responderGroupDataProvider);

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished(const SessionHandle & session) override;

    //// UnsolicitedMessageHandler Implementation ////
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    //// ExchangeDelegate Implementation ////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
    Messaging::ExchangeMessageDispatch & GetMessageDispatch() override { return GetSession().GetMessageDispatch(); }

    CASESession & GetSession() { return mPairingSession; }

private:
    Messaging::ExchangeManager * mExchangeManager                       = nullptr;
    SessionResumptionStorage * mSessionResumptionStorage                = nullptr;
    Credentials::CertificateValidityPolicy * mCertificateValidityPolicy = nullptr;

    //
    // When we're in the process of establishing a session, this is used
    // to maintain an additional, strong reference to the underlying SecureSession.
    // This is because the existing reference in PairingSession is a weak one
    // (i.e a SessionHolder) and can lose its reference if the session is evicted
    // for any reason.
    //
    // This initially points to a session that is not yet active. Upon activation, it
    // transfers ownership of the session to the SecureSessionManager and this reference
    // is released before simultaneously acquiring ownership of a new SecureSession.
    //
    Optional<SessionHandle> mPinnedSecureSession;

    CASESession mPairingSession;
    SessionManager * mSessionManager = nullptr;

    FabricTable * mFabrics                              = nullptr;
    Credentials::GroupDataProvider * mGroupDataProvider = nullptr;

    CHIP_ERROR InitCASEHandshake(Messaging::ExchangeContext * ec);

    /*
     * This will clean up any state from a previous session establishment
     * attempt (if any) and setup the machinery to listen for and handle
     * any session handshakes there-after.
     *
     * If a session had previously been established successfully, previouslyEstablishedPeer
     * should be set to the scoped node-id of the peer associated with that session.
     *
     */
    void PrepareForSessionEstablishment(const ScopedNodeId & previouslyEstablishedPeer = ScopedNodeId());
};

} // namespace chip
