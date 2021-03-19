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

namespace chip {
namespace Transport {

class PeerMessageCounter
{
public:
    static constexpr size_t kChallengeSize = 8;

    PeerMessageCounter() : mStatus(Status::NotSync) {}
    ~PeerMessageCounter() {}

    void Reset() { mStatus = Status::NotSync; }

    bool IsSyncStarted() { return mStatus != Status::NotSync; }
    bool IsSynchronizing() { return mStatus == Status::SyncInProcess; }
    bool IsSyncCompleted() { return mStatus == Status::Synced; }

    void StartSync(std::array<uint8_t, kChallengeSize> challenge)
    {
        mStatus                   = Status::SyncInProcess;
        mSyncInProcess.mChallenge = challenge;
    }

    void SyncFail() { Reset(); }

    CHIP_ERROR VerifyChallenge(uint32_t counter, std::array<uint8_t, kChallengeSize> challenge)
    {
        if (mStatus != Status::SyncInProcess)
            return CHIP_ERROR_INCORRECT_STATE;
        if (mSyncInProcess.mChallenge != challenge)
            return CHIP_ERROR_INVALID_ARGUMENT;

        mStatus             = Status::Synced;
        mSynced.mMaxCounter = counter;
        mSynced.mWindow.set();         // set all bits, deny all packets with counter less than the given counter
        mSynced.mWindow.set(0, false); // expect next packet with counter equal to the given counter
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Verify(uint32_t counter) const
    {
        if (mStatus != Status::Synced)
            return CHIP_ERROR_INCORRECT_STATE;

        if (counter <= mSynced.mMaxCounter)
        {
            uint32_t offset = mSynced.mMaxCounter - counter;
            if (offset >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
                return CHIP_ERROR_INVALID_ARGUMENT; // outside valid range
            if (mSynced.mWindow.test(offset))
                return CHIP_ERROR_INVALID_ARGUMENT; // duplicated, in window
        }

        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *    With the counter verified and the packet MAC is also verified by the secure key, we can trust the packet and adjust
     *    counter states.
     *
     * @prerequisites Verify(counter) == CHIP_NO_ERROR
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
            mSynced.mWindow.set(0, true);
        }
    }

private:
    enum class Status
    {
        NotSync,
        SyncInProcess,
        Synced,
    } mStatus;

    union
    {
        struct
        {
            std::array<uint8_t, kChallengeSize> mChallenge;
        } mSyncInProcess;

        struct
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
        } mSynced;
    };
};

} // namespace Transport
} // namespace chip
