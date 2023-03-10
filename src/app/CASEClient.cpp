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
