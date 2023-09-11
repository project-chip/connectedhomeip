/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/CASEClient.h>

namespace chip {

void CASEClient::SetRemoteMRPIntervals(const ReliableMessageProtocolConfig & remoteMRPConfig)
{
    mCASESession.SetRemoteMRPConfig(remoteMRPConfig);
}

const ReliableMessageProtocolConfig & CASEClient::GetRemoteMRPIntervals()
{
    return mCASESession.GetRemoteMRPConfig();
}

CHIP_ERROR CASEClient::EstablishSession(const CASEClientInitParams & params, const ScopedNodeId & peer,
                                        const Transport::PeerAddress & peerAddress,
                                        const ReliableMessageProtocolConfig & remoteMRPConfig,
                                        SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(params.fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Create a UnauthenticatedSession for CASE pairing.
    Optional<SessionHandle> session = params.sessionManager->CreateUnauthenticatedSession(peerAddress, remoteMRPConfig);
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    // Allocate the exchange immediately before calling CASESession::EstablishSession.
    //
    // CASESession::EstablishSession takes ownership of the exchange and will
    // free it on error, but can only do this if it is actually called.
    // Allocating the exchange context right before calling EstablishSession
    // ensures that if allocation succeeds, CASESession has taken ownership.
    Messaging::ExchangeContext * exchange = params.exchangeMgr->NewContext(session.Value(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    mCASESession.SetGroupDataProvider(params.groupDataProvider);
    ReturnErrorOnFailure(mCASESession.EstablishSession(*params.sessionManager, params.fabricTable, peer, exchange,
                                                       params.sessionResumptionStorage, params.certificateValidityPolicy, delegate,
                                                       params.mrpLocalConfig));

    return CHIP_NO_ERROR;
}

}; // namespace chip
