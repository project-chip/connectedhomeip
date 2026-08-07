/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/OperationalSessionSetup.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <system/SystemClock.h>

namespace chip {

// Minimal, test-owned access to the private members of OperationalSessionSetup
// needed to drive OnSessionEstablishmentError() directly (regression coverage
// for the stale-operational-address re-resolve fix) without standing up a real
// CASE handshake. Granted via a single `friend class` in OperationalSessionSetup.h.
class OperationalSessionSetupTestAccess
{
public:
    // Set the peer id so LookupPeerAddress() can target the right node.
    static void SetPeerId(OperationalSessionSetup & setup, const ScopedNodeId & peerId) { setup.mPeerId = peerId; }

    // Force the setup into the Connecting state with an allocated CASE client, so
    // that OnSessionEstablishmentError() (which requires mState == Connecting and
    // dereferences mCASEClient) can be invoked directly. The init params (in
    // particular the fabric table, needed by LookupPeerAddress) and the client
    // pool are set directly so the setup need not pass full CASEClientInitParams
    // validation (which requires a group data provider not present in this
    // harness).
    static CHIP_ERROR EnterConnectingWithCaseClient(OperationalSessionSetup & setup, const CASEClientInitParams & params,
                                                    CASEClientPoolDelegate & clientPool,
                                                    OperationalSessionReleaseDelegate & releaseDelegate)
    {
        setup.mInitParams      = params;
        setup.mClientPool      = &clientPool;
        setup.mReleaseDelegate = &releaseDelegate;
        setup.mAddressLookupHandle.SetListener(&setup);
        if (setup.mCASEClient == nullptr)
        {
            setup.mCASEClient = setup.mClientPool->Allocate();
            VerifyOrReturnError(setup.mCASEClient != nullptr, CHIP_ERROR_NO_MEMORY);
        }
        setup.mState = OperationalSessionSetup::State::Connecting;
        return CHIP_NO_ERROR;
    }

    // Set the number of fresh DNS-SD resolve attempts the setup is allowed to make.
    static void SetResolveAttemptsAllowed(OperationalSessionSetup & setup, uint8_t attempts)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_AUTOMATIC_CASE_RETRIES
        setup.mResolveAttemptsAllowed = attempts;
        if (attempts > setup.mRemainingAttempts)
        {
            setup.mRemainingAttempts = attempts;
        }
#else
        (void) setup;
        (void) attempts;
#endif
    }

    // Seed the setup's address-lookup handle with a (stale) cached result, as if
    // a prior resolution had completed. This mirrors the pre-operational address
    // that CASE locked onto in the stale-address timeout scenario.
    static void SeedCachedLookupResult(OperationalSessionSetup & setup, const AddressResolve::ResolveResult & result)
    {
        auto now = System::SystemClock().GetMonotonicTimestamp();
        AddressResolve::NodeLookupRequest request(PeerId(0, setup.mPeerId.GetNodeId()));
        setup.mAddressLookupHandle.ResetForLookup(now, request);
        setup.mAddressLookupHandle.LookupResult(result);
    }

    static bool HasCachedLookupResult(const OperationalSessionSetup & setup)
    {
        return setup.mAddressLookupHandle.HasLookupResult();
    }

    static OperationalSessionSetup::State GetState(const OperationalSessionSetup & setup) { return setup.mState; }

    static AddressResolve::NodeLookupHandle & GetAddressLookupHandle(OperationalSessionSetup & setup)
    {
        return setup.mAddressLookupHandle;
    }

    static void InvokeOnSessionEstablishmentError(OperationalSessionSetup & setup, CHIP_ERROR error)
    {
        setup.OnSessionEstablishmentError(error, SessionEstablishmentStage::kSentSigma1);
    }
};

} // namespace chip
