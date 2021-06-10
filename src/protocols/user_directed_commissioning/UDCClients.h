/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "UDCClientState.h"
#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <system/TimeSource.h>
#include <transport/AdminPairingTable.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

// TODO: determine when UDC client's state should time out
constexpr const uint64_t kUDCClientTimeoutMs = 60 * 60 * 1000;

/**
 * Handles a set of UDC Client Processing States.
 *
 * Intended for:
 *   - ignoring/dropping duplicate UDC messages for the same instance
 *   - tracking state of UDC work flow (see UDCClientProcessingState)
 *   - timing out failed/declined UDC Clients
 */
template <size_t kMaxClientCount, Time::Source kTimeSource = Time::Source::kSystem>
class UDCClients
{
public:
    /**
     * Allocates a new UDC client state object out of the internal resource pool.
     *
     * @param instanceName represents the UDS Client instance name
     * @param state [out] will contain the UDC Client state if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'expiration' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum UDC Client count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewUDCClientState(const char * instanceName, UDCClientState ** state)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

        if (state)
        {
            *state = nullptr;
        }

        for (size_t i = 0; i < kMaxClientCount; i++)
        {
            if (!mStates[i].IsInitialized(currentTime))
            {
                // mStates[i].SetPeerAddress(address);
                mStates[i].SetInstanceName(instanceName);
                mStates[i].SetExpirationTimeMs(currentTime + kUDCClientTimeoutMs);
                mStates[i].SetUDCClientProcessingState(UDCClientProcessingState::kDiscoveringNode);

                if (state)
                {
                    *state = &mStates[i];
                }

                err = CHIP_NO_ERROR;
                break;
            }
        }

        return err;
    }

    /**
     * Get a UDC Client state given a Peer address.
     *
     * @param address is the connection to find (based on address)
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UDCClientState * FindUDCClientState(const PeerAddress & address, UDCClientState * begin)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        UDCClientState * state = nullptr;
        UDCClientState * iter  = &mStates[0];

        if (begin >= iter && begin < &mStates[kMaxClientCount])
        {
            iter = begin + 1;
        }

        for (; iter < &mStates[kMaxClientCount]; iter++)
        {
            if (!iter->IsInitialized(currentTime))
            {
                continue;
            }
            if (iter->GetPeerAddress() == address)
            {
                state = iter;
                break;
            }
        }
        return state;
    }

    /**
     * Get a UDC Client state given a Peer address.
     *
     * @param address is the connection to find (based on address)
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UDCClientState * FindUDCClientState(const char * instanceName, UDCClientState * begin)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        UDCClientState * state = nullptr;
        UDCClientState * iter  = &mStates[0];

        if (begin >= iter && begin < &mStates[kMaxClientCount])
        {
            iter = begin + 1;
        }

        for (; iter < &mStates[kMaxClientCount]; iter++)
        {
            if (!iter->IsInitialized(currentTime))
            {
                continue;
            }

            // TODO: check length of instanceName
            if (strncmp(iter->GetInstanceName(), instanceName, USER_DIRECTED_COMMISSIONING_MAX_INSTANCE_NAME) == 0)
            {
                state = iter;
                break;
            }
        }
        return state;
    }

    // Reset all states to kNotInitialized
    void ResetUDCClientStates()
    {
        for (size_t i = 0; i < kMaxClientCount; i++)
        {
            mStates[i].Reset();
        }
    }

    /// Convenience method to mark a peer connection state as active
    void MarkUDCClientActive(UDCClientState * state)
    {
        state->SetExpirationTimeMs(mTimeSource.GetCurrentMonotonicTimeMs() + kUDCClientTimeoutMs);
    }

    /// Allows access to the underlying time source used for keeping track of connection active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    UDCClientState mStates[kMaxClientCount];
};

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
