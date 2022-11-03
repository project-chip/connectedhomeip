/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/CASEClient.h>

namespace chip {

CASEClient::CASEClient(const CASEClientInitParams & params) : mInitParams(params) {}

void CASEClient::SetRemoteMRPIntervals(const ReliableMessageProtocolConfig & remoteMRPConfig)
{
    mCASESession.SetRemoteMRPConfig(remoteMRPConfig);
}

CHIP_ERROR CASEClient::EstablishSession(const ScopedNodeId & peer, const Transport::PeerAddress & peerAddress,
                                        const ReliableMessageProtocolConfig & remoteMRPConfig,
                                        SessionEstablishmentDelegate * delegate)
{
    VerifyOrReturnError(mInitParams.fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Create a UnauthenticatedSession for CASE pairing.
    Optional<SessionHandle> session = mInitParams.sessionManager->CreateUnauthenticatedSession(peerAddress, remoteMRPConfig);
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);

    // Allocate the exchange immediately before calling CASESession::EstablishSession.
    //
    // CASESession::EstablishSession takes ownership of the exchange and will
    // free it on error, but can only do this if it is actually called.
    // Allocating the exchange context right before calling EstablishSession
    // ensures that if allocation succeeds, CASESession has taken ownership.
    Messaging::ExchangeContext * exchange = mInitParams.exchangeMgr->NewContext(session.Value(), &mCASESession);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_INTERNAL);

    mCASESession.SetGroupDataProvider(mInitParams.groupDataProvider);
    ReturnErrorOnFailure(mCASESession.EstablishSession(*mInitParams.sessionManager, mInitParams.fabricTable, peer, exchange,
                                                       mInitParams.sessionResumptionStorage, mInitParams.certificateValidityPolicy,
                                                       delegate, mInitParams.mrpLocalConfig));

    return CHIP_NO_ERROR;
}

}; // namespace chip
