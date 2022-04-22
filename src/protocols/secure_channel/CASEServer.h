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

#include <credentials/GroupDataProvider.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/CASESession.h>

namespace chip {

class CASEServer : public SessionEstablishmentDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    CASEServer() {}
    ~CASEServer() override
    {
        if (mExchangeManager != nullptr)
        {
            mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_Sigma1);
        }
    }

    CHIP_ERROR ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, TransportMgrBase * transportMgr,
                                             SessionManager * sessionManager, FabricTable * fabrics,
                                             SessionResumptionStorage * sessionResumptionStorage,
                                             Credentials::GroupDataProvider * responderGroupDataProvider);

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished(const SessionHandle & session) override;
    void OnSessionEstablishmentDone(PairingSession * pairing) override;

    //// UnsolicitedMessageHandler Implementation ////
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                            Messaging::ExchangeDelegate *& newDelegate) override;
    void OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate) override;

private:
    Messaging::ExchangeManager * mExchangeManager        = nullptr;
    SessionResumptionStorage * mSessionResumptionStorage = nullptr;

    Optional<CASESession> mPairingSession; // TODO: use a pool to enable concurrent CASE session.
    SessionManager * mSessionManager = nullptr;

    FabricTable * mFabrics                              = nullptr;
    Credentials::GroupDataProvider * mGroupDataProvider = nullptr;
};

} // namespace chip
