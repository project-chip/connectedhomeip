/*
 *    Copyright (c) 2021-2026 Project CHIP Authors
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
#include <utility>
#include <variant>

#include <lib/support/Span.h>

namespace chip {
namespace Transport {

class PeerMessageCounter
{
public:
    static constexpr size_t kChallengeSize      = 8;
    static constexpr uint32_t kInitialSyncValue = 0;

    PeerMessageCounter() : mSyncState(NotSynced{}) {}
    PeerMessageCounter(const PeerMessageCounter &)             = default;
    PeerMessageCounter(PeerMessageCounter &&)                  = default;
    PeerMessageCounter & operator=(const PeerMessageCounter &) = default;
    PeerMessageCounter & operator=(PeerMessageCounter &&)      = default;
    ~PeerMessageCounter() { Reset(); }

    void Reset() { mSyncState = NotSynced{}; }

    bool IsSynchronizing() const { return std::holds_alternative<SyncInProcess>(mSyncState); }
    bool IsSynchronized() const { return std::holds_alternative<Synced>(mSyncState); }

    void SyncStarting(FixedByteSpan<kChallengeSize> challenge)
    {
        VerifyOrDie(std::holds_alternative<NotSynced>(mSyncState));
        SyncInProcess sip;
        ::memcpy(sip.mChallenge.data(), challenge.data(), kChallengeSize);
        mSyncState = std::move(sip);
    }

    void SyncFailed() { Reset(); }

    CHIP_ERROR VerifyChallenge(uint32_t counter, FixedByteSpan<kChallengeSize> challenge)
    {
        if (!std::holds_alternative<SyncInProcess>(mSyncState))
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        SyncInProcess & sip = std::get<SyncInProcess>(mSyncState);
        if (::memcmp(sip.mChallenge.data(), challenge.data(), kChallengeSize) != 0)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mSyncState                               = Synced{};
        std::get<Synced>(mSyncState).mMaxCounter = counter;
        std::get<Synced>(mSyncState).mWindow.reset();
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Implementation of spec 4.5.4.2
     *
     * For encrypted messages of Group Session Type, any arriving message with a counter in the range
     * [(max_message_counter + 1) to (max_message_counter + 2^31 - 1)] (modulo 2^32) SHALL be considered
     * new, and cause the max_message_counter value to be updated. Messages with counters from
     * [(max_message_counter - 2^31) to (max_message_counter - MSG_COUNTER_WINDOW_SIZE - 1)] (modulo 2^
     * 32) SHALL be considered duplicate. Message counters within the range of the bitmap SHALL be
     * considered duplicate if the corresponding bit offset is set to true.
     *
     */
    CHIP_ERROR VerifyGroup(uint32_t counter) const
    {
        if (!std::holds_alternative<Synced>(mSyncState))
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        Position pos = ClassifyWithRollover(counter);
        return VerifyPositionEncrypted(pos, counter);
    }

    CHIP_ERROR VerifyOrTrustFirstGroup(uint32_t counter)
    {
        if (std::holds_alternative<NotSynced>(mSyncState))
        {
            SetCounter(counter);
            return CHIP_NO_ERROR;
        }
        if (std::holds_alternative<Synced>(mSyncState))
        {
            return VerifyGroup(counter);
        }
        VerifyOrDie(false);
        return CHIP_ERROR_INTERNAL;
    }

    /**
     * @brief
     *    With the group counter verified and the packet MIC also verified by the secure key, we can trust the packet and adjust
     *    counter states.
     *
     * @pre counter has been verified via VerifyGroup or VerifyOrTrustFirstGroup
     */
    void CommitGroup(uint32_t counter) { CommitWithRollover(counter); }

    CHIP_ERROR VerifyEncryptedUnicast(uint32_t counter) const
    {
        if (!std::holds_alternative<Synced>(mSyncState))
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        Position pos = ClassifyWithoutRollover(counter);
        return VerifyPositionEncrypted(pos, counter);
    }

    /**
     * @brief
     *    With the counter verified and the packet MIC also verified by the secure key, we can trust the packet and adjust
     *    counter states.
     *
     * @pre counter has been verified via VerifyEncryptedUnicast
     */
    void CommitEncryptedUnicast(uint32_t counter) { CommitWithoutRollover(counter); }

    CHIP_ERROR VerifyUnencrypted(uint32_t counter)
    {
        if (std::holds_alternative<NotSynced>(mSyncState))
        {
            SetCounter(counter);
            return CHIP_NO_ERROR;
        }
        if (std::holds_alternative<Synced>(mSyncState))
        {
            Position pos = ClassifyWithRollover(counter);
            return VerifyPositionUnencrypted(pos, counter);
        }
        VerifyOrDie(false);
        return CHIP_ERROR_INTERNAL;
    }

    /**
     * @brief
     *    With the unencrypted counter verified we can trust the packet and adjust
     *    counter states.
     *
     * @pre counter has been verified via VerifyUnencrypted
     */
    void CommitUnencrypted(uint32_t counter) { CommitWithRollover(counter); }

    void SetCounter(uint32_t value)
    {
        mSyncState                               = Synced{};
        std::get<Synced>(mSyncState).mMaxCounter = value;
        std::get<Synced>(mSyncState).mWindow.reset();
    }

    uint32_t GetCounter() const { return std::get<Synced>(mSyncState).mMaxCounter; }

private:
    // Counter position indicator with respect to our current max counter.
    enum class Position
    {
        BeforeWindow,
        InWindow,
        MaxCounter,
        FutureCounter,
    };

    // Classify an incoming counter value's position.  Must be used only if
    // the peer is synchronized.
    Position ClassifyWithoutRollover(uint32_t counter) const
    {
        auto & synced = std::get<Synced>(mSyncState);
        if (counter > synced.mMaxCounter)
        {
            return Position::FutureCounter;
        }

        return ClassifyNonFutureCounter(counter);
    }

    /**
     * Classify an incoming counter value's position for the cases when counters
     * are allowed to roll over.  Must be used only if the peer is
     * synchronized.
     *
     * This can be used as the basis for implementing section 4.5.4.2 in the
     * spec:
     *
     * For encrypted messages of Group Session Type, any arriving message with a counter in the range
     * [(max_message_counter + 1) to (max_message_counter + 2^31 - 1)] (modulo 2^32) SHALL be considered
     * new, and cause the max_message_counter value to be updated. Messages with counters from
     * [(max_message_counter - 2^31) to (max_message_counter - MSG_COUNTER_WINDOW_SIZE - 1)] (modulo 2^
     * 32) SHALL be considered duplicate. Message counters within the range of the bitmap SHALL be
     * considered duplicate if the corresponding bit offset is set to true.
     */
    Position ClassifyWithRollover(uint32_t counter) const
    {
        auto & synced                          = std::get<Synced>(mSyncState);
        uint32_t counterIncrease               = counter - synced.mMaxCounter;
        constexpr uint32_t futureCounterWindow = (static_cast<uint32_t>(1 << 31)) - 1;

        if (counterIncrease >= 1 && counterIncrease <= futureCounterWindow)
        {
            return Position::FutureCounter;
        }

        return ClassifyNonFutureCounter(counter);
    }

    /**
     * Classify a counter that's known to not be future counter.  This works
     * identically whether we are doing rollover or not.
     */
    Position ClassifyNonFutureCounter(uint32_t counter) const
    {
        auto & synced = std::get<Synced>(mSyncState);
        if (counter == synced.mMaxCounter)
        {
            return Position::MaxCounter;
        }

        uint32_t offset = synced.mMaxCounter - counter;
        if (offset <= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
        {
            return Position::InWindow;
        }

        return Position::BeforeWindow;
    }

    /**
     * Given an encrypted (group or unicast) counter position and the counter
     * value, verify whether we should accept it.
     */
    CHIP_ERROR VerifyPositionEncrypted(Position position, uint32_t counter) const
    {
        auto & synced = std::get<Synced>(mSyncState);
        switch (position)
        {
        case Position::FutureCounter:
            return CHIP_NO_ERROR;
        case Position::InWindow: {
            uint32_t offset = synced.mMaxCounter - counter;
            if (synced.mWindow.test(offset - 1))
            {
                return CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED;
            }
            return CHIP_NO_ERROR;
        }
        default: {
            // Equal to max counter, or before window.
            return CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED;
        }
        }
    }

    /**
     * Given an unencrypted counter position and value, verify whether we should
     * accept it.
     */
    CHIP_ERROR VerifyPositionUnencrypted(Position position, uint32_t counter) const
    {
        auto & synced = std::get<Synced>(mSyncState);
        switch (position)
        {
        case Position::MaxCounter:
            return CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED;
        case Position::InWindow: {
            uint32_t offset = synced.mMaxCounter - counter;
            if (synced.mWindow.test(offset - 1))
            {
                return CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED;
            }
            return CHIP_NO_ERROR;
        }
        default: {
            // Future counter or before window; all of these are accepted.  The
            // before-window case is accepted because the peer may have reset
            // and is using a new randomized initial value.
            return CHIP_NO_ERROR;
        }
        }
    }

    void CommitWithRollover(uint32_t counter)
    {
        Position pos = ClassifyWithRollover(counter);
        CommitWithPosition(pos, counter);
    }

    void CommitWithoutRollover(uint32_t counter)
    {
        Position pos = ClassifyWithoutRollover(counter);
        CommitWithPosition(pos, counter);
    }

    /**
     * Commit a counter value that is known to be at the given position with
     * respect to our max counter.
     */
    void CommitWithPosition(Position position, uint32_t counter)
    {
        auto & synced = std::get<Synced>(mSyncState);
        switch (position)
        {
        case Position::InWindow: {
            uint32_t offset = synced.mMaxCounter - counter;
            synced.mWindow.set(offset - 1);
            break;
        }
        case Position::MaxCounter: {
            // Nothing to do
            break;
        }
        default: {
            // Since we are committing, this becomes a new max-counter value.
            uint32_t shift     = counter - synced.mMaxCounter;
            synced.mMaxCounter = counter;
            if (shift > CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                synced.mWindow.reset();
            }
            else
            {
                synced.mWindow <<= shift;
                synced.mWindow.set(shift - 1);
            }
            break;
        }
        }
    }

    // Synthetic type for "not synced" state (std::monostate)
    using NotSynced = std::monostate;

    struct SyncInProcess
    {
        std::array<uint8_t, kChallengeSize> mChallenge;
    };

    struct Synced
    {
        /*
         *  Past <--                --> Future
         *          MaxCounter - 1
         *                 |
         *                 v
         *  | <-- mWindow -->|
         *  |[n]|  ...   |[0]|
         */
        uint32_t mMaxCounter = 0; // The most recent counter we have seen
        std::bitset<CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE> mWindow;
    };

    std::variant<NotSynced, SyncInProcess, Synced> mSyncState;
};

} // namespace Transport
} // namespace chip
