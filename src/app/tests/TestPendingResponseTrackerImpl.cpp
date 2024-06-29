/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <algorithm>
#include <vector>

#include <app/PendingResponseTrackerImpl.h>
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;

TEST(TestPendingResponseTrackerImpl, TestPendingResponseTracker_FillEntireTracker)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;
    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        EXPECT_FALSE(pendingResponseTracker.IsTracked(commandRef));
        EXPECT_EQ(CHIP_NO_ERROR, pendingResponseTracker.Add(commandRef));
        EXPECT_TRUE(pendingResponseTracker.IsTracked(commandRef));
    }

    EXPECT_EQ(std::numeric_limits<uint16_t>::max(), pendingResponseTracker.Count());

    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        EXPECT_EQ(CHIP_NO_ERROR, pendingResponseTracker.Remove(commandRef));
        EXPECT_FALSE(pendingResponseTracker.IsTracked(commandRef));
    }
    EXPECT_EQ(0u, pendingResponseTracker.Count());
}

TEST(TestPendingResponseTrackerImpl, TestPendingResponseTracker_FillSingleEntryInTracker)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // The value 40 is arbitrary; any value would work for this purpose.
    uint16_t commandRefToSet = 40;
    EXPECT_EQ(CHIP_NO_ERROR, pendingResponseTracker.Add(commandRefToSet));

    for (uint16_t commandRef = 0; commandRef < std::numeric_limits<uint16_t>::max(); commandRef++)
    {
        bool expectedIsSetResult = (commandRef == commandRefToSet);
        EXPECT_EQ(expectedIsSetResult, pendingResponseTracker.IsTracked(commandRef));
    }
}

TEST(TestPendingResponseTrackerImpl, TestPendingResponseTracker_RemoveNonExistentEntryInTrackerFails)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // The value 40 is arbitrary; any value would work for this purpose.
    uint16_t commandRef = 40;
    EXPECT_FALSE(pendingResponseTracker.IsTracked(commandRef));
    EXPECT_EQ(CHIP_ERROR_KEY_NOT_FOUND, pendingResponseTracker.Remove(commandRef));
}

TEST(TestPendingResponseTrackerImpl, TestPendingResponseTracker_AddingSecondEntryFails)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // The value 40 is arbitrary; any value would work for this purpose.
    uint16_t commandRef = 40;
    EXPECT_FALSE(pendingResponseTracker.IsTracked(commandRef));
    EXPECT_EQ(CHIP_NO_ERROR, pendingResponseTracker.Add(commandRef));
    EXPECT_TRUE(pendingResponseTracker.IsTracked(commandRef));
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, pendingResponseTracker.Add(commandRef));
}

TEST(TestPendingResponseTrackerImpl, TestPendingResponseTracker_PopFindsAllPendingRequests)
{
    chip::app::PendingResponseTrackerImpl pendingResponseTracker;

    // The specific values in requestsToAdd are not significant; they are chosen arbitrarily for testing purposes.
    std::vector<uint16_t> requestsToAdd = { 0, 50, 2, 2000 };
    for (const uint16_t & commandRef : requestsToAdd)
    {
        EXPECT_FALSE(pendingResponseTracker.IsTracked(commandRef));
        EXPECT_EQ(CHIP_NO_ERROR, pendingResponseTracker.Add(commandRef));
        EXPECT_TRUE(pendingResponseTracker.IsTracked(commandRef));
    }

    EXPECT_EQ(requestsToAdd.size(), pendingResponseTracker.Count());

    for (size_t i = 0; i < requestsToAdd.size(); i++)
    {
        auto commandRef = pendingResponseTracker.PopPendingResponse();
        EXPECT_TRUE(commandRef.HasValue());
        bool expectedCommandRef = std::find(requestsToAdd.begin(), requestsToAdd.end(), commandRef.Value()) != requestsToAdd.end();
        EXPECT_TRUE(expectedCommandRef);
    }
    EXPECT_EQ(0u, pendingResponseTracker.Count());
    auto commandRef = pendingResponseTracker.PopPendingResponse();
    EXPECT_FALSE(commandRef.HasValue());
}

} // namespace
