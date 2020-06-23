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

#include <transport/PeerConnections.h>

#include <support/CodeUtils.h>

namespace chip {
namespace Transport {

CHIP_ERROR PeerConnectionsBase::CreateNewPeerConnectionState(const PeerAddress & address, PeerConnectionState ** state)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    if (state)
    {
        *state = nullptr;
    }

    for (size_t i = 0; i < mConnectionStateArraySize; i++)
    {
        if (!mConnectionStateArray[i].GetPeerAddress().IsInitialized())
        {
            mConnectionStateArray[i] = PeerConnectionState(address);
            mConnectionStateArray[i].SetLastActivityTimeMs(GetCurrentMonotonicTimeMs());

            if (state)
            {
                *state = &mConnectionStateArray[i];
            }

            err = CHIP_NO_ERROR;
            break;
        }
    }

    return err;
}

bool PeerConnectionsBase::FindPeerConnectionState(const PeerAddress & address, PeerConnectionState ** state)
{
    *state = nullptr;
    for (size_t i = 0; i < mConnectionStateArraySize; i++)
    {
        if (mConnectionStateArray[i].GetPeerAddress() == address)
        {
            *state = &mConnectionStateArray[i];
            break;
        }
    }
    return *state != nullptr;
}

bool PeerConnectionsBase::FindPeerConnectionState(NodeId nodeId, PeerConnectionState ** state)
{
    *state = nullptr;
    for (size_t i = 0; i < mConnectionStateArraySize; i++)
    {
        if (!mConnectionStateArray[i].GetPeerAddress().IsInitialized())
        {
            continue;
        }
        if (mConnectionStateArray[i].GetPeerNodeId() == nodeId)
        {
            *state = &mConnectionStateArray[i];
            break;
        }
    }
    return *state != nullptr;
}

void PeerConnectionsBase::ExpireInactiveConnections(uint64_t maxIdleTimeMs)
{
    const uint64_t currentTime = GetCurrentMonotonicTimeMs();

    for (size_t i = 0; i < mConnectionStateArraySize; i++)
    {
        if (!mConnectionStateArray[i].GetPeerAddress().IsInitialized())
        {
            continue; // not an active connection
        }

        uint64_t connectionActiveTime = mConnectionStateArray[i].GetLastActivityTimeMs();
        if (connectionActiveTime + maxIdleTimeMs >= currentTime)
        {
            continue; // not expired
        }

        if (OnConnectionExpired)
        {
            OnConnectionExpired(mConnectionStateArray[i], mConnectionExpiredArgument);
        }

        // Connection is assumed expired, marking it as invalid
        mConnectionStateArray[i].Reset();
    }
}

} // namespace Transport
} // namespace chip
