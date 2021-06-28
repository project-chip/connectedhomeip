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

#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>

namespace chip {

class CASEServer : public SessionEstablishmentDelegate, public Messaging::ExchangeDelegate
{
public:
    CASEServer() {}
    ~CASEServer()
    {
        if (mExchangeManager != nullptr)
        {
            mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::CASE_SigmaR1);
        }

        mCredentials.Release();
    }

    CHIP_ERROR ListenForSessionEstablishment(Messaging::ExchangeManager * exchangeManager, TransportMgrBase * transportMgr,
                                             SecureSessionMgr * sessionMgr, Transport::AdminPairingTable * admins,
                                             SessionIDAllocator * idAllocator);

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    void OnSessionEstablishmentError(CHIP_ERROR error) override;
    void OnSessionEstablished() override;

    //// ExchangeDelegate Implementation ////
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
    Messaging::ExchangeMessageDispatch * GetMessageDispatch(Messaging::ReliableMessageMgr * reliableMessageManager,
                                                            SecureSessionMgr * sessionMgr) override
    {
        return mPairingSession.GetMessageDispatch(reliableMessageManager, sessionMgr);
    }

    CASESession & GetSession() { return mPairingSession; }

private:
    Messaging::ExchangeManager * mExchangeManager = nullptr;

    CASESession mPairingSession;
    uint16_t mSessionKeyId         = 0;
    SecureSessionMgr * mSessionMgr = nullptr;

    Transport::AdminId mAdminId = Transport::kUndefinedAdminId;

    Transport::AdminPairingTable * mAdmins = nullptr;
    Credentials::ChipCertificateSet mCertificates;
    Credentials::OperationalCredentialSet mCredentials;
    Credentials::CertificateKeyId mRootKeyId;

    CHIP_ERROR InitCASEHandshake(Messaging::ExchangeContext * ec);

    SessionIDAllocator * mIDAllocator = nullptr;

    void Cleanup();
};

} // namespace chip
