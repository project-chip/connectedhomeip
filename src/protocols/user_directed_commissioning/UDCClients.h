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

#include "UDCClientState.h"
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <system/TimeSource.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

// UDC client state times out after 1 hour. This may need to be tweaked.
constexpr const System::Clock::Timestamp kUDCClientTimeout = System::Clock::Milliseconds64(60 * 60 * 1000);

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
        const System::Clock::Timestamp currentTime = mTimeSource.GetMonotonicTimestamp();

        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

        if (state)
        {
            *state = nullptr;
        }

        for (auto & stateiter : mStates)
        {
            if (!stateiter.IsInitialized(currentTime))
            {
                stateiter.SetInstanceName(instanceName);
                stateiter.SetExpirationTime(currentTime + kUDCClientTimeout);
                stateiter.SetUDCClientProcessingState(UDCClientProcessingState::kDiscoveringNode);

                if (state)
                {
                    *state = &stateiter;
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
     * @param index is the index of the connection to find
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UDCClientState * GetUDCClientState(size_t index)
    {
        if (index >= kMaxClientCount)
        {
            return nullptr;
        }

        const System::Clock::Timestamp currentTime = mTimeSource.GetMonotonicTimestamp();
        UDCClientState state                       = mStates[index];
        if (!state.IsInitialized(currentTime))
        {
            return nullptr;
        }
        return &mStates[index];
    }

    /**
     * Get a UDC Client state given a Peer address.
     *
     * @param address is the connection to find (based on address)
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UDCClientState * FindUDCClientState(const PeerAddress & address)
    {
        const System::Clock::Timestamp currentTime = mTimeSource.GetMonotonicTimestamp();

        UDCClientState * state = nullptr;

        for (auto & stateiter : mStates)
        {
            if (!stateiter.IsInitialized(currentTime))
            {
                continue;
            }
            if (stateiter.GetPeerAddress() == address)
            {
                state = &stateiter;
                break;
            }
        }
        return state;
    }

    /**
     * Get a UDC Client state given an instance name.
     *
     * @param instanceName is the instance name to find (based upon instance name)
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    UDCClientState * FindUDCClientState(const char * instanceName)
    {
        const System::Clock::Timestamp currentTime = mTimeSource.GetMonotonicTimestamp();

        UDCClientState * state = nullptr;

        for (auto & stateiter : mStates)
        {
            if (!stateiter.IsInitialized(currentTime))
            {
                continue;
            }

            // TODO: check length of instanceName
            if (strncmp(stateiter.GetInstanceName(), instanceName, Dnssd::Commission::kInstanceNameMaxLength + 1) == 0)
            {
                state = &stateiter;
                break;
            }
        }
        return state;
    }

    // Reset all states to kNotInitialized
    void ResetUDCClientStates()
    {
        for (auto & stateiter : mStates)
        {
            stateiter.Reset();
        }
    }

    /// Convenience method to mark a UDC Client state as active (non-expired)
    void MarkUDCClientActive(UDCClientState * state)
    {
        state->SetExpirationTime(mTimeSource.GetMonotonicTimestamp() + kUDCClientTimeout);
    }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    UDCClientState mStates[kMaxClientCount];
};

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
