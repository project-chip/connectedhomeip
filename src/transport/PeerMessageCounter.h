/*
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
 *    @file
 *      This file defines the CHIP message counters of remote nodes.
 *
 */
#pragma once

#include <array>
#include <bitset>

#include <support/Span.h>

namespace chip {
namespace Transport {

class PeerMessageCounter
{
public:
    static constexpr size_t kChallengeSize = 8;

    PeerMessageCounter() : mStatus(Status::NotSynced) {}
    ~PeerMessageCounter() { Reset(); }

    void Reset()
    {
        switch (mStatus)
        {
        case Status::NotSynced:
            break;
        case Status::SyncInProcess:
            mSyncInProcess.~SyncInProcess();
            break;
        case Status::Synced:
            mSynced.~Synced();
            break;
        }
        mStatus = Status::NotSynced;
    }

    bool IsSynchronizing() { return mStatus == Status::SyncInProcess; }
    bool IsSynchronized() { return mStatus == Status::Synced; }

    void SyncStarting(FixedByteSpan<kChallengeSize> challenge)
    {
        assert(mStatus == Status::NotSynced);
        mStatus = Status::SyncInProcess;
        new (&mSyncInProcess) SyncInProcess();
        ::memcpy(mSyncInProcess.mChallenge.data(), challenge.data(), kChallengeSize);
    }

    void SyncFailed() { Reset(); }

    CHIP_ERROR VerifyChallenge(uint32_t counter, FixedByteSpan<kChallengeSize> challenge)
    {
        if (mStatus != Status::SyncInProcess)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (::memcmp(mSyncInProcess.mChallenge.data(), challenge.data(), kChallengeSize) != 0)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mSyncInProcess.~SyncInProcess();
        mStatus = Status::Synced;
        new (&mSynced) Synced();
        mSynced.mMaxCounter = counter;
        mSynced.mWindow.reset(); // reset all bits, accept all packets in the window
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Verify(uint32_t counter) const
    {
        if (mStatus != Status::Synced)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (counter <= mSynced.mMaxCounter)
        {
            uint32_t offset = mSynced.mMaxCounter - counter;
            if (offset >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                return CHIP_ERROR_INVALID_ARGUMENT; // outside valid range
            }
            if (mSynced.mWindow.test(offset))
            {
                return CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED; // duplicated, in window
            }
        }

        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *    With the counter verified and the packet MIC also verified by the secure key, we can trust the packet and adjust
     *    counter states.
     *
     * @pre Verify(counter) == CHIP_NO_ERROR
     */
    void Commit(uint32_t counter)
    {
        if (counter <= mSynced.mMaxCounter)
        {
            uint32_t offset = mSynced.mMaxCounter - counter;
            mSynced.mWindow.set(offset);
        }
        else
        {
            uint32_t offset = counter - mSynced.mMaxCounter;
            // advance max counter by `offset`
            mSynced.mMaxCounter = counter;
            if (offset < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                mSynced.mWindow <<= offset;
            }
            else
            {
                mSynced.mWindow.reset();
            }
            mSynced.mWindow.set(0);
        }
    }

    void SetCounter(uint32_t value)
    {
        Reset();
        mStatus = Status::Synced;
        new (&mSynced) Synced();
        mSynced.mMaxCounter = value;
        mSynced.mWindow.reset();
    }

    uint32_t GetCounter() { return mSynced.mMaxCounter; }

private:
    enum class Status
    {
        NotSynced,     // No state associated
        SyncInProcess, // mSyncInProcess will be active
        Synced,        // mSynced will be active
    } mStatus;

    struct SyncInProcess
    {
        std::array<uint8_t, kChallengeSize> mChallenge;
    };

    struct Synced
    {
        /*
         *  Past <--                --> Future
         *             MaxCounter
         *                 |
         *                 v
         *  | <-- mWindow -->|
         *  |[n]|  ...   |[0]|
         */
        uint32_t mMaxCounter; // The most recent counter we have seen
        std::bitset<CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE> mWindow;
    };

    // We should use std::variant here when migrated to C++17
    union
    {
        SyncInProcess mSyncInProcess;
        Synced mSynced;
    };
};

} // namespace Transport
} // namespace chip
