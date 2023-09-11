/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
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
    Optional<ReliableMessageProtocolConfig> mrpLocalConfig             = Optional<ReliableMessageProtocolConfig>::Missing();

    CHIP_ERROR Validate() const
    {
        // sessionResumptionStorage can be nullptr when resumption is disabled.
        // certificateValidityPolicy is optional, too.
        ReturnErrorCodeIf(sessionManager == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(exchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(fabricTable == nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorCodeIf(groupDataProvider == nullptr, CHIP_ERROR_INCORRECT_STATE);

        return CHIP_NO_ERROR;
    }
};

class DLL_EXPORT CASEClient
{
public:
    void SetRemoteMRPIntervals(const ReliableMessageProtocolConfig & remoteMRPConfig);

    const ReliableMessageProtocolConfig & GetRemoteMRPIntervals();

    CHIP_ERROR EstablishSession(const CASEClientInitParams & params, const ScopedNodeId & peer,
                                const Transport::PeerAddress & peerAddress, const ReliableMessageProtocolConfig & remoteMRPConfig,
                                SessionEstablishmentDelegate * delegate);

private:
    CASESession mCASESession;
};

} // namespace chip
