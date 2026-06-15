/*
 *    Copyright (c) 2026 Project CHIP Authors
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
 *    @file
 *      This file defines the CHIP message counters of remote nodes.
 *
 */

#include <transport/PeerMessageCounter.h>

namespace chip {
namespace Transport {

PeerMessageCounter & PeerMessageCounter::operator=(const PeerMessageCounter & other)
{
    VerifyOrReturnValue(this != &other, *this);
    Reset();
    mStatus = other.mStatus;
    switch (other.mStatus)
    {
    case Status::SyncInProcess:
        new (&mSyncInProcess) SyncInProcess(other.mSyncInProcess);
        break;
    case Status::Synced:
        new (&mSynced) Synced(other.mSynced);
        break;
    case Status::NotSynced:
        break;
    }
    return *this;
}

PeerMessageCounter & PeerMessageCounter::operator=(PeerMessageCounter && other) noexcept
{
    if (this != &other)
    {
        *this         = static_cast<const PeerMessageCounter &>(other);
        other.mStatus = Status::NotSynced;
    }
    return *this;
}

} // namespace Transport
} // namespace chip
