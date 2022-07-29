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
#include <messaging/ExchangeMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/secure_channel/CASESession.h>

namespace chip {

class CASEClient;

struct CASEClientInitParams
{
    SessionManager * sessionManager                                    = nullptr;
    SessionResumptionStorage * sessionResumptionStorage                = nullptr;
    Credentials::CertificateValidityPolicy * certificateValidityPolicy = nullptr;
    Messaging::ExchangeManager * exchangeMgr                           = nullptr;
    FabricTable * fabricTable                                          = nullptr;
    Credentials::GroupDataProvider * groupDataProvider                 = nullptr;

    Optional<ReliableMessageProtocolConfig> mrpLocalConfig = Optional<ReliableMessageProtocolConfig>::Missing();
};

class DLL_EXPORT CASEClient
{
public:
    CASEClient(const CASEClientInitParams & params);

    void SetRemoteMRPIntervals(const ReliableMessageProtocolConfig & remoteMRPConfig);

    CHIP_ERROR EstablishSession(const ScopedNodeId & peer, const Transport::PeerAddress & peerAddress,
                                const ReliableMessageProtocolConfig & remoteMRPConfig, SessionEstablishmentDelegate * delegate);

private:
    CASEClientInitParams mInitParams;

    CASESession mCASESession;
};

} // namespace chip
