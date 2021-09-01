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

/**
 * @brief Defines processing state managed by Commissioner for an active User Directed Commissioning (UDC) client.
 */

#pragma once

#include <lib/mdns/Resolver.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

enum class UDCClientProcessingState : uint8_t
{
    kNotInitialized,
    kDiscoveringNode,
    kNodeDiscoveryFailed,
    kPromptingUser,
    kUserDeclined,
    kObtainingOnboardingPayload,
    kCommissioningNode,
    kCommissioningFailed,
};

using PeerAddress = ::chip::Transport::PeerAddress;

/**
 * Defines the handling state of a UDC Client.
 *
 * Information contained within the state:
 *   - PeerAddress represents how to talk to the UDC client
 *   - PeerInstanceName is the DNS-SD instance name of the UDC client
 *   - ExpirationTimeMs is a timestamp of when this object should expire.
 *   - UDCClientProcessingState contains the current state of processing this UDC Client
 *
 */
class UDCClientState
{
public:
    UDCClientState() : mPeerAddress(PeerAddress::Uninitialized()) {}

    UDCClientState(UDCClientState &&)      = default;
    UDCClientState(const UDCClientState &) = default;
    UDCClientState & operator=(const UDCClientState &) = default;
    UDCClientState & operator=(UDCClientState &&) = default;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    const char * GetInstanceName() const { return mInstanceName; }
    void SetInstanceName(const char * instanceName) { strncpy(mInstanceName, instanceName, sizeof(mInstanceName)); }

    UDCClientProcessingState GetUDCClientProcessingState() const { return mUDCClientProcessingState; }
    void SetUDCClientProcessingState(UDCClientProcessingState state) { mUDCClientProcessingState = state; }

    uint64_t GetExpirationTimeMs() const { return mExpirationTimeMs; }
    void SetExpirationTimeMs(uint64_t value) { mExpirationTimeMs = value; }

    bool IsInitialized(uint64_t currentTime)
    {
        // if state is not the "not-initialized" and it has not expired
        return (mUDCClientProcessingState != UDCClientProcessingState::kNotInitialized && mExpirationTimeMs > currentTime);
    }

    /**
     *  Reset the connection state to a completely uninitialized status.
     */
    void Reset()
    {
        mPeerAddress              = PeerAddress::Uninitialized();
        mExpirationTimeMs         = 0;
        mUDCClientProcessingState = UDCClientProcessingState::kNotInitialized;
    }

private:
    PeerAddress mPeerAddress;
    char mInstanceName[chip::Mdns::kMaxInstanceNameSize + 1];
    UDCClientProcessingState mUDCClientProcessingState;
    uint64_t mExpirationTimeMs = 0;
};

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
