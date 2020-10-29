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

    PeerMessageCounter() : mStatus(Status::NotSync) { }
    ~PeerMessageCounter() { }

    void Reset() { mStatus = Status::NotSync; }

    bool IsSyncStarted() { return mStatus != Status::NotSync; }
    bool IsSynchronizing() { return mStatus == Status::SyncInProcess; }
    bool IsSyncCompleted() { return mStatus == Status::Synced; }

    void StartSync(std::array<uint8_t, kChallengeSize> challenge)
    {
        mStatus = Status::SyncInProcess;
        mSyncInProcess.mChallenge = challenge;
    }

    void SyncFail() { Reset(); }

    CHIP_ERROR VerifyChallenge(uint32_t counter, std::array<uint8_t, kChallengeSize> challenge)
    {
        if (mStatus != Status::SyncInProcess) return CHIP_ERROR_INCORRECT_STATE;
        if (mSyncInProcess.mChallenge != challenge) return CHIP_ERROR_INVALID_ARGUMENT;

        mStatus = Status::Synced;
        mSynced.mMaxCounter = counter - 1;
        mSynced.mWindow.set();
        return CHIP_NO_ERROR;
    }

    /**
     * If counter is valid, return true and adjust max counter. If counter is
     * invalid, return false
     */
    CHIP_ERROR Verify(uint32_t counter)
    {
        if (mStatus != Status::Synced) return CHIP_ERROR_INCORRECT_STATE;

        uint32_t diff = counter - mSynced.mMaxCounter;
        if (diff == 0) return CHIP_ERROR_INVALID_ARGUMENT; // duplicated, max counter has already been received

        if (diff >= 0x80000000u)
        {
            uint32_t offset = 0xFFFFFFFFu - diff;
            if (offset >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) return CHIP_ERROR_INVALID_ARGUMENT; // outside valid range
            if (mSynced.mWindow.test(offset)) return CHIP_ERROR_INVALID_ARGUMENT; // duplicated, in window
        }

        // advance max counter by `diff`
        mSynced.mMaxCounter = counter;
        mSynced.mWindow <<= diff;
        if (diff <= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) mSynced.mWindow.set(diff - 1); // set bit represents old max counter
        return CHIP_NO_ERROR;
    }

private:
    enum class Status {
        NotSync,
        SyncInProcess,
        Synced,
    } mStatus;

    union {
        struct {
            std::array<uint8_t, kChallengeSize> mChallenge;
        } mSyncInProcess;

        struct {
            /*
             *  Past <--                         --> Future
             *                 MaxCounter
             *                     |
             *                     v
             *  | <-- mWindow -->|   |
             *  | n |  ...   | 0 |   |
             */
            uint32_t mMaxCounter; // The most recent counter we have seen
            std::bitset<CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE> mWindow;
        } mSynced;
    };
};

} // namespace Transport
} // namespace chip
