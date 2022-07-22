/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/**
 *    @file
 *      This file implements unit tests for the SessionManager implementation.
 */

#include <lib/support/UnitTestRegistration.h>
#include <transport/MessageCounter.h>
#include <transport/PeerMessageCounter.h>

#include <errno.h>
#include <nlbyteorder.h>
#include <nlunit-test.h>
#include <vector>

namespace {

using namespace chip;

static uint32_t counterValuesArray[] = { 0, 10, 0x7FFFFFFF, 0x80000000, 0x80000001, 0x80000002, 0xFFFFFFF0, 0xFFFFFFFF };

void GroupRollOverTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = 1; k <= 2 * CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) == CHIP_NO_ERROR);

            counter.CommitGroup(n);

            // 1. A counter value of N + k comes in, we detect it as valid and commit it.
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n + k) == CHIP_NO_ERROR);
            counter.CommitGroup(n + k);

            // 2. A counter value of N comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

            // 3. A counter value between N - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE and
            //    N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE (but not including
            //    N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) comes in, we treat it as duplicate.
            for (uint32_t i = n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; i != (n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE);
                 i++)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(i) != CHIP_NO_ERROR);
            }

            // 4. A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE comes in, is treated as valid.
            if (k != CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                NL_TEST_ASSERT(inSuite,
                               counter.VerifyOrTrustFirstGroup(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
            }
            else
            {
                NL_TEST_ASSERT(inSuite,
                               counter.VerifyOrTrustFirstGroup(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) != CHIP_NO_ERROR);
            }
        }
    }
}

void GroupBackTrackTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        chip::Transport::PeerMessageCounter counter;
        NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) == CHIP_NO_ERROR);

        counter.CommitGroup(n);
        // 1.   Some set of values N - k come in, for 0 < k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE.
        //      All of those should be considered valid and committed.
        for (uint32_t k = 1; k * k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n - (k * k)) == CHIP_NO_ERROR);
            counter.CommitGroup(n - (k * k));
        }
        // 2. Counter value N + 3 comes in
        NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n + 3) == CHIP_NO_ERROR);
        counter.CommitGroup(n + 3);

        // 3. The same set of values N - k come in as in step (1) and are all considered duplicates/out of window.
        for (uint32_t k = 1; k * k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n - (k * k)) != CHIP_NO_ERROR);
        }

        // 4. The values that were not in the set in step (a) (but are at least N + 3 - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
        //    come in, and all are treated as allowed.
        for (uint32_t k = n + 3 - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k != n + 3; ++k)
        {
            if (n - k == 0 || n - k == 1 || n - k == 4 || n - k == 9 || n - k == 16 || n - k == 25)
            {
                continue;
            }
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(k) == CHIP_NO_ERROR);
            counter.CommitGroup(k);
        }
    }
}

void GroupBigLeapTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = (static_cast<uint32_t>(1 << 31) - 5); k <= (static_cast<uint32_t>(1 << 31) - 1); k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) == CHIP_NO_ERROR);

            counter.CommitGroup(n);

            // 1. A counter value of N + k comes in, we detect it as valid and commit it.
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n + k) == CHIP_NO_ERROR);
            counter.CommitGroup(n + k);

            // 2. A counter value of N comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) != CHIP_NO_ERROR);

            // 3. A counter value between N and N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE
            //    (but not including N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) comes in, we treat it as duplicate.

            // Only test some values to save processing time
            std::vector<uint32_t> testValues;
            testValues.push_back(static_cast<uint32_t>(n + (k / 32) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 16) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 8) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 4) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 3) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 2) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE - 1));

            // Will be inside the valid window of counter + (2^31 -1)
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);

            for (auto it : testValues)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(it) != CHIP_NO_ERROR);
            }

            // 4. A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE comes in, is treated as valid.
            NL_TEST_ASSERT(inSuite,
                           counter.VerifyOrTrustFirstGroup(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
        }
    }
}

void GroupOutOfWindow(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = (static_cast<uint32_t>(1 << 31)); k <= (static_cast<uint32_t>(1 << 31) + 2); k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n) == CHIP_NO_ERROR);

            counter.CommitGroup(n);

            // 1. A counter value of N + k comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyOrTrustFirstGroup(n + k) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
        }
    }
}

void UnicastSmallStepTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = 1; k <= 2 * CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            chip::Transport::PeerMessageCounter counter;
            counter.SetCounter(chip::Transport::PeerMessageCounter::kInitialSyncValue);
            if (counter.VerifyEncryptedUnicast(n) == CHIP_NO_ERROR)
            {
                // Act like we got this counter value on the wire.
                counter.CommitEncryptedUnicast(n);
            }
            else
            {
                // Can't happen immediately after the initial value; just pretend we got here via
                // sufficiently large jumps that there is nothing in the window.
                counter.SetCounter(n);
            }

            // A counter value of N comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

            // A counter value of N + k comes in, we detect it as valid only if it would not
            // overflow, and commit it.
            if (k > UINT32_MAX - n)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n + k) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
                // The other tests make no sense if we did not commit N+k as the new max counter.
                continue;
            }

            NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n + k) == CHIP_NO_ERROR);
            counter.CommitEncryptedUnicast(n + k);

            // A counter value of N comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

            // A counter value between N - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE and
            // N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE (but not including
            // N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) comes in, we treat it as
            // duplicate.  We have to watch out for undeflow here, though: the limits on this loop
            // need to be guarded so they don't underflow.
            uint32_t outOfWindowStart =
                (n >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) ? (n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) : 0;
            uint32_t outOfWindowEnd =
                (n + k >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) ? (n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) : 0;
            for (uint32_t i = outOfWindowStart; i < outOfWindowEnd; i++)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(i) != CHIP_NO_ERROR);
            }

            // A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE (if that does not
            // underflow) comes in.  If it is not equal to n and not equal to 0 (which we always
            // treat as seen), it is treated as valid.  Otherwise it's treated as duplicate.
            if ((n + k) >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                if ((k != CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) && (n + k != CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE))
                {
                    NL_TEST_ASSERT(
                        inSuite, counter.VerifyEncryptedUnicast(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
                }
                else
                {
                    NL_TEST_ASSERT(
                        inSuite, counter.VerifyEncryptedUnicast(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) != CHIP_NO_ERROR);
                }
            }
        }
    }
}

void UnicastLargeStepTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = (static_cast<uint32_t>(1 << 31) - 5); k <= (static_cast<uint32_t>(1 << 31) - 1); k++)
        {
            chip::Transport::PeerMessageCounter counter;
            counter.SetCounter(chip::Transport::PeerMessageCounter::kInitialSyncValue);
            if (counter.VerifyEncryptedUnicast(n) == CHIP_NO_ERROR)
            {
                // Act like we got this counter value on the wire.
                counter.CommitEncryptedUnicast(n);
            }
            else
            {
                // Can't happen immediately after the initial value; just pretend we got here via
                // sufficiently large jumps that there is nothing in the window.
                counter.SetCounter(n);
            }

            // 1. A counter value of N + k comes in, we detect it as valid only
            // if it would not overflow, and commit it.
            if (k > UINT32_MAX - n)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n + k) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
                // The other tests make no sense if we did not commit N+k as the new max counter.
                continue;
            }

            NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n + k) == CHIP_NO_ERROR);
            counter.CommitEncryptedUnicast(n + k);

            // 2. A counter value of N comes in, we detect it as duplicate.
            NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(n) != CHIP_NO_ERROR);

            // 3. A counter value between N and N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE
            //    (but not including N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) comes in, we treat it as duplicate.

            // Only test some values to save processing time
            std::vector<uint32_t> testValues;
            testValues.push_back(static_cast<uint32_t>(n + (k / 32) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 16) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 8) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 4) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 3) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 2) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE - 1));

            // n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE will be smaller than the current allowed counter values.
            if (n >= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                NL_TEST_ASSERT(inSuite,
                               counter.VerifyEncryptedUnicast(n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) != CHIP_NO_ERROR);
            }

            for (auto it : testValues)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyEncryptedUnicast(it) != CHIP_NO_ERROR);
            }

            // 4. A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE comes in, is treated as valid.
            NL_TEST_ASSERT(inSuite,
                           counter.VerifyEncryptedUnicast(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
        }
    }
}

void UnencryptedRollOverTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = 1; k <= 2 * CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);

            counter.CommitUnencrypted(n);

            // 1. A counter value of N + k comes in, we detect it as valid and commit it.
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n + k) == CHIP_NO_ERROR);
            counter.CommitUnencrypted(n + k);

            // 2. A counter value of N comes in, we detect it as duplicate if
            // it's in the window.
            if (k <= CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);
                // Don't commit here so we change our max counter value.
            }

            // 4. A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE comes in, is treated as valid.
            if (k != CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
            {
                NL_TEST_ASSERT(inSuite,
                               counter.VerifyUnencrypted(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
            }
            else
            {
                NL_TEST_ASSERT(inSuite,
                               counter.VerifyUnencrypted(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) != CHIP_NO_ERROR);
            }
        }
    }
}

void UnencryptedBackTrackTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        chip::Transport::PeerMessageCounter counter;
        NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);

        counter.CommitUnencrypted(n);
        // 1.   Some set of values N - k come in, for 0 < k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE.
        //      All of those should be considered valid and committed.
        for (uint32_t k = 1; k * k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n - (k * k)) == CHIP_NO_ERROR);
            counter.CommitUnencrypted(n - (k * k));
        }
        // 2. Counter value N + 3 comes in
        NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n + 3) == CHIP_NO_ERROR);
        counter.CommitUnencrypted(n + 3);

        // 3. The same set of values N - k come in as in step (1) and are all considered duplicates.
        //    This test is valid because 25 + 3 < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE, so none of these values
        //    are out of window, and 25 is the biggest k*k value we are dealing with.
        for (uint32_t k = 1; k * k < CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k++)
        {
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n - (k * k)) != CHIP_NO_ERROR);
        }

        // 4. The values that were not in the set in step (a) (but are at least N + 3 - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE)
        //    come in, and all are treated as allowed.
        for (uint32_t k = n + 3 - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE; k != n + 3; ++k)
        {
            if (n - k == 0 || n - k == 1 || n - k == 4 || n - k == 9 || n - k == 16 || n - k == 25)
            {
                continue;
            }
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(k) == CHIP_NO_ERROR);
            counter.CommitUnencrypted(k);
        }
    }
}

void UnencryptedBigLeapTest(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = (static_cast<uint32_t>(1 << 31) - 5); k <= (static_cast<uint32_t>(1 << 31) - 1); k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);

            counter.CommitUnencrypted(n);

            // 1. A counter value of N + k comes in, we detect it as valid and commit it.
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n + k) == CHIP_NO_ERROR);
            counter.CommitUnencrypted(n + k);

            // 2. A counter value of N comes in, we detect it as valid, since
            // it's out of window.
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);
            // Don't commit, though.

            // 3. A counter value between N and N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE
            //    (but not including N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) comes in, we treat it as valid.
            // Don't commit any of these, though.

            // Only test some values to save processing time
            std::vector<uint32_t> testValues;
            testValues.push_back(static_cast<uint32_t>(n + (k / 32) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 16) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 8) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 4) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 3) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + (k / 2) - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE));
            testValues.push_back(static_cast<uint32_t>(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE - 1));

            // Will be inside the valid window of counter + (2^31 -1)
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);

            for (auto it : testValues)
            {
                NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(it) == CHIP_NO_ERROR);
            }

            // 4. A counter value of N + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE comes in, is treated as valid.
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n + k - CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE) == CHIP_NO_ERROR);
        }
    }
}

void UnencryptedOutOfWindow(nlTestSuite * inSuite, void * inContext)
{
    for (auto n : counterValuesArray)
    {
        for (uint32_t k = (static_cast<uint32_t>(1 << 31)); k <= (static_cast<uint32_t>(1 << 31) + 2); k++)
        {
            chip::Transport::PeerMessageCounter counter;
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n) == CHIP_NO_ERROR);

            counter.CommitUnencrypted(n);

            // 1. A counter value of N + k comes in, we treat it as valid, since
            // it's out of window.
            NL_TEST_ASSERT(inSuite, counter.VerifyUnencrypted(n + k) == CHIP_NO_ERROR);
        }
    }
}

} // namespace

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Group Roll over Test",           GroupRollOverTest),
    NL_TEST_DEF("Group Backtrack Test",           GroupBackTrackTest),
    NL_TEST_DEF("Group All value test",           GroupBigLeapTest),
    NL_TEST_DEF("Group Out of Window Test",       GroupOutOfWindow),
    NL_TEST_DEF("Unicast small step Test",        UnicastSmallStepTest),
    NL_TEST_DEF("Unicast large step Test",        UnicastLargeStepTest),
    NL_TEST_DEF("Unencrypted Roll over Test",     UnencryptedRollOverTest),
    NL_TEST_DEF("Unencrypted Backtrack Test",     UnencryptedBackTrackTest),
    NL_TEST_DEF("Unencrypted All value test",     UnencryptedBigLeapTest),
    NL_TEST_DEF("Unencrypted Out of Window Test", UnencryptedOutOfWindow),
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Main
 */
int TestPeerMessageCounter()
{
    // Run test suit against one context

    nlTestSuite theSuite = { "Transport-TestPeerMessageCounter", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPeerMessageCounter);
