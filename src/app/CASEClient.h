/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
