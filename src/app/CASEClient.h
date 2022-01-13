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

#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/SessionIDAllocator.h>

namespace chip {

class CASEClient;

typedef void (*OnCASEConnected)(void * context, CASEClient * client);
typedef void (*OnCASEConnectionFailure)(void * context, CASEClient * client, CHIP_ERROR error);

struct CASEClientInitParams
{
    SessionManager * sessionManager          = nullptr;
    Messaging::ExchangeManager * exchangeMgr = nullptr;
    SessionIDAllocator * idAllocator         = nullptr;
    FabricInfo * fabricInfo                  = nullptr;

    Optional<ReliableMessageProtocolConfig> mrpLocalConfig = Optional<ReliableMessageProtocolConfig>::Missing();
};

class DLL_EXPORT CASEClient : public SessionEstablishmentDelegate
{
public:
    CASEClient(const CASEClientInitParams & params);

    void SetMRPIntervals(const ReliableMessageProtocolConfig & mrpConfig);

    CHIP_ERROR EstablishSession(PeerId peer, const Transport::PeerAddress & peerAddress,
                                const ReliableMessageProtocolConfig & mrpConfig, OnCASEConnected onConnection,
                                OnCASEConnectionFailure onFailure, void * context);

    // Implementation of SessionEstablishmentDelegate
    void OnSessionEstablishmentError(CHIP_ERROR error) override;

    void OnSessionEstablished() override;

    CHIP_ERROR DeriveSecureSessionHandle(SessionHolder & handle);

private:
    CASEClientInitParams mInitParams;

    CASESession mCASESession;
    PeerId mPeerId;
    Transport::PeerAddress mPeerAddress;

    OnCASEConnected mConnectionSuccessCallback         = nullptr;
    OnCASEConnectionFailure mConnectionFailureCallback = nullptr;
    void * mConectionContext                           = nullptr;
};

} // namespace chip
