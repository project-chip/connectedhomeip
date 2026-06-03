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

#include <app/CASESessionManager.h>
#include <app/OperationalSessionSetup.h>

namespace chip {

// Provides tests access to private members of OperationalSessionSetup so they
// can force the internal state without driving a real CASE handshake.
class TestOperationalSessionSetupAccess
{
public:
    // Force the setup into a representative "establishing" state.
    static void SetEstablishing(OperationalSessionSetup & setup) { setup.mState = OperationalSessionSetup::State::Connecting; }

    // Force the setup into the fully-connected (non-establishing) state.
    static void SetSecureConnected(OperationalSessionSetup & setup)
    {
        setup.mState = OperationalSessionSetup::State::SecureConnected;
    }

    // Force the setup into the uninitialized (non-establishing) state.
    static void SetUninitialized(OperationalSessionSetup & setup) { setup.mState = OperationalSessionSetup::State::Uninitialized; }

    // Force the setup into each of the other establishing states individually.
    // Together with SetEstablishing (which uses Connecting) these cover all 5 states
    // for which IsEstablishingSession() returns true.
    static void SetNeedsAddress(OperationalSessionSetup & setup) { setup.mState = OperationalSessionSetup::State::NeedsAddress; }
    static void SetResolvingAddress(OperationalSessionSetup & setup)
    {
        setup.mState = OperationalSessionSetup::State::ResolvingAddress;
    }
    static void SetHasAddress(OperationalSessionSetup & setup) { setup.mState = OperationalSessionSetup::State::HasAddress; }
    static void SetWaitingForRetry(OperationalSessionSetup & setup)
    {
        setup.mState = OperationalSessionSetup::State::WaitingForRetry;
    }

    static bool IsEstablishing(const OperationalSessionSetup & setup) { return setup.IsEstablishingSession(); }

    // Set the peer id so the setup can be located via the pool's
    // FindSessionSetup, independent of how it was constructed.
    static void SetPeerId(OperationalSessionSetup & setup, const ScopedNodeId & peerId) { setup.mPeerId = peerId; }
};

// Provides tests access to private members of CASESessionManager so they can
// install a session-setup pool directly (without a full Init) and exercise
// ReleaseSession(peerId).
class TestCASESessionManagerAccess
{
public:
    static void SetSessionSetupPool(CASESessionManager & manager, OperationalSessionSetupPoolDelegate * pool)
    {
        manager.mConfig.sessionSetupPool = pool;
    }
};

} // namespace chip
